///Include Shield SD RTC
#include <Wire.h>
#include <RTClib.h>

//Define RTC
RTC_DS1307 rtc;
//Eegir seteo manual (0) o automatico (1)
bool set_tipo = true;


////////////--Setup--///////////
void setup() {
  Serial.begin(9600); //Se inicia la comunicación serial
  //Inicializacion de RTC
  inicializaRTC();
  Serial.println("SET OK - dd/mm/yyy HH:MM:SS");
  //Imprimir fecha y hora seteada
  imprimir_fecha();
}

////////////--loop--///////////
void loop() {

  //NULL

}

/////----Inicializacion RTC----/////
void inicializaRTC() {
  Wire.begin();//Establece la velocidad de datos del bus I2C
  rtc.begin(); //Establece la velocidad de datos del RTC

  if (! rtc.isrunning()) { // saber si el reloj externo esta activo o no
    Serial.println("RTC is NOT running!");
  }
  if (set_tipo) { //set automatico
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  else
  {
    //DateTime (year,month,day,hour,min,second)
    rtc.adjust(DateTime(2014, 1, 20, 0, 0, 0));
  }
  //Para sincronizar con el PC HACER ESTO UNA SOLA VEZ
  Serial.println("CLK seteado");
}
/////----Imprimir fecha y hora----/////
void imprimir_fecha() {
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

