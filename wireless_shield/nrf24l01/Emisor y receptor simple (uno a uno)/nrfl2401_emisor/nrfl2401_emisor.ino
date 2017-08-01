#include <RF24_config.h>
#include <SPI.h>
#include <Wire.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(7,8);
const uint64_t pipes[2] = { 0xE8E8F0F0E1LL, 0xF0F0F0F0D2LL }; //canales de transmisión: para escribir y leer
int msg;


void setup(void)
{   
    //Wireless lines UP!
    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH);

    //pinout por las dudas
    pinMode(10, OUTPUT);
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);

    Serial.begin(9600);
    
    radio.begin();
    
    radio.openWritingPipe(pipes[0]); // Abrir para escribir
    radio.openReadingPipe(1,pipes[1]);
    // Set the PA Level low to prevent power supply related issues since this is a
    // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
    radio.setPALevel(RF24_PA_LOW);

    radio.startListening();                                  // First, stop listening so we can talk.

}

void loop(void)
{  
  radio.stopListening();  
  for (int x=0;x<255;x++)
    {  msg = x ;
       if(radio.write(&msg,sizeof(msg))){
         Serial.println(msg);       
       }
     }  
}


