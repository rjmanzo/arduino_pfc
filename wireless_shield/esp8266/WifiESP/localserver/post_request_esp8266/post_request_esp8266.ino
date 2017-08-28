/*
 WiFiEsp example: WebClient
 This sketch connects to google website using an ESP8266 module to
 perform a simple web search.
 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp-example-client.html
*/

/*
System Data
nodo_id | nod_descrip 
---------+-------------
       1 | Nodo_0
       2 | Nodo_1
       3 | Nodo_2
       4 | Nodo_3
       5 | Nodo_4

sen_id | sen_descrip | type_sen_id | type_sen_id | type_sen_descrip 
--------+-------------+-------------+-------------+------------------
      1 | DHT22       |           1 |           1 | Temperatura
      2 | DHT22       |           2 |           2 | Humedad
      4 | BMP183      |           1 |           1 | Temperatura
      5 | MB7092      |           3 |           3 | Distancia
      3 | BMP183      |           4 |           4 | Presion
      6 | MCP9808     |           1 |           1 | Temperatura
      7 | Tensión     |           5 |           5 | Batería
 
*/

#include "WiFiEsp.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(3, 2); // RX, TX
#endif

#define POWA_W 6

char ssid[] = "Minga!";            // your network SSID (name)
char pass[] = "hj?*ara4";        // your network password
//char ssid[] = "rena";            // your network SSID (name)
//char pass[] = "123456789";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//char server[] = "ceneha.herokuapp.com";
char server[] = "192.168.1.117";

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  pinMode(POWA_W,OUTPUT);
  digitalWrite(POWA_W,HIGH);
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");
  // if you get a connection, report back via serial
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make HTTP POST request
    client.println("POST /api-reg/ HTTP/1.1");
    client.println("Host: 192.168.1.117");
    client.println("Authorization: Token 2b29d7ed651de2bc3d6bff6bab7efd4a3e8c542a");
    //client.println("User-Agent: Arduino/1.0\n");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    
    // Set up our Phant post parameters
    //char json[] = "{\"data\":5.5,\"fechahora\":\"2017-08-12 22:16:00\",\"nod_red_id\":2,\"sen_id\":1}";
    String json = "{\"data\":5.5,\"fechahora\":\"2017-08-12 22:16:00\",\"nod_red_id\":2,\"sen_id\":1}";
         
    //add content length and data
    client.print("Content-Length: "); client.println(json.length());
    client.println();
    client.print(json);

  }
}

void loop()
{
  // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client
  if (!client.connected()) {
    Serial.println();
    Serial.println("Disconnecting from server...");
    client.stop();

    // do nothing forevermore
    while (true);
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
