#include "elm.h"
#include <stdio.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

SoftwareSerial *serial_elm;

void elm_setup(int rx_pin, int tx_pin) 
{
    serial_elm = new SoftwareSerial(rx_pin, tx_pin); // RX, TX

    serial_elm->begin(9600);

    // get a prompt
    elm_cmd("AT", 0, 10000);

    // received prompt
    elm_cmd("ATL1",   0, 10000);
    elm_cmd("ATH1",   0, 10000);
    elm_cmd("ATCAF0", 0, 10000);

}

int elm_cmd(char *cmd, int send_at, int timeout_ms) 
{
    char c;
    char p;
    unsigned int t_start, t_end;

    // clean up any junk
    while (1) {
        c = serial_elm->read();
        if (c == -1) {
            break;
        }
    }  

    serial_elm->println(cmd);
    printf(cmd);
    delay(1);

    t_start = millis();
    while (1) {
        if (send_at) {
            serial_elm->println("AT");
            delay(1);
        }

        p = 0;
        while (1) {
            c = serial_elm->read();
            if (c == -1) {
                break;
            }

            putchar(c);
            if (c == '>') {
                p = 1;
            }
        };

        if (p) {
            return 0;
        }

        t_end  = millis();
        if (t_end - t_start > timeout_ms) {
            return 1;
        }  
        delay(100);        
    }
    
    return 1;
}

void elm_prius_lock(int on) {
    int i;
    PROGMEM char *ELM_CMD_PRIUS_LOCK   = "18 80 53 02 00 09 00";
    PROGMEM char *ELM_CMD_PRIUS_UNLOCK = "18 80 53 04 00 19 00";

    elm_cmd("ATSH631", 0, 10000);

    for (i=0; i<2; i++) {
        if (on) {
            elm_cmd(ELM_CMD_PRIUS_LOCK, 0, 10000);
        } else {
            elm_cmd(ELM_CMD_PRIUS_UNLOCK, 0, 10000);
        } 
    }
}
