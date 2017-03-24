
#include <Wire.h>
#include "ds3231.h"
#include "rtc_ds3231.h"

#define BUFF_MAX 128

uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;
unsigned long prev, interval = 5000;
float temp;

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    DS3231_init(DS3231_INTCN);
    memset(recv, 0, BUFF_MAX);
    Serial.println("GET time & temp");
}

void loop()
{
    char in;
    char buff[BUFF_MAX];
    unsigned long now = millis();
    struct ts t;

      //get time
      DS3231_get(&t);
      //get temperature
      temp = DS3231_get_treg();

        // there is a compile time option in the library to include unixtime support
#ifdef CONFIG_UNIXTIME
        snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d %ld", t.year,
             t.mon, t.mday, t.hour, t.min, t.sec, t.unixtime);
#else
        snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year,
             t.mon, t.mday, t.hour, t.min, t.sec);
#endif

        Serial.println(buff);
        Serial.println(temp);
       
}


