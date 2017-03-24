#include "SoftwareSerial.h"
//ESP8266 Soft. Serial connection with Arduino-----------------
SoftwareSerial esp(3, 2); // RX Pin 3 Receptor | TX Pin 2 Transmisor

#define POWA_W 6 //WIRELESS power line
#define POWA_D 9 //DATALOGGER power line
# define baudios 9600
char caracter; //variable para control de com. entre 

float voltaje (){
 // the loop routine runs over and over again forever
  // read the input on analog pin 0:
  analogReference(INTERNAL);
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = (sensorValue * 4.675) / 1024.0;
  // print out the value you read:
  return voltage;
}

void setup()
  {  
    //seteo la velocidad de la consola y el puerto serial para comunicarme con la esp8266
    Serial.begin(baudios);
    esp.begin(baudios);
    //wireless shield line up!
    pinMode(POWA_W,OUTPUT);
    digitalWrite(POWA_W,HIGH);   
    //datalogger shield line up!
    pinMode(POWA_D,OUTPUT);
    digitalWrite(POWA_D,HIGH);
    
  }

 void loop() {
   if (esp.available())
   {
   caracter = esp.read();
   Serial.print(caracter);
   }
   if (Serial.available())
   {
   caracter = Serial.read();
   esp.print(caracter);
   }

   //Mido el voltaje
   Serial.println(voltaje());
   
 }
