#include "SoftwareSerial.h"

#define POWA_W 6 //WIRELESS power line
# define baudios 9600

SoftwareSerial Serial1(3, 2); // RX, TX
char caracter; //variable para control de com. entre 



void setup()
  {  
    //wireless shield line up!
    pinMode(POWA_W,OUTPUT);
    digitalWrite(POWA_W,HIGH);  
    //seteo la velocidad de la consola y el puerto serial para comunicarme con la esp8266
    Serial.begin(baudios);
    Serial1.begin(baudios); 
    
  }

 void loop() {
   if (Serial1.available())
   {
   caracter = Serial1.read();
   Serial.print(caracter);
   }
   if (Serial.available())
   {
   caracter = Serial.read();
   Serial1.print(caracter);
   }
   
 }
