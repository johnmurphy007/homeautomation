/*
  I2C to MQTT Gateway.

  TODO:
  - Read MQTT broker address externally (sent to it, or from file?)
*/
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>         // Used for I2C communications.
#include <PubSubClient.h>

//I2C receive device address
const byte MY_ADDRESS = 42;    //I2C comms with other Arduino

bool conn_ok; // used to indicate if Mqtt connection is good or not.
bool haveData = false;  // set to true when data recevied via I2C

volatile struct
{
  int                   nodeID;
  int			              sensorID;
  unsigned long         time;
  float                 var1_float;
  float			            var2_float;
} SensorNode;


// ******* Ethernet INIT **************************************************
#define MQTT_CLIENT_ID "arduinoClient"

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrive
  Serial.print("Message arrived: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

byte server[] = { 192, 168, 1, 12 };   // IP of MQTT Broker
//byte dns[] = { 192, 168, 1, 254 };
byte mac[]    = { 0xB8, 0x28, 0xEB, 0xC3, 0xDB, 0x02 };  //Value does not matter
IPAddress ip(192, 168, 1, 16);   // Will be updated later with actual IP
EthernetClient ethClient;

PubSubClient client(server, 1883, callback, ethClient);
unsigned long keepalivetime=0;
unsigned long MQTT_reconnect=0;
// ************************************************************************



void setup() {
  Serial.begin (9600);
  Wire.begin(MY_ADDRESS);      // I2C address (slave)

  //wait for IP address
  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(3000);
  }
      // Serial.println("ETHERNET DETAILS:");                         // For Debug
      // Serial.print("IP = ");                                       // For Debug
      // Serial.println(Ethernet.localIP());                          // For Debug
      // Serial.print("The gateway IP address is: ");                 // For Debug
      // Serial.println(Ethernet.gatewayIP());                        // For Debug
      // Serial.print("The DNS server IP address is: ");              // For Debug
      // Serial.println(Ethernet.dnsServerIP());                      // For Debug
      // if (Ethernet.hardwareStatus() == EthernetNoHardware) {       // For Debug
      //   Serial.println("Ethernet shield was not found.");          // For Debug
      // } else if (Ethernet.hardwareStatus() == EthernetW5100) {     // For Debug
      //   Serial.println("W5100 Ethernet controller detected.");     // For Debug
      // }                                                            // For Debug

  Serial.println("ethernet OK");
  keepalivetime=millis();
  Wire.onReceive(receiveEvent);

  while (client.connect(MQTT_CLIENT_ID) != 1)
  {
    Serial.println("Error connecting to MQTT");
    delay(3000);
  }
  Serial.println("setup complete");
}


void loop() {
  if (haveData) {
      Serial.println("starting MQTT send");

      conn_ok = client.connected();
      if (conn_ok == 0) { //no connection, reconnect
        mqttconnect();
      }

      mqttsend(2, SensorNode.var1_float);
      delay(200);
      //Check if there is data in 'var2_float' and only send if that is the case.
      mqttsend(3, SensorNode.var2_float);
      delay(200);


      Serial.println("finished MQTT send");
      haveData = false;
  }

  //client.loop();   //client.loop needs to run every iteration.
}

void mqttconnect() {
  client.disconnect();
  delay(5000);
  while (client.connect(MQTT_CLIENT_ID) != 1)
  {
    Serial.println("Error connecting to MQTT");
    delay(4000);
  }
  Serial.println("connected to MQTT");
}

void mqttsend(int varnum, float data) {
  char buff_topic[6];           // mqtt topic
  char buff_message[12];        // mqtt message
  // buff_topic[6];
  // buff_message[7];
  sprintf(buff_topic, "%02d%01d%01d", SensorNode.nodeID, SensorNode.sensorID, varnum);
  dtostrf (data, 2, 1, buff_message);

  Serial.print("Topic = ");         //For Debug
  Serial.println(buff_topic);       //For Debug
  Serial.print("Message = ");       //For Debug
  Serial.println(buff_message);     //For Debug

  client.publish(buff_topic, buff_message);
}

// called by interrupt service routine when incoming data arrives
void receiveEvent(int howMany)
{
   if (howMany < sizeof SensorNode)
     return;

  // read into structure
  byte *p = (byte *) &SensorNode;
  for (byte i = 0; i < sizeof SensorNode; i++)
    *p++ = Wire.read();

  haveData = true;
}
