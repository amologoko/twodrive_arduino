#include <I2C.h>
#include "lcd.h"
#include "time.h"
#include "mpr121.h"
#include "clock.h"
#include "eeprom.h"

void clock_setup() {
    time_t cur_time;
    int c;
    int i, done;
    struct tm t_orig;
    char s[32];
    unsigned long t_start, t_end = 0, t_prev;

    // initialize time
    memset(&t_orig, 0, sizeof(t_orig));
    t_orig.tm_year = 112;
    t_orig.tm_mon  = 9-1;
    t_orig.tm_mday = 23;
    t_orig.tm_hour = 12;
    t_orig.tm_min  = 58;
    set_init_time(&t_orig);

    lcd_str_pos(rtc_ctime_date(), 0,  1);
    lcd_str_pos(rtc_ctime_time(), 40, 0);

    // start a 15 second timer
    t_start = millis() / 1000;
    while (1) {

        lcd_str_pos(rtc_ctime_date(), 0,  0);
        lcd_str_pos(rtc_ctime_time(), 40, 0);
        lcd_str_pos("Set?", 12, 0);

        // if no action taken within the timeout, proceed with current time
        t_end = millis() / 1000;
        if (t_end - t_start > 15) {
            break;
        }
        if (t_end != t_prev) {
            t_prev = t_end;
            sprintf(s, "%2d", 15-(t_end-t_start));
            lcd_str_pos(s, 53, 0);
        }

        if ((c = mpr121_read_char()) == -1) {
            delay(10);
        }
        if (c == '*') {
            break;
        } 
        else if (c == '#') {

            // check the password; if doesn't match, return
            if (pass_check() == 0) {
                return;
            }

            done = 0;
            while (!done) {
                lcd_cursor(1, 0);
                lcd_str_pos("Enter below: ", 0, 1);
                lcd_str_pos("MM/DD/YY hh:mm", 40, 0);
                lcd_pos(40);
                memset(&t_orig, 0, sizeof(t_orig));

                i = 0;
                while (1) {
                    c = mpr121_read_char();
                    if (c != -1) {
                        
                        if (c == '*') {
                            break;
                        }

                        lcd_chr(c);
                        s[i] = c;
                        i++;
                        if (i > 0 && i % 2 == 0) {
                            lcd_move(1, 1);
                        }
                        if (i == 10) {
                            lcd_cursor(0, 0);
                            break;
                        }
                    }
                }

                // process the input
                s[10] = 0;
                t_orig.tm_min = atoi(s + 8);

                s[8] = 0;
                t_orig.tm_hour = atoi(s + 6);

                s[6] = 0;
                t_orig.tm_year = atoi(s + 4) + 100;

                s[4] = 0;
                t_orig.tm_mday = atoi(s + 2);

                s[2] = 0;
                t_orig.tm_mon = atoi(s + 0) - 1;

                t_orig.tm_sec = 0;

                sprintf(s, "%02d/%02d/%02d %02d:%02d", 
                        t_orig.tm_mon + 1, 
                        t_orig.tm_mday,
                        t_orig.tm_year - 100, 
                        t_orig.tm_hour,
                        t_orig.tm_min);

                lcd_str_pos(s, 0, 0);
                lcd_str_pos("Is this correct?", 40, 0);
              
                while (1) {
                    c = mpr121_read_char();
                    if (c == '*') {
                        break;
                    } else if (c == '#') {
                        done = 1;
                        break;
                    }
                }

                if (done) {
                    rtc_set_tm(&t_orig);
                }
            }
            break;
        }
    }
}

#define DS_I2C_ADDR 0x68

void ds1338_set_register(byte r, byte v){
    byte b;

    I2c.write((byte)DS_I2C_ADDR, r, v);
    delay(10);
}

// dump filtered electrode data
byte ds1338_get_byte(byte addr) {
    byte c = 0, b;

    I2c.read((byte)DS_I2C_ADDR, addr, (byte)1);
    delay(10);
    while (I2c.available() && c < 1) {
        b = I2c.receive();
        c++;
    }
    if (c != 1) {
        printf("ERROR:  read %d bytes\n", c);
    }
    return b;
}

