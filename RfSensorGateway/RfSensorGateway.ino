#include <SPI.h>        // SPI used to connect RF board to Arduino
#include <nRF24L01.h>   //RF library
#include <RF24.h>       //RF library
#include "printf.h"
#include <Wire.h>       // I2C communications

// ******* RF INIT ********************************************************
int ce = 7;   //  ce = pin 7
int csn = 8;  //  csn = pin 8
RF24 radio(ce,csn);
// Set up RF channel (channelIdentifier). Creating 2 channel identifers in order
// to have bidirectional comms. Each channel value can be any 5 letter string.
const byte channelIdentifier[][6] = {"node1","node2"};
// ************************************************************************

int TARGET_I2C_ADDRESS = 42;

typedef struct {
  int             nodeID;
  int			        sensorID;
  unsigned long   time;
  float           var1_float;
  float			      var2_float;
} Payload;
Payload theData;

void setup()
{
  while (!Serial);
  Serial.begin(9600);//300,600,1200,2400,4800,9600,14400,19200,28800,38400,57600,115200

  radio.begin();
  radio.setAutoAck(1); // Ensure autoACK is enabled so receiver sends ack to let
  //you know it got the transmitted packet payload

  radio.openWritingPipe(channelIdentifier[0]);
  radio.openReadingPipe(1,  channelIdentifier[1]);

  radio.startListening();

  Serial.println("Start Print Details...");         // For debug
  printf_begin();                                   // For debug
  radio.printDetails();                             // For debug
  Serial.println("...End Print Details");           // For debug

  Wire.begin(); //start I2C as master (no address)
}

void loop()
{
  if (radio.isChipConnected()) {
        Serial.println("Chip connected to SPI");    // For debug
  } else {
        Serial.print("Chip NOT connected to SPI");  // For debug
  }

  if (radio.available()) {
    radio.read( &theData, sizeof(theData) );
    // Spew it
    Serial.print("NodeID = ");          // For debug
    Serial.println(theData.nodeID);     // For debug
    Serial.print("sensorID = ");        // For debug
    Serial.println(theData.sensorID);   // For debug
    Serial.print("var1_usl = ");        // For debug
    Serial.println(theData.time);       // For debug
    Serial.print("var2_float = ");      // For debug
    Serial.println(theData.var1_float); // For debug
    Serial.print("var3_float = ");      // For debug
    Serial.println(theData.var2_float); // For debug

    Serial.println("Sending data via I2C");
    Wire.beginTransmission(TARGET_I2C_ADDRESS);
    Wire.write((byte *) &theData, sizeof theData);
    Wire.endTransmission();
    Serial.println("Finished sending data to I2C");
  }
}
