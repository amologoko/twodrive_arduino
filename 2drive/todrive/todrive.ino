#include <I2C.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "mpr121.h"

#include <stdio.h>
#include <stdint.h>
#include "time.h"
#include "lcd.h"
#include "clock.h"
#include "eeprom.h"
#include "bt.h"
#include "ui.h"
#include "code.h"
#include "elm.h"

//////////////////////////////////////////////////////////////////////////////////////////
// configuration
//////////////////////////////////////////////////////////////////////////////////////////

// char PIN_LCD_RS     = 22;
// char PIN_LCD_RW     = 24; 
// char PIN_LCD_E      = 26;
// char PIN_LCD_B8[8]  = {28, 30, 32, 34, 36, 38, 40, 42};
// char PIN_LCD_B4[4]  = {                36, 38, 40, 42};
// char PIN_LCD_LCDP   = 44;

char PIN_LCD_RS     = A3;
char PIN_LCD_RW     = A2; 
char PIN_LCD_E      = A1;
char PIN_LCD_B4[4]  = {A0, 15, 14, 16};
char PIN_LCD_LCDP   = 10;

char PIN_ELM_UARTRX = 9;
char PIN_ELM_UARTTX = 8;


//////////////////////////////////////////////////////////////////////////////////////////
// redirect standard output
//////////////////////////////////////////////////////////////////////////////////////////
static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
  Serial.write(c) ;
  return 0 ;
}

/////////////////////////////////////////////////////////////////////////////////////////
// touchpad ISR - if detected while in sleep, go to code input processing loop
/////////////////////////////////////////////////////////////////////////////////////////
int twodrive_code_read_bt();

int awoke = 0;

void wakeUp() {
    
    //printf("WakeUp\n");

    // disable self, turn on the LCD and remove whatever character is in the buffer
    //mpr121_isr(0, NULL);
    if (awoke == 0) {
        lcd_on(1);
        lcd_str("Wait...");
        mpr121_read_char();
        awoke = 1;
        delay(100);
    }
    // set the awoke flag
}


////
// arduino funcs
///

void setup(void)
{
    char buf[2];
    int  rc;

    // Start the UART
    Serial.begin(9600) ;
    //Serial.setTimeout(1000000);

    // fill in the UART file descriptor with pointer to writer.
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

    // The uart is the standard output device STDOUT.
    stdout = &uartout;

    printf("2Drive, ver %d\n", 1);

    // configure the LCD
    //printf("Config LCD\n");
    lcd_setup(0, PIN_LCD_RS, PIN_LCD_RW, PIN_LCD_E, PIN_LCD_B4, PIN_LCD_LCDP);
    //lcd_setup(1, PIN_LCD_RS, PIN_LCD_RW, PIN_LCD_E, PIN_LCD_B8, PIN_LCD_LCDP);

    lcd_str("Ver: " __DATE__);
    lcd_str_pos(__TIME__, 40, 0);
    delay(1000);

    lcd_str("2Drive");    
    lcd_pos(40);
    delay(500);

    //printf("Config ELM327\n");
    rc = elm_setup(PIN_ELM_UARTRX, PIN_ELM_UARTTX);
    if (rc) {
        lcd_str_pos("x", 40, 0);
        delay(2000);
        
        // print the RC
        sprintf(buf, "%d", rc);
        lcd_str_pos(buf, 40, 0);
        delay(2000);

        // dump the buffer
        //extern char elm_buf[256];
        //lcd_str_pos(elm_buf, 0, 0);
    } else {
        lcd_str_pos(".", 40, 0);
    }
    delay(250);

    // start the I2C interface
    //printf("Config I2C\n");
    lcd_str_pos(".", 41, 0);
    I2c.begin();
    I2c.timeOut(3000);
    delay(250);

    // configure RTC
    printf("Config RTC\n");
    lcd_str_pos(".", 42, 0);
    rtc_setup();
    delay(250);
    
    // run initial MPR121 initialization
    //printf("Config touchpad\n");
    lcd_str_pos(".", 43, 0);
    mpr121_setup();
    delay(250);

    // // debug - keep dumping MPR121 data once a second - use to debug
    // printf("Starting to dump data\n");
    // while (1) {
    //     printf("Dumping data\n");
    //     mpr121_dump_data();
    //     delay(1000);
    // }

    // configure bluetooth
    //printf("Config Bluetooth\n");
    //lcd_str_pos(".", 44, 0);
    //bt_setup();
    //delay(250);

    //printf("Booting complete\n");
    lcd_str_pos(".", 45, 0);
    delay(250);

    // setup serial # if not already set
    if (eeprom_sern_valid() == 0) {
        sern_setup();
    }
    //printf("Serial # is:  %s\n", eeprom_sern_read());

    // setup password if not already set
    if (eeprom_pass_read() == PASS_UNSET) {
        pass_setup();
    }
    //printf("Password:  %ld\n", eeprom_pass_read());

    // initialize the crypto
    crypt_setup();
    
    for (int i=0; i<3; i++) {
        lcd_str_pos(rtc_ctime_date(), 0,  1);
        lcd_str_pos(rtc_ctime_time(), 40, 0);
        delay(1000);
    }

    lcd_str("Ready");
    delay(1000);

    //twodrive_code_read();

    lcd_str("Going to sleep");
    delay(1000);
    lcd_on(0);
    
    // register the ISR - if keypad touched, will wake up and go to process the code
    awoke = 0;
    mpr121_isr(1, wakeUp);
}

