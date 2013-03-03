//////////////////////////////////////////////////////////////////////////////////////////
// MPR stuff
//////////////////////////////////////////////////////////////////////////////////////////

#include "mpr121.h"
#include <I2C.h>

int     irq_pin    = 7;
boolean touchStates[12];     // keep track of the previous touch states
char   *touchChars = "0123456789*#";

void mpr121_set_register(byte r, byte v){
    byte b;

    I2c.write((byte)MPR_I2C_ADDR, r, v);
    delay(10);
    I2c.read((byte)MPR_I2C_ADDR, (byte)r, (byte)1);
    delay(10);
    while (!I2c.available()) {
    }
     
    b = I2c.receive();
    if (b != v) {
        //printf("reg_write:  %02x write=%02x read=%02x\n", r, v, b);
    }
}

// dump filtered electrode data
byte mpr121_get_byte(byte addr) {
    byte c = 0, b;

    I2c.read((byte)MPR_I2C_ADDR, addr, (byte)1);
    while (I2c.available() && c < 1) {
        b = I2c.receive();
        c++;
    }
    if (c != 1) {
        printf("E: read %d bytes\n", c);
    }
    return b;
}

#define set_register mpr121_set_register
#define get_byte     mpr121_get_byte

void mpr121_setup(void) {
    int i;

    // configure IRQ pin as pulled up
    pinMode(irq_pin, INPUT);
    digitalWrite(irq_pin, 1);

    set_register(ELE_CFG, 0x00); 
    
    // Section A - Controls filtering when data is > baseline.
    set_register(MHD_R, 0x01);                                         // AN3891
    set_register(NHD_R, 0x01);
    set_register(NCL_R, 0x00);
    set_register(FDL_R, 0x00);

    // Section B - Controls filtering when data is < baseline.
    set_register(MHD_F, 0x01);                                         // AN3891
    set_register(NHD_F, 0x01);
    set_register(NCL_F, 0xFF);
    set_register(FDL_F, 0x02);
  
    // Section C - Sets touch and release thresholds for each of 12 electrodes - electrode registers are consequtive
    for (i=0; i<12; i++) {
        set_register(ELE0_T+2*i, TOU_THRESH);                                  // AN3892
        set_register(ELE0_R+2*i, REL_THRESH);
    }
  
    // Section D
    // Set the Filter Configuration
    // Set ESI2
    set_register(FIL_CFG, 0x24);
  
    // Section E
    // Electrode Configuration
    // Set ELE_CFG to 0x00 to return to standby mode
    //set_register(ELE_CFG, 0x0C);  // Enables all 12 Electrodes
      
    //mpr121_dump_regs();
  
  // Section F
  // Enable Auto Config and auto Reconfig                - AN3889
  //set_register(ATO_CFG0, 0x0B);
  //set_register(ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V - from datasheet
  //set_register(ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V          - from datasheet
  //set_register(ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V        - from datasheet

    //printf("AFE:  %02x\n", get_byte(AFE_CFG));
    set_register(AFE_CFG, 0x10);    // change current level from 16 to 32 uA
    //printf("AFE:  %02x\n", get_byte(AFE_CFG));
  
    // enter run state 
    // copy initial values into baseline register
    set_register(ELE_CFG,  0xCC);    
}

//
// calibrate the touchpad to have the average idle value within the following
// range and tolerance
//
void mpr121_calib(int val, int tol) {
    byte           addr;
    int            i;
    int            c;
    unsigned short w;
    int            b;

    // collect filtered
    for (i=0; i<12; i++) {
        addr = 4 + i*2;
        I2c.read((byte)MPR_I2C_ADDR, addr, (byte)2);
        c = 0;
        w = 0;
        while (I2c.available() && c < 2) {
            b = I2c.receive();
            w |= (b << c*8);
            c++;
        }
        if (c != 2) {
            printf("E: read %d bytes\n", c);
        }
        printf("%04x  ", w);
    }
    printf("\n");
}

void wakeUp();

