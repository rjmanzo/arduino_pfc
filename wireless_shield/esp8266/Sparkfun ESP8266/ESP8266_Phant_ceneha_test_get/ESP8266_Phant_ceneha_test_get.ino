/************************************************************
ESP8266_Phant.ino
SparkFun ESP8266 AT library - Phant Posting Example
Jim Lindblom @ SparkFun Electronics
Original Creation Date: July 16, 2015
https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library

This example demonstrates how to use the TCP client 
functionality of the SparkFun ESP8266 WiFi library to post 
sensor readings to a Phant stream on 
https://data.sparkfun.com

This sketch is set up to post to a publicly available stream
https://data.sparkfun.com/streams/DJjNowwjgxFR9ogvr45Q
Please don't abuse it! But feel free to post a few times to
verify the sketch works. If it fails, check the HTTP response
to make sure the post rate hasn't been exceeded.

Development environment specifics:
  IDE: Arduino 1.6.5
  Hardware Platform: Arduino Uno
  ESP8266 WiFi Shield Version: 1.0

This code is beerware; if you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, 
please buy us a round!

Distributed as-is; no warranty is given.
************************************************************/
// The SparkFunESP8266WiFi library uses SoftwareSerial
// to communicate with the ESP8266 module. Include that
// library first:
#include <SoftwareSerial.h>
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
// Phant Constants //
/////////////////////
// Phant detsination server:
const String phantServer = "ceneha.herokuapp.com";
// Phant public key:
const String uri = "/api-reg/?format=json";
// Phant private key:
//const String privateKey = "1JyPN7Mrd0UdY7x6ozxw";
const String privateKey = "Y2VuZWhhOmNlbmVoYTEyMw==";
String httpHeader = "GET " + uri + " HTTP/1.1\n" +
                    "Host: " + phantServer + "\n" +
                    "Authorization: Basic " + privateKey + "\n" +
                    "User-Agent: Arduino/1.0\n";/* + 
                    //"Content-Type: application/json;\n";                    
                    //"Content-Type: application/json; charset=UTF-8\n";*/

void setup() 
{
    //Wireless Power line up!
  pinMode(POWA_W,OUTPUT);
  digitalWrite(POWA_W,HIGH);
  
  int status;
  Serial.begin(9600);
  
  // To turn the MG2639 shield on, and verify communication
  // always begin a sketch by calling cell.begin().
  // To turn the MG2639 shield on, and verify communication
  // always begin a sketch by calling cell.begin().
  while (esp8266.begin() != true)
  {
    Serial.print("Error connecting to ESP8266.");
  //delay(1000);
  }
  
  esp8266.setMode(ESP8266_MODE_STA); // Set WiFi mode to station
  if (esp8266.status() <= 0) // If we're not already connected
  {
    if (esp8266.connect(mySSID, myPSK) < 0)
    {
      Serial.println(F("Error connecting"));
      while (1) ;
    }    
  }
  
  // Get our assigned IP address and print it:
  Serial.print(F("My IP address is: "));
  Serial.println(esp8266.localIP());
  
  Serial.println(F("Press any key to post to Phant!"));
}

void loop()
{
  // If a character has been received over serial:
  if (Serial.available())
  {
    // !!! Make sure we haven't posted recently
    // Post to Phant!
    postToPhant();
    // Then clear the serial buffer:
    while (Serial.available())
      Serial.read();
  }
}

void postToPhant()
{
  // Create a client, and initiate a connection
  ESP8266Client client;
  
  if (client.connect(phantServer, 80) <= 0)
  {
    Serial.println(F("Failed to connect to server."));
    return;
  }
  Serial.println(F("Connected."));
  
  // Set up our Phant post parameters:
  //String params;
  //params += "{\"data\":99,\"fechahora\":\"2017-04-12 22:16:00\",\"nod_red_id\":2,\"sen_id\":2}";

  //Serial.println();
  //delay(1000);
  //Serial.println();
  //delay(1000);
  
  Serial.println(F("Getting data from server..."));


  client.print(httpHeader);
  //client.print("Content-Length: "); client.println(params.length());
  client.println("Connection: close\n\n");
  //client.print(params);

  // available() will return the number of characters
  // currently in the receive buffer.
  while (client.available())
    Serial.write(client.read()); // read() gets the FIFO char
  
  // connected() is a boolean return value - 1 if the 
  // connection is active, 0 if it's closed.
  if (client.connected())
    client.stop(); // stop() closes a TCP connection.
}