int first = 1;
unsigned long ts_bt_last = 0;

void loop() {
    int c_in;

    /*
    // interactive debug interface
    if (Serial.available() > 0) {
        c_in = Serial.read();
        switch (c_in) {
        case 'd':
            printf("Dumping:\n");
            mpr121_dump_regs();
            break;

        case 'D':
            while (1) {
                mpr121_dump_data();
                delay(2000);
            }
            break;

        case 'i':
            printf("Reinitializing...  ");
            mpr121_setup();
            printf("done\n");
            break;

        case 'c':
            printf("Executing the config command...  ");
            mpr121_set_register(ELE_CFG, 0x0C);  // Enables all 12 Electrodes
            printf("done\n");
            break;

        case 'L':
            printf("Writing to LCD\n");
            //printf("0x%02x\n", lcd_rd(1));
            //printf("0x%02x\n", lcd_rd(0));
            lcd_str("Hello, world!!!");
            printf("Status: %02x\n", lcd_rd(0));
            break;

        case 'C':
            printf("Clearing LCD\n");
            lcd_clr();
            break;

        case 't':
            printf("Printing clock\n");
            lcd_on(1);
            lcd_str_pos(rtc_ctime_date(), 0,  1);
            lcd_str_pos(rtc_ctime_time(), 40, 0);
            delay(5000);
            lcd_on(0);
            break;

        case 'B':
            printf("Toggling LCD backlight\n");
            lcd_backlight(-1);
            break;

        case 'R':
            int i;
            printf("Reading LCD buffer:  ");
            for (i=0; i<8; i++) {
                lcd_wr(0, 0x80 | i);
                printf("%02x ", lcd_rd(1));
            }
            break;

        case 'r':
            rtc_dump();
            break;

        default:
            printf("Unknown command '%c'\n", c_in);
        }
    }
*/

    // sleep if no bluetooth activity detected for 30 seconds
    if ((millis() / 1000)- ts_bt_last > 30) {
        delay(500);
    }
    
    // if was woken up, attempt to read the code - routine will exit 
    if (awoke) {
        //printf("Awoke\n");
        mpr121_isr(0, wakeUp);

        twodrive_code_read_keypad();

        // if completed successfully or if expired
        // re-register the ISR - if keypad touched, will wake up and go to process the code
        awoke = 0;
        mpr121_isr(1, wakeUp);
    }

    // check the bluetooth - if anything is pending, start processing
    if (bt_rx_pending()) {
        ts_bt_last = millis() / 1000;
        bt_loop();
    }
}
