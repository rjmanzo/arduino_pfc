/************************************************************
Based on ESP8266_Phant.ino / Sparfunk Library
************************************************************/
// The SparkFunESP8266WiFi library uses SoftwareSerial
// to communicate with the ESP8266 module. Include that
// library first:
#include <SoftwareSerial.h>
//Include Json Library to make json object
#include <ArduinoJson.h>
// Include the ESP8266 AT library:
#include <SparkFunESP8266WiFi.h>
// Wireless power pin
#define POWA_W 6

//////////////////////////////
// WiFi Network Definitions //
//////////////////////////////
// Replace these two character strings with the name and
// password of your WiFi network.
const char mySSID[] = "Minga!";
const char myPSK[] = "hj?*ara4";
/////////////////////
// Server Constants //
/////////////////////
// destination server:
const String SERVER = "ceneha.herokuapp.com";
const String URI = "/api-reg/";
// ceneha key access
//const String KEY = "ZGF0YWxvZ2dlcjpkYXRhbG9nZ2VyMTIz";
const String KEY = "ceneha:ceneha123";
String httpHeader = "POST " + URI + "HTTP/1.1\n" + 
                    "Host: " + SERVER + "\r\n" +
                    "Autorization: Basic " + KEY + "\r\n" +
                    "Connection: close\n" + 
                    "Content-Type: application/json\n"; 

//Genero un buffer dinamico para el objeto Json 
//DynamicJsonBuffer jsonBuffer;

// JsonBuffer with all the other nodes of the object tree.
// Memory is freed when jsonBuffer goes out of scope.
//JsonObject& root = jsonBuffer.createObject();  

//String Data
String payload; 

void setup() 
{
  //Wireless Power line up!
  pinMode(POWA_W,OUTPUT);
  digitalWrite(POWA_W,HIGH);
  
  int status;
  Serial.begin(9600);

  delay(1000);  
    
  //ESP8266 init procress 
  while (esp8266.begin() != true)
  {
    Serial.println(F("Error connecting to ESP8266...."));
    delay(500);
  }
  Serial.println("Init...DONE");

  //Seteo el modo de funcionamiento de la placa ESP en modo estacion
  esp8266.setMode(ESP8266_MODE_STA); // Set WiFi mode to station
  //if (esp8266.status() <= 0) // If we're not already connected
  //{
  //  if (esp8266.connect(mySSID, myPSK) < 0)
  //  {
  //    Serial.println(F("Error connecting.."));
  //  }    
  //}

 if (esp8266.status() <= 0)
  {
    while (esp8266.connect(mySSID, myPSK) < 0){}
  }
  delay(1000);
  Serial.println(F("Connected to WiFi"));
  delay(2000);

  // Get our assigned IP address and print it:
  //Serial.print(F("My IP address is: "));
  Serial.print(esp8266.localIP());
  delay(2000);

  //Json part-------

  payload = "{\"data\":5,\"fechahora\":\"2017-04-12 22:16:00\",\"nod_red_id\":2,\"sen_id\":2}";

  // Add values in the object
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root.set<long>("time", 1351824120);
  //root["data"] = 25;
  //root["fechahora"] = "2017-04-12 22:16:00";
  //root["nod_red_id"] = 2;
  //root["sen_id"] = 2;

  //data = root.printTo(Serial);
  // This prints:
  // {"data": 25,"fechahora":"2017-04-12T22:16:00","nod_red_id":2, "sen_id":2}
  Serial.println(payload);
  
  Serial.println(F("Press any key to post to Phant!"));
}

void loop()
{
  // If a character has been received over serial:
  if (Serial.available())
  {
    // !!! Make sure we haven't posted recently
    // Post to Phant!
    PostRequest();
    // Then clear the serial buffer:
    while (Serial.available())
      Serial.read();
  }
}

void PostRequest()
{
  // Create a client, and initiate a connection
  ESP8266Client client;
   
  while (client.connect("ceneha.herokuapp.com", 80) <= 0)
  {
    Serial.println("Failed to connect to server.");
    return;
  }
  Serial.println("Connected.");
  
  // Set up our Phant post parameters:
  //String params;
  //params += String(root.printTo(Serial));
  
  
  Serial.println("Posting to...!");

  client.print(httpHeader);
  client.print("Content-Length: "); client.print(payload.length());
  //client.print("Content-Length: "); client.println(root.measureLength());
  //client.println();
  client.println(payload);
  char c;

  // available() will return the number of characters
  // currently in the receive buffer.
  while (client.available()){
    char c = client.read();
    Serial.print(c); // read() gets the FIFO char
  }
  // connected() is a boolean return value - 1 if the 
  // connection is active, 0 if it's closed.
  if (client.connected())
    client.stop(); // stop() closes a TCP connection.
    Serial.println("Conexion cerrada");
}
