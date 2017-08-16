// Date and time functions using a RX8025 RTC connected via I2C and Wire lib

//#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <TimeLib.h>
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include "ds3231.h"

#define POWA 9 // pin 9 power suplite line

void set_datetime();
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec); //sobrecarga

void setup ()
{

  //POWER LINE CONFIG
  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH); //HABILITO LA LINEA POWER ENVIANDO UN HIGH A LA BASE DEL TRANSISTOR

  Serial.begin(9600);
  Wire.begin();

  DS3231_init(DS3231_INTCN); // Init clock
  delay(50);

  //Setting datetime - Manual or PC mode 
  set_datetime();
  //set_datetime(2017,10,2,23,15,00);

}

void loop ()
{

  struct ts t;
  DS3231_get(&t);
  //DateTime now = rtc.now(); //get the current date-time
  Serial.print(t.year, DEC);
  Serial.print('/');
  Serial.print(t.mon, DEC);
  Serial.print('/');
  Serial.print(t.mday, DEC);
  Serial.print(' ');
  Serial.print(t.hour, DEC);
  Serial.print(':');
  Serial.print(t.min, DEC);
  Serial.print(':');
  Serial.print(t.sec, DEC);
  Serial.println();
  delay(500);  
 
}

//This function
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec ) {

  struct ts t;
  t= {sec,minutes,hours,days,mon,years,0,0,0};
  DS3231_set(t);

}

void set_datetime() { //setting by computer clock

  struct ts t; //struct time
  t= {second(),minute(),hour(),day(),month(),year(),weekday(),0,0};
  DS3231_set(t); //setting the clock
  
}





