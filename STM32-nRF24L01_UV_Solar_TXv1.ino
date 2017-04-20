/*

 Sensor Receiver.
 Each sensor modue has a unique ID.
 
 TMRh20 2014 - Updates to the library allow sleeping both in TX and RX modes:
      TX Mode: The radio can be powered down (.9uA current) and the Arduino slept using the watchdog timer
      RX Mode: The radio can be left in standby mode (22uA current) and the Arduino slept using an interrupt pin
 */
 /* nrf24 pins
 * |v+  |gnd | 
 * |csn |ce  |
 * |Mosi|sck |
 * |irq |miso|
 * stm32 mapping
 * v+  -> 3.3v
 * grn -> grd
 * csn -> PB1
 * ce  -> PB0
 * IRQ -> PB10
 * sck -> PA5
 * MISO-> PA6
 * MOSI-> PA7
 * 
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Set up nRF24L01 radio on SPI-1 bus (MOSI-PA7, MISO-PA6, SCLK-PA5) ... IRQ not used?
RF24 radio(PB0,PB1);

const uint64_t pipes[2] = { 0xF0F0F0F0E4LL, 0xF0F0F0F0e2LL };   // Radio pipe addresses for the 2 nodes to communicate.
//const uint8_t addresses[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};


int UVvalue;
int SolarValue;
char UVString[6];
char  SolarString[6];

char payload[13];
int i;
int j;


bool blinky = false;

void setup(){
     Serial.begin(115200);
  delay(1000);

   pinMode(PC13, OUTPUT);
   pinMode(PA3, INPUT_ANALOG);// solar
   pinMode(PA2, INPUT_ANALOG);   //uv

   
  Serial.println("\n\rRF24 Sensor Receiver");
  
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  // Setup and configure rf radio

  radio.begin();


  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setChannel(0x4c);  //channel 76
  //radio.setPALevel(RF24_PA_LOW);   //RF_SETUP        = 0x03
  radio.setPALevel(RF24_PA_MAX);   //RF_SETUP        = 0x07
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  
//delay(10000);
// wait for the serial port

  // Open pipes to other nodes for communication

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)


  radio.openWritingPipe(pipes[0]); // transmitt
  radio.openReadingPipe(1,pipes[1]);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
}

void loop(){

 byte gotByte;                                           // Initialize a variable for the incoming response
    //>>>>>>>>>>>>>>>>start of temp code >>>>>>>>>>>>>>>>>>

 
   delay(5000);

    UVvalue = analogRead(PA2);
    SolarValue = analogRead(PA3);

  //Serial.print(",\t");
  Serial.print("uv int is" );
  Serial.println(UVvalue);
  sprintf(UVString, "%d", UVvalue);
  sprintf(SolarString, "%d", SolarValue);
  Serial.print("uv string is ");
  Serial.println(UVString);
  sprintf(payload, "%d,%d", SolarValue,UVvalue);
  
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    radio.stopListening();   
  //radio.printDetails();
        blinky = !blinky;
    if (blinky){
      digitalWrite(PC13, HIGH);
    }else {
      digitalWrite(PC13, LOW);
    }

  
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ");
   if ( radio.write(&payload,sizeof(payload)) ){                         // Send the temperature to the other radio
                      
        if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
            Serial.print(F("Got blank response. round-trip delay: "));
            //delay(30000);
           //radio.printDetails();    
        }else{      
            while(radio.available() ){                      // If an ack with payload was received
                radio.read( &gotByte, 1 );                  // Read it, and display the response time
                unsigned long timer = micros();
                
                Serial.print(F("Got response "));
                Serial.print(gotByte);
                Serial.print(F(" round-trip delay: "));
               
                Serial.println(F(" microseconds"));
               
                delay(30000);
            }
        }
   }
  
}// main loop


