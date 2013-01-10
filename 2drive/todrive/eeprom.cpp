#include <stdio.h>
#include <string.h>
#include <EEPROM.h>
#include "lcd.h"
#include "mpr121.h"

#define EEP_HDR_START 0
#define EEP_HDR_LEN   4

#define EEP_PASS_START 4
#define EEP_PASS_LEN   4

#define EEP_SERN_START 8
#define EEP_SERN_LEN   16

char sern_buf[17];

char *eeprom_sern_read() {
    int i;
    memset(sern_buf, 0, sizeof(sern_buf));
    for (i=0; i<EEP_SERN_LEN; i++) {
        sern_buf[i] = EEPROM.read(EEP_SERN_START+i);
        if (! isascii(sern_buf[i])) {
            sern_buf[i] = '?';
        }
    }
    return sern_buf;
}

int eeprom_sern_valid() {
    int i;
    memset(sern_buf, 0, sizeof(sern_buf));
    for (i=0; i<EEP_SERN_LEN; i++) {
        sern_buf[i] = EEPROM.read(EEP_SERN_START+i);
        if (! isascii(sern_buf[i])) {
            return 0;
        }
    }

    return 1;
}


void eeprom_sern_write(char *sern) {
    int i;
    for (i=0; i<EEP_SERN_LEN; i++) {
        EEPROM.write(EEP_SERN_START+i,  sern[i]);
    }
}

unsigned long eeprom_pass_read() {
    int i;
    unsigned long w = 0;
    for (i=0; i<EEP_PASS_LEN; i++) {
        w |= EEPROM.read(EEP_PASS_START+i) << 8*i;
    }
    return w;
}

void eeprom_pass_write(unsigned long w) {
    int i;
    for (i=0; i<EEP_PASS_LEN; i++) {
        EEPROM.write(EEP_PASS_START+i,  (w >> 8*i) & 0xFF);
    }
}




// setup a password
void pass_setup() {
    int c;
    int i;
    char s[32];
    unsigned long p1, p2;
    int pass = 0;
    int enter = 0;

    while (1) {

        if (pass == 0) {
            lcd_str_pos("Set new password", 0,  1);
        } else {
            lcd_str_pos("Re-enter again", 0,  1);
        }
        lcd_str_pos("........", 40, 0);
        lcd_pos(40);
        lcd_cursor(1, 0);

        i     = 0;
        enter = 0;
        memset(s, 0, sizeof(s));
        while (1) {
            c = mpr121_read_char();
            if (c != -1) {
                        
                if (c == '*') {
                    break;
                }
                if (c == '#') {
                    enter = 1;
                    break;
                }

                lcd_chr(c);
                s[i] = c;
                i++;
                if (i == 8) {
                    enter = 1;
                    break;
                }
            }
        }

        if (enter == 0) {
            continue;
        }
        lcd_cursor(0, 0);

        // process the input
        if (pass == 0) {
            p1 = atoi(s);
            pass++;
        } else {
            p2 = atoi(s);

            if (p1 != p2) {
                lcd_str_pos("Passwords", 0, 1); 
                lcd_str_pos("did not match", 40, 0);
                pass = 0;
                delay(5000);
            } else {
                lcd_str_pos("Password set", 0, 1);
                eeprom_pass_write(p1);
                return;
            }
        }              
    }
}

// setup a password
int pass_check() {
    int c;
    int i;
    char s[32];
    unsigned long p1, p2;
    int enter = 0;

    while (1) {

        lcd_str_pos("Enter password:", 0,  1);
        lcd_str_pos("........", 40, 0);
        lcd_pos(40);
        lcd_cursor(1, 0);

        i     = 0;
        enter = 0;
        memset(s, 0, sizeof(s));
        while (1) {
            c = mpr121_read_char();
            if (c != -1) {
                        
                if (c == '*') {
                    break;
                }
                if (c == '#') {
                    enter = 1;
                    break;
                }

                lcd_chr(c);
                s[i] = c;
                i++;
                if (i == 8) {
                    enter = 1;
                    break;
                }
            }
        }

        if (enter == 0) {
            continue;
        }
        lcd_cursor(0, 0);

        p1 = atoi(s);
        p2 = eeprom_pass_read();

        if (p1 != p2) {
            lcd_str_pos("Incorrect", 0, 1); 
            lcd_str_pos("password", 40, 0);
            delay(3000);
            return 0;
        } else {
            return 1;
        }
    }

    return 0;
}

// setup a serial #
void sern_setup() {
    int c;
    int i;
    char s[32];
    unsigned long p1, p2;
    int pass = 0;
    int enter = 0;

    while (1) {

        if (pass == 0) {
            lcd_str_pos("Set serial #", 0,  1);
        } else {
            lcd_str_pos("Re-enter again", 0,  1);
        }
        lcd_str_pos("................", 40, 0);
        lcd_pos(40);
        lcd_cursor(1, 0);

        i     = 0;
        enter = 0;
        memset(s, 0, sizeof(s));
        while (1) {
            c = mpr121_read_char();
            if (c != -1) {
                        
                if (c == '*') {
                    break;
                }

                lcd_chr(c);
                s[i] = c;
                i++;
                if (i == EEP_SERN_LEN) {
                    enter = 1;
                    break;
                }
            }
        }

        if (enter == 0) {
            continue;
        }
        lcd_cursor(0, 0);

        // process the input
        if (pass == 0) {
            memcpy(sern_buf, s, EEP_SERN_LEN);
            pass++;
        } else {
            if (memcmp(sern_buf, s, EEP_SERN_LEN)) {
                lcd_str_pos("Serial numbers", 0, 1); 
                lcd_str_pos("did not match", 40, 0);
                pass = 0;
                delay(5000);
            } else {
                lcd_str_pos("Serial number set", 0, 1);
                eeprom_sern_write(sern_buf);
                return;
            }
        }              
    }
}
