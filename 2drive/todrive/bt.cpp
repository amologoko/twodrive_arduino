#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdint.h>

#include "bt.h"
#include "eeprom.h"
#include "clock.h"

SoftwareSerial serial_bt(8, 9); // RX, TX

char bt_cmd[CMD_MAX_LEN+1];
char bt_cmd_len;
int  bt_mid_cmd = 0;

#define BT_CMD_SN   "sn"
#define BT_CMD_TIME "time"
#define BT_CMD_CODE "code" 

void bt_setup()  
{
  // Open serial communications and wait for port to open:
  serial_bt.begin(9600);
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

    if (bt_mid_cmd == 0) {
        serial_bt.print("2drive>  ");
        bt_cmd_len = 0;
        memset(bt_cmd, 0, sizeof(bt_cmd));
        bt_mid_cmd = 1;
    } 

    while (1) {
        c = serial_bt.read();
        if (c == -1) {
            return cnt;
        } else {

            // on carriage return, a command was read
            if (c != '\n' && c != '\r') {
                serial_bt.print(c);
                bt_cmd[bt_cmd_len++] = c;
                cnt = 1;

                if (bt_cmd_len == CMD_MAX_LEN) {
                    bt_cmd[0] = 0;
                    serial_bt.println("");
                    bt_mid_cmd = 0;
                    return 1;
                }

                continue;
            }

            // command processing
            serial_bt.println("");
            bt_mid_cmd = 0;

            if (strncmp(bt_cmd, BT_CMD_SN, strlen(BT_CMD_SN)) == 0) {
                sprintf(buf, "Serial number:  %s\n", eeprom_sern_read());
                bt_response(buf);
            } else if (strncmp(bt_cmd, BT_CMD_TIME, strlen(BT_CMD_SN)) == 0) {
                //sprintf(buf, "Date:  %s\n", rtc_ctime_date());
                //bt_response(buf);
                //sprintf(buf, "Time:  %s\n", rtc_ctime_time());
                bt_response(buf);
            } else if (strncmp(bt_cmd, BT_CMD_CODE, strlen(BT_CMD_SN)) == 0) {
                sprintf(buf, "Serial number:  %s\n", eeprom_sern_read());
                bt_response(buf);
            } else {
                bt_response("Unsupported command\n");                
            }

            // ready for the next command
            serial_bt.print("2drive>  ");
            bt_cmd_len = 0;
            memset(bt_cmd, 0, sizeof(bt_cmd));
            bt_mid_cmd = 1;

            return 2;

        }        
    }

    return 0;
}

void bt_response(char *str) 
{
    serial_bt.println(str);
}

int  bt_rx_pending()
{
    return serial_bt.available();
}



//
// example use
//

/*
void setup() {
    Serial.begin(9600);
    Serial.println("ELM327 test");

    bt_setup();
}

void loop() {
    unsigned long code;
    char buf[32];

    while (1) {
        code = bt_loop();
        if (code) {
            sprintf(buf, "Code received:  %ld", code);
            bt_response(buf);
        }
    }
}
*/
