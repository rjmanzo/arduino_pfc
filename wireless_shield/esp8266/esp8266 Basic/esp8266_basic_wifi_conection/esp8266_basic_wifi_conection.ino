#include "SoftwareSerial.h"
#include <Wire.h>
//#include "ds3231.h"
//ESP8266 Serial connection with Arduino-----------------
SoftwareSerial esp(3, 2); // RX Pin 3 Receptor | TX Pin 2 Transmisor

//POWER LINE --------------------------------------------
#define POWA_W 6 //WIRELESS power line

// WIFI SSID parameters----------------------------------
String ESP8266_SSID = "Minga!";
String ESP8266_PASSWORD = "hj?*ara4";
//Server config definition-------------------------------
String SERVER = "http://ceneha.herokuapp.com";
//Integer PORT = 80;
String TYPE = "TCP";
//String server = "http://ceneha.herokuapp.com/api-reg/"; // www.example.com
//String uri = "/api-reg/";// our example is /esppost.php
//String user = "datalogger";
//String password = "datalogger123";

//variables de control
char cmd; //variable para control de comunicacion entre consola, esp y arduino
int baudios = 9600;
String data; //variable para mandar los datos al servidor

void setup() {

  //Datalogger line up!
  //pinMode(POWA_D,OUTPUT);
  //digitalWrite(POWA_D,HIGH);

  //wireless shield line up!
  pinMode(POWA_W, OUTPUT);
  digitalWrite(POWA_W, HIGH);

  //seteo la velocidad de la cons  ola y el puerto serial para comunicarme con la esp8266
  Serial.begin(baudios);
  esp.begin(baudios);
  //reset esp. Hay que resetearlo por q no siempre anda en el primer intento
  esp_reset();

  //establecemos la conexion wifi
  //connectWifi();

  //establecemos una conexion TCP con el server
  //tcp_connection();

}

void loop() {

  // check time variable
  //unsigned long currentMillis = millis();

  //habilito el puerto serial de Arduino para comunicarme con la placa y así controlar el sistema via consola
  if (esp.available())
  {
    cmd = esp.read();
    Serial.print(cmd);
  }
  if (Serial.available())
  {
    cmd = Serial.read();
    esp.print(cmd);
  }
}

//reset the esp8266 module
void esp_reset() {

  esp.println("AT+RST");
  delay(3000);
  if (esp.find("OK") ) Serial.println("Module Reset");

}

//connect to your wifi network
void connectWifi() {

  String cmd = "AT+CWJAP=\"" + ESP8266_SSID + "\",\"" + ESP8266_PASSWORD + "\"";

  esp.println(cmd);
  delay(5000);
  if (esp.find("OK")) {
    Serial.println("Conectado a:");
    

  }
  else {
    Serial.println("Cannot connect to wifi");
    connectWifi();
  }

}

//CONEXION tcp
void tcp_connection() {

  char cmd = "AT+CIPSTART=\"TCP""\",\"""http://ceneha.herokuapp.com""\",80\r\n";

  Serial.println(cmd);
  esp.println(cmd);
  //esp.println("AT+CIPSTART="+"TCP"+","+"http://ceneha.herokuapp.com"+,+80);
  Serial.println(cmd);
  delay(1000);
  if (esp.find("CONNECT")) {
    Serial.println("Conexion TCP Exitosa!");

  }
  else {
    Serial.println("Conexion TCP Fallida :(");
  }

}

