#include <stdio.h>
#include <stdint.h>
#include "eeprom.h"
#include "mpr121.h"
#include "lcd.h"
#include "clock.h"
#include "code.h"
#include "time.h"
#include "elm.h"

#define CODE_LOCK      "#000"
#define CODE_TIME      "#100"
#define CODE_SERN      "#101"
#define CODE_PASS      "#102"
#define CODE_SET_SERN  "#13#1#911"

#define KP_CMD_MAX_LEN 10

char kp_cmd[KP_CMD_MAX_LEN+1];
char kp_cmd_len;
int  kp_mid_cmd = 0;


int ui_read_keypad()
{
	twodrive_data_t data;
    uint32_t code;
    struct tm tm_now, tm_r_start, tm_r_end;
    time_t t_now, t_r_start, t_r_end;
    unsigned long secs_valid; 
    unsigned long days_valid;
    int c;
    char *sp;
    char str[32];

    while (1) {

        // if wasn't mid-command, re-print the prompt
        if (kp_mid_cmd == 0) {
            lcd_str("Enter the code:");
            lcd_str_pos("..........", 40, 0);
            lcd_pos(40);
            lcd_cursor(1, 0);

            kp_cmd_len = 0;
            memset(kp_cmd, 0, sizeof(kp_cmd));
            kp_mid_cmd = 1;
        }

        // read the character
        c = mpr121_read_char();
        if (c == -1) {
            return 0;
        }
                    
        // on a *, process special menus; if not a valid special menu, restart
        if (c == '*') {
            if (strncmp(kp_cmd, CODE_TIME, sizeof(CODE_TIME)) == 0) {
                lcd_cursor(0, 0);
                clock_setup();
            } else if (strncmp(kp_cmd, CODE_SERN, sizeof(CODE_SERN)) == 0) {
                lcd_cursor(0, 0);
                sp = eeprom_sern_read();
                //printf("Serial number:  %s\n", sp);
                lcd_str("Serial number:");
                lcd_str_pos(sp, 40, 0);
                while (mpr121_read_char() == -1) {
                }

            } else if (strncmp(kp_cmd, CODE_PASS, sizeof(CODE_PASS)) == 0) {
                if (eeprom_pass_read() != PASS_UNSET) {
                    if (pass_check()) {
                        pass_setup();
                        delay(3000);
                    }
                }
            } else if (strncmp(kp_cmd, CODE_LOCK, sizeof(CODE_LOCK)) == 0) {
                lcd_cursor(0, 0);
                //printf("Locking the door\n");
                lcd_str("Locking the door");
                elm_prius_lock(1);
                delay(5000);
            }

            // restart with a new command
            kp_mid_cmd = 0;
        }

        lcd_chr(c);
        kp_cmd[kp_cmd_len] = c;
        kp_cmd_len++;

        if (kp_cmd_len < KP_CMD_MAX_LEN) {
            continue;
        }

        //
        // code processing
        //
        kp_cmd[KP_CMD_MAX_LEN] = 0;
        lcd_cursor(0, 0);

        // process the input
        code = strtoul(kp_cmd, NULL, 10);
        //printf("Code entered:  %s %lu\n", kp_cmd, code);

        if (!twodrive_dec(code, &data)) {
            lcd_str("Incorrect code");
        } else {
            memset(&tm_now,     0, sizeof(tm_r_start));
            memset(&tm_r_start, 0, sizeof(tm_r_start));

            tm_now = *rtc_get_tm();
            tm_now.tm_year = 100 + (tm_now.tm_year % 4);

            tm_r_start.tm_mon  = data.month - 1;
            tm_r_start.tm_mday = data.day;
            tm_r_start.tm_year = 100 + data.year;

            //printf("Code decoded as %02d/%02d/%04d for %d days\n", tm_r_start.tm_mon, tm_r_start.tm_mday, tm_r_start.tm_year, data.ndays);
            //printf("Now             %02d/%02d/%04d\n",             tm_now.tm_mon, tm_now.tm_mday, tm_now.tm_year);

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
                sprintf(str, "Valid %d days", days_valid);
                lcd_str("Accepted");
                lcd_str_pos(str, 40, 0);		                
                elm_prius_lock(0);
            } else {
                lcd_str("Invalid dates");
                delay(1000);
            }

        }
        delay(5000);
        kp_mid_cmd = 0;
        kp_cmd_len = 0;
    }            

    return 1;
}

void ui_goto_sleep() {
    // if was mid-processing, stop and shut off the screen
    if (kp_mid_cmd == 1) {
        kp_mid_cmd = 0;
    }
    lcd_on(0);
}
