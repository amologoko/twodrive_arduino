#include <stdio.h>
#include <stdint.h>
#include <I2C.h>
#include "lcd.h"

static char  pin_rs;
static char  pin_rw; 
static char  pin_e;
static char  pin_b[8];
static char  pin_lcdp;
static char  pin_lcdn;

static char  b_rn_w;
static char  b_lcd;

static int  _lcd_pins;

void lcd_bus_dir(char rn_w) {
    int i;

    if (rn_w) {
        for (i=0; i<_lcd_pins; i++) {
            pinMode(pin_b[i],  OUTPUT);
        }
        b_rn_w = 1;
    } else {
        for (i=0; i<_lcd_pins; i++) {
            pinMode(pin_b[i],  INPUT);
        }
        b_rn_w = 0;
    }
}

void lcd_wr(char rs, char data) {
    int  i;
    char d;

    if (b_rn_w == 0) {
        lcd_bus_dir(1);
    }

    if (_lcd_pins == 8) {
        d = data;
    } else {
        d = (data >> 4) & 0xF;
    }
    
    // set RS, RW
    digitalWrite(pin_rs, rs);
    digitalWrite(pin_rw, 0);
    digitalWrite(pin_e,  0);
    for (i=0; i<_lcd_pins; i++) {
        digitalWrite(pin_b[i], (d >> i) & 1);
    }
    delayMicroseconds(1);

    // pulse E
    digitalWrite(pin_e,  1);
    delayMicroseconds(1);

    digitalWrite(pin_e,  0);
    delayMicroseconds(1);

    // for the 4-bit mode, shift out the lower-nibble
    if (_lcd_pins == 4) {
        for (i=0; i<4; i++) {
            digitalWrite(pin_b[i], (data >> i) & 1);
        }
        delayMicroseconds(1);

        // pulse E
        digitalWrite(pin_e,  1);
        delayMicroseconds(1);

        digitalWrite(pin_e,  0);
        delayMicroseconds(1);
    }

}

void lcd_wr_mode4(char rs, char data) {
    int i;

    if (b_rn_w == 0) {
        lcd_bus_dir(1);
    }
    
    // set RS, RW
    digitalWrite(pin_rs, rs);
    digitalWrite(pin_rw, 0);
    digitalWrite(pin_e,  0);
    for (i=0; i<4; i++) {
        digitalWrite(pin_b[i], (data >> i) & 1);
    }
    delayMicroseconds(1);

    // pulse E
    digitalWrite(pin_e,  1);
    delayMicroseconds(1);

    digitalWrite(pin_e,  0);
    delayMicroseconds(1);
}


char lcd_rd(char rs) {
    int  i;
    char ret;

    if (b_rn_w) {
        lcd_bus_dir(0);
    }
    
    // set RS, RW
    digitalWrite(pin_rs, rs);
    digitalWrite(pin_rw, 1);
    digitalWrite(pin_e,  0);
    delayMicroseconds(1);

    // pulse E
    digitalWrite(pin_e,  1);
    delayMicroseconds(40);

    // before falling edge, grab the data
    ret = 0;
    for (i=0; i<8; i++) {
        ret |= (digitalRead(pin_b[i]) << i);
    }
    digitalWrite(pin_e,  0);
    delayMicroseconds(1);
}

void lcd_backlight(int on) {
    if (on == -1) {
        on = !b_lcd;
    }
    b_lcd = on;
    digitalWrite(pin_lcdp, on ? 1 : 0);
}

void lcd_cursor(char cursor, char cursor_blink)
{
    lcd_wr(0, 0xC | (cursor << 1) | (cursor_blink));
}

void lcd_on(int on) {
    if (on) {
        lcd_wr(0, 0xC);
        lcd_backlight(1);
    } else {
        lcd_wr(0, 0x8);
        lcd_backlight(0);
    }
}


void lcd_setup(char mode_8bit, char rs, char rw, char e, char *b, char lcdp, char lcdn) {
    int i;

    // configure things
    if (mode_8bit) {
        _lcd_pins = 8;
    } else {
        _lcd_pins = 4;
    }

    pin_rs   = rs;
    pin_rw   = rw;
    pin_e    = e;
    pin_lcdp = lcdp;
    pin_lcdn = lcdn;

    for (i=0; i<_lcd_pins; i++) {
        pin_b[i] = b[i];
    }

    pinMode(pin_rs, OUTPUT);
    pinMode(pin_rw, OUTPUT);
    pinMode(pin_e,  OUTPUT);
    
    lcd_bus_dir(1);

    pinMode(pin_lcdp,  OUTPUT);
    pinMode(pin_lcdn,  OUTPUT);

    lcd_backlight(1);

    delay(15);

    // set the data bus mode
    if (mode_8bit) {
        lcd_wr(0, 0x3C);
    } else {
        lcd_wr_mode4(0, 0x3);
        delay(10);
        lcd_wr_mode4(0, 0x3);
        delay(10);
        lcd_wr_mode4(0, 0x3);
        lcd_wr_mode4(0, 0x2);
        lcd_wr(0, 0x2C);
    }
    delay(2);
    
    // clear display
    lcd_wr(0, 0x01);
    delay(2);

    // return home
    lcd_wr(0, 0x2);
    delay(2);

    // move forward, no blinking
    lcd_wr(0, 0x4);
    delay(2);
    
    // display on, no cursor or blink
    lcd_on(1);
    delayMicroseconds(40);
}

void lcd_clr() {
   // clear display
    lcd_wr(0, 0x1);
    delay(2);
}

void lcd_chr(char c)
{
    lcd_wr(1, c);
    delayMicroseconds(43);
}

void lcd_pos(char off) {
    lcd_wr(0, 0x80 | off);
    delayMicroseconds(40);
}

void lcd_move(char ln_r, char dist)
{
    while (dist-- > 0) {
        lcd_wr(0, 0x10 | (ln_r << 2));
        delayMicroseconds(40);
    }
}

void lcd_str_pos(char *str, char off, char clr) {
    int i;

    // clear display
    if (clr) {
        lcd_wr(0, 0x1);
        delay(2);
    }

    // move to a particular offset
    lcd_pos(off);

    for (i=0; i<32; i++) {
        if (*str == 0) {
            break;
        }

        lcd_wr(1, *str);
        delayMicroseconds(43);

        str++;
    }
}
