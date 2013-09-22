#include <stdio.h>
#include <stdint.h>

#include "bt.h"
#include "eeprom.h"
#include "clock.h"
#include "code.h"
#include "time.h"
#include "elm.h"
#include "lcd.h"
#include <Arduino.h>
#include <avr/pgmspace.h>

char bt_cmd[BT_CMD_MAX_LEN+1];
char bt_cmd_len;
int  bt_mid_cmd = 0;

#define BT_CMD_SN   "sn"
#define BT_CMD_TIME "time"
#define BT_CMD_CODE "code"
#define BT_CMD_LOCK "lock"

#define BT_PROMPT "2drive> "

void bt_setup()  
{
  // Open serial communications and wait for port to open:
  Serial1.begin(9600);
}

//
// run over and over; return codes:
// 0 - no activity
// 1 - something was read
// 2 - complete command was read
// 
int bt_loop() 
{
    char c;
    int  cnt = 0;
    char buf[64];
    uint32_t code;
    struct tm tm_now, tm_r_start, tm_r_end;
    time_t t_now, t_r_start, t_r_end;
    unsigned long secs_valid; 
    unsigned long days_valid;
	twodrive_data_t data;
    
    if (bt_mid_cmd == 0) {
        Serial1.print(BT_PROMPT);
        bt_cmd_len = 0;
        memset(bt_cmd, 0, sizeof(bt_cmd));
        bt_mid_cmd = 1;
    } 

    while (1) {
        if (Serial1.available() == 0) {
            break;
        }

        c = Serial1.read();
        if (c == -1) {
            return cnt;
        } 

        //printf("** %d\n", c);
            
        // on carriage return, a command was read
        if (c != '\n' && c != '\r') {
            Serial1.print(c);
            bt_cmd[bt_cmd_len++] = c;
            cnt = 1;

            if (bt_cmd_len == BT_CMD_MAX_LEN) {
                bt_cmd[0] = 0;
                Serial1.println("");
                bt_mid_cmd = 0;
                return 1;
            }

            continue;
        }

        // command processing
        Serial1.println("");
        bt_mid_cmd = 0;

        if (strncmp(bt_cmd, BT_CMD_SN, strlen(BT_CMD_SN)) == 0) {
            sprintf(buf, "Serial number:  %s", eeprom_sern_read());
            bt_response(buf);
        } else if (strncmp(bt_cmd, BT_CMD_TIME, strlen(BT_CMD_TIME)) == 0) {
            sprintf(buf, "Date:  %s", rtc_ctime_date());
            bt_response(buf);
            sprintf(buf, "Time:  %s", rtc_ctime_time());
            bt_response(buf);
        } else if (strncmp(bt_cmd, BT_CMD_LOCK, strlen(BT_CMD_LOCK)) == 0) {
            elm_prius_lock(1);
            sprintf(buf, "Locked");
            bt_response(buf);                
        } else if (strncmp(bt_cmd, BT_CMD_CODE, strlen(BT_CMD_CODE)) == 0) {
            bt_cmd[strlen(BT_CMD_CODE)+10] = 0;
            code = strtoul(bt_cmd + strlen(BT_CMD_CODE), NULL, 10);
            //sprintf(buf, "Code entered:  %s %lu", bt_cmd + strlen(BT_CMD_CODE), code);
            //bt_response(buf);

            if (!twodrive_dec(code, &data)) {
                bt_response("Incorrect code");
            } else {
                memset(&tm_now,     0, sizeof(tm_r_start));
                memset(&tm_r_start, 0, sizeof(tm_r_start));

                tm_now = *rtc_get_tm();
                tm_now.tm_year = 100 + (tm_now.tm_year % 4);

                tm_r_start.tm_mon  = data.month - 1;
                tm_r_start.tm_mday = data.day;
                tm_r_start.tm_year = 100 + data.year;

                //sprintf(buf, "Code decoded as %02d/%02d/%04d for %d days", tm_r_start.tm_mon, tm_r_start.tm_mday, tm_r_start.tm_year, data.ndays);
                //bt_response(buf);
                //
                //sprintf(buf, "Now             %02d/%02d/%04d",             tm_now.tm_mon, tm_now.tm_mday, tm_now.tm_year);
                //bt_response(buf);

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
                    sprintf(buf, "Accepted, valid %d days", days_valid);
                    bt_response(buf);
                    elm_prius_lock(0);
                } else {
                    bt_response("Invalid dates");
                }

            }
        } else {
            bt_response("Unsupported command");                
        }

        // ready for the next command
        Serial1.print(BT_PROMPT);
        memset(bt_cmd, 0, sizeof(bt_cmd));
        bt_mid_cmd = 1;
        bt_cmd_len = 0;

        return 2;
    }

    return 0;
}

void bt_response(char *str) 
{
    Serial1.println(str);
}

int  bt_rx_pending()
{
    return Serial1.available();
}

