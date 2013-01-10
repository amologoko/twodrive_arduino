#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdint.h>

SoftwareSerial serial_bt(13, 6); // RX, TX


void elm_setup() 
{
    serial_bt.begin(9600);
}


#define CMD_MAX_LEN 32

char bt_cmd[CMD_MAX_LEN+1];
char bt_cmd_len;
int  bt_mid_cmd = 0;

void bt_setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("ELM327 test");

  serial_bt.begin(9600);
}

//
// run over and over; returns zero or a code
// 
long bt_loop() 
{
    char c;

    if (bt_mid_cmd == 0) {
        serial_bt.print("2drive>  ");
        bt_cmd_len = 0;
        memset(bt_cmd, 0, sizeof(bt_cmd));
        bt_mid_cmd = 1;
    } 

    while (1) {
        c = serial_bt.read();
        if (c == -1) {
            return 0;
        } else {

            if (c == '\n' || c == '\r') {
                serial_bt.println("");
                bt_mid_cmd = 0;
                break;
            }

            serial_bt.print(c);
            bt_cmd[bt_cmd_len++] = c;

            if (bt_cmd_len == CMD_MAX_LEN) {
                bt_cmd[0] = 0;
                serial_bt.println("");
                bt_mid_cmd = 0;
                return 0;
            }
        }        
    }

    // figure out the command
    return atol(bt_cmd);

}

void bt_response(char *str) 
{
    serial_bt.println(str);
}


//
// example use
//

void setup() {
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
