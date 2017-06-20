#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

RF24 radio(7, 8);
const uint64_t pipe = 0xE8E8F0F0E1LL;
int msg[1];

void setup(void)
   {  
      //Wireless lines UP!
      pinMode(6, OUTPUT);
      digitalWrite(6, HIGH);

      //HAY UN PROBLEMA EN LA ALIMENTACIÓN Y PASA ENERGIA HACIA EL PIN QUE CONTROLA EL SHIELD DATALOGGER. POR ESO LO PONEMOS EN BAJA
      //pinMode(9, OUTPUT);
      //digitalWrite(9, LOW);
      Serial.begin(9600); 
      radio.begin();
      radio.openReadingPipe(1,pipe);
      radio.startListening();
   }

void loop(void)
   {   if (radio.available())
          {   
            radio.read(msg, 1);
            int dato = msg[0];
            Serial.println(dato);
          }
   }

