#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

//#define POWA_W 6 //Seleccionamos el pin en el que se conectar´a el sensor

//float prueba;
int msg[1] ;

RF24 radio(7, 8);
const uint64_t pipe = 0xE8E8F0F0E1LL; //direccion de 64bits

void setup(void)
{   
    //Wireless lines UP!
    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH);

    Serial.begin(9600);
    radio.begin();
    radio.openWritingPipe(pipe);
}

void loop(void)
{  for (int x=0;x<2255;x++)
    {  msg[0] = x ;
          radio.write(msg, 1);
          Serial.println(msg[0]);
      }
}


