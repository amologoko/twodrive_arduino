#include <Wire.h>

//   int ledPin = 13;                 // LED connected to digital pin 13
//   
//   void setup()
//   {
//     pinMode(ledPin, OUTPUT);      // sets the digital pin as output
//     Serial.begin(9600);
//     Wire.begin();
//     
//     Serial.println("Booting 2Drive...  Version 1");
//   }
//   
//   int i=0;
//   
//   void loop()
//   {
//     digitalWrite(ledPin, HIGH);   // sets the LED on
//     delay(1000);                  // waits for a second
//     digitalWrite(ledPin, LOW);    // sets the LED off
//     delay(1000);                  // waits for a second  
//   //  Serial.println(i);
//   //  i++;
//   }
//   
//   

#include <SoftwareSerial.h>

SoftwareSerial serial_elm(10, 11); // RX, TX

int elm_cmd(char *cmd, int send_at, int timeout_ms) 
{
    char c;
    char p;
    unsigned int t_start, t_end;

    // clean up any junk
    while (1) {
        c = serial_elm.read();
        if (c == -1) {
            break;
        }
    }  

    serial_elm.println(cmd);
    Serial.println(cmd);
    delay(1);

    t_start = millis();
    while (1) {
        if (send_at) {
            serial_elm.println("AT");
            delay(1);
        }

        p = 0;
        while (1) {
            c = serial_elm.read();
            if (c == -1) {
                break;
            }

            Serial.print(c);
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

void elm_setup() 
{
    serial_elm.begin(9600);
    digitalWrite(10, 1);      // sets the digital pin as output

    // get a prompt
    elm_cmd("AT", 0, 10000);

    // received prompt
    elm_cmd("ATL1",   0, 10000);
    elm_cmd("ATH1",   0, 10000);
    elm_cmd("ATCAF0", 0, 10000);

}

void elm_prius_lock(int on) {
    int i;

    elm_cmd("ATSH631", 0, 10000);

    for (i=0; i<2; i++) {
        if (on) {
            elm_cmd("18 80 53 02 00 09 00", 0, 10000);
        } else {
            elm_cmd("18 80 53 04 00 19 00", 0, 10000);
        } 
    }
}

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("ELM327 test");

  pinMode(7, INPUT);
  pinMode(6, OUTPUT);
  digitalWrite(7, 1);
  digitalWrite(6, 0);

  elm_setup();
}

int lock = 0;

void loop() // run over and over
{
  int s;
  if (digitalRead(7) != lock) {
      delay(1);
      s = digitalRead(7);
      if (s != lock) {
          lock = s;
          elm_prius_lock(lock);
      }
  }
}
