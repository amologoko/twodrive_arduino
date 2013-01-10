#include <stdio.h>
#include <stdint.h>
#include "eeprom.h"
#include "mpr121.h"
#include "lcd.h"
#include "clock.h"
#include "code.h"
#include "time.h"

#define CODE_TIME      "#100"
#define CODE_SERN      "#101"
#define CODE_PASS      "#102"
#define CODE_SET_SERN  "#13#1#911"

int twodrive_code_read_keypad()
{
	twodrive_data_t data;
    uint32_t code;
    struct tm tm_now, tm_r_start, tm_r_end;
    time_t t_now, t_r_start, t_r_end;
    unsigned long timer_s, timer_e;
    unsigned long secs_valid; 
    unsigned long days_valid;
    int i;
    char s[32];
    int done;
    int c;
    char *sp;
        
	data.year = 2012;
	data.month = 8;
	data.day = 13;
	data.ndays = 10;

    done    = 0;

    while (!done) {
        lcd_str("Enter the code:");
        lcd_str_pos("..........", 40, 0);
        lcd_pos(40);
        lcd_cursor(1, 0);

        i = 0; 
        memset(s, 0, sizeof(s));
        timer_s = timer_e = millis();
        while (i < 10 && (timer_e - timer_s < 15000)) {
            c = mpr121_read_char();
            timer_e = millis();
            
            if (c != -1) {
                timer_e = timer_s = millis();

                // on a *, process special menus; if not a valid special menu, restart
                if (c == '*') {
                    if (strncmp(s, CODE_TIME, sizeof(CODE_TIME)) == 0) {
                        lcd_cursor(0, 0);
                        clock_setup();
                    } else if (strncmp(s, CODE_SERN, sizeof(CODE_SERN)) == 0) {
                        lcd_cursor(0, 0);
                        sp = eeprom_sern_read();
                        printf("Serial number:  %s\n", sp);
                        lcd_str("Serial number:");
                        lcd_str_pos(sp, 40, 0);
                        while (mpr121_read_char() == -1) {
                        }

                    } else if (strncmp(s, CODE_PASS, sizeof(CODE_PASS)) == 0) {
                        pass_setup();
                        delay(3000);
                    }

                    lcd_cursor(1, 0);
                    lcd_str("Enter the code:");
                    lcd_str_pos("..........", 40, 0);
                    lcd_pos(40);
                    i = 0; 
                    memset(s, 0, sizeof(s));
                    continue;
                }

                lcd_chr(c);
                s[i] = c;
                i++;
            }
        }
        lcd_cursor(0, 0);

        // after 60 seconds of inactivity, turn off the display and go back to sleep
        if (timer_e - timer_s >= 15000) {
            lcd_on(0);
            return 1;
        }

        // process the input
        s[10] = 0;
        code = strtoul(s, NULL, 10);
        printf("Code entered:  %s %lu\n", s, code);

        if (!twodrive_dec(code, &data)) {
            lcd_str("Incorrect code");
        }
        else {
            memset(&tm_now,     0, sizeof(tm_r_start));
            memset(&tm_r_start, 0, sizeof(tm_r_start));

            tm_now = *rtc_get_tm();
            tm_now.tm_year = 100 + (tm_now.tm_year % 4);

            tm_r_start.tm_mon  = data.month - 1;
            tm_r_start.tm_mday = data.day;
            tm_r_start.tm_year = 100 + data.year;

            printf("Code decoded as %02d/%02d/%04d for %d days\n", tm_r_start.tm_mon, tm_r_start.tm_mday, tm_r_start.tm_year, data.ndays);
            printf("Now             %02d/%02d/%04d\n",             tm_now.tm_mon, tm_now.tm_mday, tm_now.tm_year);

            t_now     = mktime(&tm_now);
            t_r_start = mktime(&tm_r_start);
            t_r_end   = t_r_start + SECS_DAY*data.ndays;

            // if current date is in the code-specified range, we are good to go
            if (t_now >= t_r_start && t_now <= t_r_end) {
                secs_valid = t_r_end - t_now;
                days_valid = secs_valid / SECS_DAY;
                if (secs_valid % SECS_DAY) {
                    days_valid++;
                }
                sprintf(s, "Valid %d days", days_valid);
                lcd_str("Accepted");
                lcd_str_pos(s, 40, 0);		                
            } else {
                lcd_str("Invalid dates");
                delay(1000);

                printf("1 %lu\n", t_r_start);
                printf("2 %lu\n", t_now);
                printf("3 %lu\n", t_r_end);
            }

        }
        delay(5000);

    }            

    return 0;
}

