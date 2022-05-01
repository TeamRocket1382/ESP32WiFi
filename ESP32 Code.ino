#include <SPI.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <RH_RF95.h>
#define RFM95_CS 4
#define RFM95_RST 34
#define RFM95_INT 25
#define RF95_FREQ 915.0
#define LED 13

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Enter your WiFi SSID and password
char ssid[] = "AndroidAP143B";             // your network SSID (name)
char pass[] = "bbhn3083";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

#define SERVER "bustrackerteam1382.herokuapp.com"
#define PATH   "/new?password=Team1382"
void setup() {
// Singleton instance of the radio driver

  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  while (!Serial); // wait until serial console is open, remove if not tethered to computer
  Serial.begin(115200);    
  WiFi.begin(ssid, pass);
   // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Connected to WiFi");
printWifiStatus();
  

  delay(100);
  Serial.println("Feather LoRa RX Test!");
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
    while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}
 int16_t packetnum = 0;
 uint32_t bytes = 0;
void loop()
{ 
  delay(5000); // Wait 1 second between transmits, could also 'sleep' here!
  WiFiClientSecure client;
  client.setInsecure();
   Serial.println("\nStarting connection to server...");
 

  // if you get a connection, report back via serial:
  if (client.connect(SERVER, 443)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET " PATH " HTTP/1.1");
    client.println("Host: " SERVER);
    client.println("Connection: close");
    client.println();
  }
   char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);


  return;
  }
   client.find("\r\n\r\n", 4);

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.

  StaticJsonDocument<400> doc;
  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract values
  JsonObject root_0 = doc[0];
  //JsonObject root_0 = doc;
  Serial.println(F("Response:"));
  int busID = doc["busID"]; // 883
int status1 = doc["status"]; // 1
int duration = doc["duration"]; // 4
  client.stop();
  char str_temp[16];
  char str_temp2[16];
  //Converts BUSID, switc position, latitude and longitude into char array and assign it to the packet to be transmitted.
 // dtostrf(busID,4,4,str_temp);
  //dtostrf(duration,4,4,str_temp2);
  char mydata[22];
  sprintf(mydata, "%d,%d,%d",busID,status1,duration);
  Serial.println("Transmitting..."); // Send a message to rf95_server
  
  char radiopacket[20] = "Deez nuts #      ";
  itoa(packetnum++, radiopacket+13, 10);
  Serial.print("Sending "); Serial.println(radiopacket);
  radiopacket[19] = 0;
  
  Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)mydata, 22);

  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
