///Include Shield SD RTC
#include <Wire.h>
#include <RTClib.h>
 
//Define SD RTC
RTC_DS1307 rtc;
  
////////////--Setup--///////////
void setup() {
  Serial.begin(9600); //Se inicia la comunicación serial 
 
 //Inicializacion de RTC
 inicializaRTC();
 
 Serial.println("dd/mm/yyy HH:MM:SS");
 delay(2000); //espero 2 sg antes de empezar a leer
} 

////////////--loop--/////////// 
void loop() {
    //Imprimir fecha y hora
    imprimir_fecha();
    delay(1000); //espero un sg para la prox. lectura
} 
 
/////----Inicializacion RTC----/////
void inicializaRTC(){
 Wire.begin();//Establece la velocidad de datos del bus I2C
 rtc.begin(); //Establece la velocidad de datos del RTC
 
  if (! rtc.isrunning()) { // saber si el reloj externo esta activo o no
    Serial.println("RTC is NOT running!");
    //Para sincronizar con el PC HACER ESTO UNA SOLA VEZ
  // RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}
 
//////////////-----------------------///////////////
/////----Imprimir fecha y hora----///// 
void imprimir_fecha(){
    DateTime now = rtc.now(); 
    Serial.print(now.year());
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.print(now.day());
    Serial.print(' ');
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(':');
    Serial.print(now.second());
    Serial.print(", ");
}
 
