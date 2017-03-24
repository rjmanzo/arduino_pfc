#include "SoftwareSerial.h"
#include <Wire.h>
//#include "ds3231.h"
//ESP8266 Serial connection with Arduino-----------------
SoftwareSerial esp(3, 2); // RX Pin 3 Receptor | TX Pin 2 Transmisor

//POWER LINE --------------------------------------------
#define POWA_W 6 //WIRELESS power line
#define POWA_D 9 //WIRELESS power line

// WIFI SSID parameters----------------------------------
String ESP8266_SSID="ceneha-oeste";
String ESP8266_PASSWORD="nuevooeste";
//Server config definition-------------------------------
String server = "http://ceneha.herokuapp.com/api-reg/"; // www.example.com
//String uri = "/api-reg/";// our example is /esppost.php
String user = "datalogger";
String password = "datalogger123";

//variables de control
char cmd; //variable para control de comunicacion entre consola, esp y arduino
int baudios = 9600;
//long interval = 1000;           // server interval conection
//unsigned long previousMillis = 0; //inicializo la variable de tiempo
String data; //variable para mandar los datos al servidor
//struct ts t; //Tomo el tiempo del reloj

void setup() {

  //Datalogger line up!
  //pinMode(POWA_D,OUTPUT);
  //digitalWrite(POWA_D,HIGH);

  //wireless shield line up!
  pinMode(POWA_W,OUTPUT);
  digitalWrite(POWA_W,HIGH);

  //seteo la velocidad de la cons  ola y el puerto serial para comunicarme con la esp8266
  Serial.begin(baudios);
  esp.begin(baudios);
  //reset esp. Hay que resetearlo por q no siempre anda en el primer intento
  esp_reset();

  //establecemos la conexion wifi
  connectWifi();

  //inicializo el clock
  //Wire.begin();
  //DS3231_init(DS3231_INTCN);
  //delay(10);

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
    
    //sleep_mode();

    //bucle de envio de datos
    //if(currentMillis - previousMillis > interval){

      //actualizo la variable para el prox. ciclo
      //previousMillis = currentMillis;
      // tomo el tiempo actual
      //DS3231_get(&t);
      // Guardo el dato de fecha y hora
      //data = "Done - " + String(t.wday) +"/"+ String (t.mon) + "/" + String(t.year);
      //data += data + " - " + String(t.hour) +"/"+ String (t.min)+ "/" + String(t.sec);
      //sending data to the web
      //httppost();      
    //}
}

//reset the esp8266 module
void esp_reset() {

  esp.println("AT+RST");
  delay(1000);
  if(esp.find("OK") ) Serial.println("Module Reset");

}

//reset the esp8266 module
void sleep_mode() {

  esp.println("AT+GSLP=5000");
  delay(10);
  if(esp.find("OK") ) Serial.println("Sleep Time!");

}

//connect to your wifi network
void connectWifi() {

  String cmd = "AT+CWJAP=\"" + ESP8266_SSID + "\",\"" + ESP8266_PASSWORD + "\"";

  esp.println(cmd);
  delay(3000);
  if(esp.find("OK")) {
    Serial.println("Connected!");

  }

  else {
    
    connectWifi();
    Serial.println("Cannot connect to wifi");
    }

}

  
// publicar datos via post
void httppost () {

    esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.

    if( esp.find("OK")) {

      Serial.println("TCP connection ready");

    } delay(1000);

    String postRequest =

    "POST " + uri + " HTTP/1.0\r\n" +

    "Host: " + server + "\r\n" +

    "Accept: *" + "/" + "*\r\n" +

    "Content-Length: " + data.length() + "\r\n" +

    "Content-Type: application/x-www-form-urlencoded\r\n" +

    "\r\n" + data;

    String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.

    esp.print(sendCmd);

    esp.println(postRequest.length() );

    delay(500);

    if(esp.find(">")) { Serial.println("Sending.."); esp.print(postRequest);

    if( esp.find("SEND OK")) { Serial.println("Packet sent");

    while (esp.available()) {

      String tmpResp = esp.readString();

      Serial.println(tmpResp);

    }

    // close the connection
    esp.println("AT+CIPCLOSE");

    }

}}




