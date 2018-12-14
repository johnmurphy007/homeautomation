#include <SPI.h>        // SPI used to connect RF board to Arduino
#include <nRF24L01.h>   //RF library
#include <RF24.h>       //RF library
#include "printf.h"
#include <OneWire.h>    //Temperature I2C related
#include <DallasTemperature.h> //Temp I2C related


// ******* RF INIT ********************************************************
int ce = 7;   //  ce = pin 7
int csn = 8;  //  csn = pin 8
RF24 radio(ce,csn);
// Set up RF channel (channelIdentifier). Creating 2 channel identifers in order
// to have bidirectional comms. Each channel value can be any 5 letter string.
const byte channelIdentifier[][6] = {"node1","node2"};
// ************************************************************************

// ******* TEMPERATURE INIT ***********************************************
#define ONE_WIRE_BUS 3 // Temperature Data wire is pin 3 on the Arduino
OneWire oneWire(ONE_WIRE_BUS); // Setup oneWire to comm w/any OneWire device
DallasTemperature tempsensors(&oneWire); // Pass oneWire ref to Dallas sensor.
//Temperature sensor readings:
float t0;
float t1;
// ************************************************************************

// ******* CT INIT ********************************************************
// ************************************************************************

// ******* DATA STUCTURES *************************************************
#define NODEID        1  //unique for each node on same network
// Payload structure of RF data that will be transmitted.
typedef struct {
  int           nodeID;     //node ID - unique per Arduino sensor board
  int           sensorID;   //sensorID - e.g. 1 = temp, 2 = ct etc
  unsigned long time;       //time in ms when reading was taken
  float         var1_float; //sensor data value 1
  float         var2_float; //sensor data value 2 (optional)
} Payload;
Payload SendPayload;

//Unsure about the following var is really used (investigate)
unsigned long dev4_period_time; //seconds since last period
const unsigned long dev4_period = 5000;  //send data every X milliseconds
// ************************************************************************


void setup()
{
  Serial.begin(9600);
  radio.begin();
  radio.setAutoAck(1); // Ensure autoACK is enabled so receiver sends ack to
  //let you know the transmitted packet payload was received.
  radio.setRetries(15, 15); // Sets the delay between retries & # of retries
  //Setup Writing and Reading channel pipe. Note: writing address of 1st arduino
  //needs to be the reading address of second arduino
  radio.openWritingPipe(channelIdentifier[1]);
  radio.openReadingPipe(1, channelIdentifier[0]);
  //Ard 2:                                         // For debug
  //radio.openWritingPipe(address[0]);             // For debug
  //radio.openReadingPipe(1,  address[1]);         // For debug

  // Optionally, reduce the payload size to improve reliability
  //radio.setPayloadSize(8);
  radio.setDataRate(RF24_1MBPS); //RF24_250KBPS, RF24_1MBPS, or RF24_2MBPS
  radio.setPALevel(RF24_PA_MAX); //RF24_PA_LOW
  //radio.setCRCLength(RF24_CRC_16);  //Optional

  radio.enableDynamicAck();  // Re-read what this option does!

  radio.stopListening();

  Serial.println("Start Print Details...");       // For debug
  printf_begin();                                 // For debug
  radio.printDetails();                           // For debug
  Serial.println("...End Print Details");         // For debug
  Serial.print("Hardware is PVariant or not: ");  // For debug
  Serial.println(radio.isPVariant());             // For debug


  tempsensors.begin(); // Start I2C Temperature Sensors
}

void loop()
{

  SendPayload = getTemperature();
  sendDataRF(SendPayload);

  //TODO: Insert time logic for how often measurements are taken and transmitted.

// delay(1000);
//   SendPayload = getCT();
//     SendPayload.sensorID = 3;
//    sendDataRF(SendPayload);

  delay(5000);
}

void sendDataRF(Payload SendPayload) {
  if (radio.isChipConnected()) {
      Serial.println("Chip connected to SPI");
  } else {
      Serial.println("Chip NOT connected to SPI");
  }
  bool ok;
  ok = radio.write(&SendPayload, sizeof(SendPayload), 0);
  if (ok) {
    Serial.println("Transmitted ok");
  } else {
    Serial.println("ISSUE transmitting");
  }
}

Payload getTemperature() {
  //**** TEMPERATURE ****
  Serial.println("Requesting temperatures...");
  tempsensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  t0 = tempsensors.getTempCByIndex(0);
  t1 = tempsensors.getTempCByIndex(1);

  ///******* START OF TEST DATA ******
  //set send data
  if (isnan(t0) || isnan(t1)) {
    Serial.println("Failed to read from Temp sensors");
    } else {
    Serial.print("Temperature: ");
    Serial.print(t0);
    Serial.print(" *C\t");
    Serial.print(t1);
    Serial.print(" *C\t");

    Payload SendPayloadT;
    SendPayloadT.nodeID = NODEID;
    SendPayloadT.sensorID = 1;
    SendPayloadT.time = millis();
    SendPayloadT.var1_float = t0;
    SendPayloadT.var2_float = t1;
    Serial.println("Sending data... ");
    Serial.print("Sending struct (");
    Serial.print(sizeof(SendPayloadT));
    Serial.println(" bytes) ... ");
    return SendPayloadT;
  }
}

Payload getCT() {
  //**** CurrentTransfrom ****
  Serial.println("Requesting CT...");

  //ctdata = ;
  //if (isnan(ctdata)) {
    Serial.println("Failed to read from CT sensors");
  //  } else {

    Payload SendPayloadCT;
    SendPayloadCT.nodeID = NODEID;
    SendPayloadCT.sensorID = 2;
    SendPayloadCT.time = millis();
    SendPayloadCT.var1_float = t0;
    SendPayloadCT.var2_float = t1;
    Serial.println("Sending data... ");
    Serial.print("Sending struct (");
    Serial.print(sizeof(SendPayloadCT));
    Serial.println(" bytes) ... ");
    return SendPayloadCT;
  //}
}
