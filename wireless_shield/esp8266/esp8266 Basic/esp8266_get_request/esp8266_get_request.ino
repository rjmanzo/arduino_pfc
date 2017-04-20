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
String SERVER = "ceneha.herokuapp.com";
String URI = "/api-reg/";
String AUTH = "ZGF0YWxvZ2dlcjpkYXRhbG9nZ2VyMTIz"; //bASE64 Encode
//String SERVER = "arduino.cc";
//String URI = "/asciilogo.txt";
//variables de control
char cmd; //variable para control de comunicacion entre consola, esp y arduino
int baudios = 9600;
String data; //variable para mandar/recibir los datos al servidor

//****-----------------ZONA DE FUNCIONES ------***///

//reset the esp8266 module
void reset_conection() {
  
  Serial.println("Reseteando ESP8266");
  esp.println("AT+RST");
  delay(2000);
  //if (esp.find("ready")) Serial.println("Modulo reseteado!");
}

//reset the esp8266 module
void mode_conection() {

  String cmd ="AT+CWMODE=1\r\n"; 
  
  esp.println(cmd);
  //delay(50);
  if (esp.find("OK") ){ 
    Serial.println("Modo: Station + AP");
    }
  else{
    Serial.println("Error! modo no valido");
  }
}


//connect to your wifi network
void connectWifi() {
  
  String cmd = "AT+CWJAP=\"";
  cmd += ESP8266_SSID;
  cmd +="\",\"";
  cmd += ESP8266_PASSWORD;
  cmd += "\"";

  Serial.println(cmd);
  esp.println(cmd);
  delay(5000);
  if (esp.find("OK")) {
    Serial.println("Conectado!");
    Serial.println();
    esp.println("AT+CIFSR\r\n");
    delay(2000);

  }
  else {
    Serial.println("Cannot connect to wifi");
    connectWifi();
  }
}

//CONEXION tcp
void tcp_connection() {

  //seteo la conexion tcp en single mode
  esp.println("AT+CIPMUX=0");
  delay(10);
  if(esp.find("OK")){
    Serial.println();
    Serial.print("Single TCP Mode");
  }
  delay(2000);

  //configuro el modo de conexion a multiple
  //esp.println("AT+CIPMUX=1");
  //conexion tcp con server
  String cmd = "AT+CIPSTART=";
  cmd += "\"TCP";
  cmd += "\",\"";
  cmd += SERVER;
  cmd += "\",";
  cmd += 80;
  cmd += "\r\n";

  Serial.println(cmd);
  Serial.println();
  esp.println(cmd);
  Serial.println();
  delay(50);
  if (esp.find("OK")) {
    Serial.println("Conexion TCP Exitosa!");

  }
  else {
    Serial.println("Conexion TCP Fallida :(");
  }

}

//CONEXION tcp
void close_tcp() {
  String cmd = "AT+CIPCLOSE";
  //Serial.println(cmd);
  esp.println(cmd);
  delay(50);
  if (esp.find("CLOSED")) {
    Serial.println();
    Serial.println("Conexion TCP cerrada");
    }
}

//get_request
void get_request() { 
  
  //get request string
  String hostt = "GET " + URI;
  hostt += " HTTP/1.1\r\n";
  hostt += "Host:"+ SERVER + "\r\n";
  hostt += "Authorization: Basic ZGF0YWxvZ2dlcjpkYXRhbG9nZ2VyMTIz\r\n";
  hostt += "Connection: close\r\n\r\n";

  Serial.println(hostt);
  
  // GET /asciilogo.txt HTTP/1.0
  // Host:arduino.cc:80

  /*
  GET /api-reg HTTP/1.1\r\n
  Authorization: Basic ZGF0YWxvZ2dlcjpkYXRhbG9nZ2VyMTIz\r\n
  Host: ceneha.herokuapp.com
  */

  //Seteo la longitud del request y abro el pedido 
  String cmd = "AT+CIPSEND=";
  cmd += String(hostt.length());
  Serial.println();
  esp.println(cmd);
  Serial.println();
  delay(50); 

  
  //Si todo salio correcto, envio la peticion al servidor
  if(esp.find(">")){
    esp.println(hostt);
    delay(500);
    //Siempre y cuando se haya la consulta bien procede...
    //if(esp.find("SEND OK")){
      //Serial.println("peticion http enviada");  
      delay(2000);
      data = "";
      boolean fin_respuesta = false;
  
      //Recorro hasta encontrar un OK, CLOSED o 500 caracteres max.
      while (!fin_respuesta){
        //Muestro en el serial los datos obtenidos
        while(esp.available() > 0){
          //esp.println();
          char c = esp.read();
          Serial.write(c);
          data += c;  
          
          //respuesta finalizada
          if(esp.find("OK")){
            close_tcp();
            fin_respuesta= true;
          }
          //tiempo de respuesta agotado
          if(esp.find("CLOSED")){
            fin_respuesta= true;
          }
          //exceso de caracteres
          if(data.length()>500){
            //cierro la conexion, exceso de caracteres
            close_tcp();
            fin_respuesta= true;
          }
        }
      }
    //}
     //debug de resposta do esp8266
    //Imprime o que o esp8266 enviou para o arduino
    //Serial.println("Arduino : " + response);
    }
    
  Serial.println("Arduino : " + data);
    
  }

//**** ARDUINO ZONE ***/------------------

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

  //wait for Serial config 
  delay(1000);

  //reset esp. Hay que resetearlo por q no siempre anda en el primer intento
  reset_conection();

  //establecemos el modo de conexion
  mode_conection();
  
  //establecemos la conexion wifi
  connectWifi();

  //establecemos una conexion TCP con el server
  //tcp_connection();

  //establecemos una conexion TCP con el server
  //get_request();
  
}

void loop() {

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
  
  //establecemos una conexion TCP con el server
  tcp_connection();

  //establecemos una conexion TCP con el server
  get_request();

  //cierro la conexion tcp
  //close_tcp();


}