// get timestamp as a burst read
int ds1338_get_ts(byte *arr) {
    byte c = 0;

    I2c.read((byte)DS_I2C_ADDR, (byte)0, (byte)7);
    while (I2c.available() && c < 7) {
        arr[c] = I2c.receive();
        c++;
    }
    if (c != 7) {
        printf("ERROR:  read %d bytes\n", c);
        return -1;
    }
    return c;
}

void ds1338_set_ts(byte *b){
    I2c.write((byte)DS_I2C_ADDR, 0, b, 7);
    delay(10);
}



void rtc_setup()
{
    int i = 0;
    ds1338_set_register(7, 0);
    rtc_dump();
}

void rtc_dump()
{
    int i = 0;
    byte b[7];

    for (i=0; i<8; i++) {
        printf("%d:  %02x\n", i, ds1338_get_byte(i));
    }
    ds1338_get_ts(b);
    for (i=0; i<7; i++) {
        printf("ts %d:  %02x\n", i, b[i]);
    }
    printf("%s\n", rtc_ctime_time());
    printf("%s", rtc_ctime());
}

// shared buffers
#define ASCBUFSIZE 64
static char      g_ascbuf[ASCBUFSIZE+1];
static struct tm g_tm;

struct tm *rtc_get_tm() {
    byte b[7];

    memset(&g_tm, sizeof(g_tm), 0);
    
    // get timestamp as one burst read
    ds1338_get_ts(b);

    g_tm.tm_sec  = (b[0] & 0xF) + ((b[0] >> 4) & 0x7) * 10;
    g_tm.tm_min  = (b[1] & 0xF) + ((b[1] >> 4) & 0x7) * 10;
    g_tm.tm_hour = (b[2] & 0xF) + ((b[2] >> 4) & 1) * 10;
    
    // check 12 hour format - if bit 6 is set, 12-hour format
    if (b[2] & 0x40) {
        g_tm.tm_hour += ((b[2] >> 5) & 1) * 12;
    } else {
        g_tm.tm_hour += ((b[2] >> 5) & 1) * 20;
    }

    g_tm.tm_mday = (b[4] & 0xF) + ((b[4] >> 4) & 0x3) * 10;
    g_tm.tm_mon  = (b[5] & 0xF) + ((b[5] >> 4) & 0x1) * 10 - 1;     // RTC keeps months 1-12; TM is 0-11
    g_tm.tm_year = (b[6] & 0xF) + ((b[6] >> 4) & 0xF) * 10 + 100;   // TM starts at 1900
    
    return &g_tm;
}

int rtc_set_tm(struct tm *t) {
    
    byte b[7];
    byte mon;
    byte year;

    b[0] = (t->tm_sec % 10)  | (((t->tm_sec  / 10) << 4) & 0x70);
    b[1] = (t->tm_min % 10)  | (((t->tm_min  / 10) << 4) & 0x70);
    b[2] = (t->tm_hour % 10) | (((t->tm_hour / 10) << 4) & 0x30);             // hour in 24-hour format - bit 6=0
    b[4] = (t->tm_mday % 10) | (((t->tm_mday / 10) << 4) & 0x30);

    mon  = t->tm_mon  + 1;
    year = t->tm_year - 100;  
  
    b[5] = (mon  % 10)         | (((mon         / 10) << 4) & 0x10);
    b[6] = (year % 10)         | (((year        / 10) << 4) & 0xF0);
    
    ds1338_set_ts(b);
    
    return 0;
}


char *rtc_ctime_time() {
    struct tm *t = rtc_get_tm();
    sprintf(g_ascbuf, "%02d:%02d:%02d", 
            t->tm_hour,
            t->tm_min,
            t->tm_sec);

    return g_ascbuf;
}

char *rtc_ctime_hhmm() {
    struct tm *t = rtc_get_tm();
    sprintf(g_ascbuf, "%02d:%02d", 
            t->tm_hour,
            t->tm_min);

    return g_ascbuf;
}

char *rtc_ctime_date() {
    struct tm *t = rtc_get_tm();
    sprintf(g_ascbuf, "%02d/%02d/%02d", 
            t->tm_mon+1, 
            t->tm_mday,
            t->tm_year - 100);

    return g_ascbuf;
}

char *rtc_ctime() {
    return asctime(rtc_get_tm());
}