void mpr121_isr_read_inputs() {
    byte     LSB;
    byte     MSB;
    uint16_t touched;

    //printf("In ISR\n");
    wakeUp();
        
    //read the touch state from the MPR121
    I2c.read(MPR_I2C_ADDR, 0, 2);
    
    LSB = I2c.receive();
    MSB = I2c.receive();
    
    touched = ((MSB << 8) | LSB); //16bits that make up the touch states

    //if (touched) {
    //    printf("Touched:  %02x\n", touched);
    //}
    //xreturn;
    
    for (int i=0; i < 12; i++) {  // Check what electrodes were pressed
        if (touched & (1<<i)) {
      
            if(touchStates[i] == 0){
                //pin i was just touched
                //Serial.print("pin ");
                //Serial.print(i);
                //Serial.println(" was just touched");
                printf("%c", touchChars[i]);
        
            } else if (touchStates[i] == 1){
                //pin i is still being touched
            }  
      
            touchStates[i] = 1;      
        } else {
            if (touchStates[i] == 1){
                //Serial.print("pin ");
                //Serial.print(i);
                //Serial.println(" is no longer being touched");
          
                //pin i is no longer being touched
            }
        
            touchStates[i] = 0;
        }    
    }
}

int mpr121_read_char() {
    byte     LSB;
    byte     MSB;
    uint16_t touched;
    int      i;
        
    //read the touch state from the MPR121
    I2c.read(MPR_I2C_ADDR, 0, 2);
    
    LSB = I2c.receive();
    MSB = I2c.receive();
    
    touched = ((MSB << 8) | LSB); //16bits that make up the touch states

    //if (touched) {
    //    printf("Touched:  %02x\n", touched);
    //}
    //xreturn;
    
    for (i=0; i<12; i++) {  // Check what electrodes were pressed
        if (touched & (1<<i)) {      
            if (touchStates[i] == 0){
                touchStates[i] = 1;      
                return touchChars[i];       
            }
        } else {
            touchStates[i] = 0;
        }    
    }

    return -1;
}


void mpr121_dump_regs() {
    byte addr = 0;
    byte b;

    while (addr < 128) {
        I2c.read((byte)MPR_I2C_ADDR, (byte)addr, (byte)1);
        while (I2c.available()) {
            b = I2c.receive();
            printf("%02x:  ", addr);
            if (b == 0) {
                printf(".\n");
            } else {  
                printf("%02x\n", (unsigned byte)b);
            }
            addr++;            
        }
    }      
}

void mpr121_dump_data() {
    byte i;
    int c;
    unsigned int w;
    byte b;
    byte addr;

    for (i=0; i<12; i++) {
        printf("%4d  ", i); 
    }
    printf("\n");

    // dump filtered electrode data
    for (i=0; i<12; i++) {
        addr = 4 + i*2;
        I2c.read((byte)MPR_I2C_ADDR, addr, (byte)2);
        c = 0;
        w = 0;
        while (I2c.available() && c < 2) {
            b = I2c.receive();
            w |= (b << c*8);
            c++;
        }
        if (c != 2) {
            printf("E: read %d bytes\n", c);
        }
        printf("%04x  ", w);
    }
    printf("\n");

    // dump baseline electrode data
    for (i=0; i<12; i++) {
        I2c.read((byte)MPR_I2C_ADDR, (byte)(0x1e + i), (byte)1);
        c = 0;
        w = 0;
        while (I2c.available() && c < 1) {
            b = I2c.receive();
            w |= (b << c*8);
            c++;
        }
        if (c != 1) {
            printf("E: read %d bytes\n", c);
        }
        
        // baseline regs contain 8MSBs
        w = (w << 2);

        printf("%04x  ", w);
    }
    printf("\n\n");

}

void mpr121_isr(byte on, void (*f)(void)) {
    // register an interrupt
    //printf("registering ISR %d\n", on);
    if (on) {
        // clear any pending status
        attachInterrupt(4, f, FALLING);
        mpr121_read_char();
    } else {
        detachInterrupt(4);
    }
}
