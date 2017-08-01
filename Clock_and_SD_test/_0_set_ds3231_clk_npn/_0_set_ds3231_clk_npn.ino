// Date and time functions using a RX8025 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "ds3231.h"
//#include "rtc_ds3231.h"
#include <Time.h>

#define POWA 9 // pin 9 power suplite line

//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also.
//DateTime dt(2011, 11, 10, 15, 18, 0, 5);

void setup ()
{
  Serial.begin(9600);
  Wire.begin();

  //POWER LINE CONFIG
  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH); //HABILITO LA LINEA POWER ENVIANDO UN HIGH A LA BASE DEL TRANSISTOR
  delay(50);

  DS3231_init(DS3231_INTCN);
  //RTC CONFIG
  rtc.begin();
  //rtc.setDateTime(DateTime(__DATE__, __TIME__));
  DateTime now = rtc.now();
  struct ts t = {now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()};
  DS3231_set(t);
  //rtc.setDateTime(DateTime(__DATE__, __TIME__));
  //void DS3231::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
  //rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above
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
  //Serial.print(weekDay[now.dayOfWeek()]);
  //Serial.println();
  delay(500);
}
