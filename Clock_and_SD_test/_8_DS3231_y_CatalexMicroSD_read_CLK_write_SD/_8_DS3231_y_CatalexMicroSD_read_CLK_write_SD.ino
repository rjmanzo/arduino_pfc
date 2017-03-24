#include <Wire.h>
#include <SD.h>
//#include <SPI.h>
#include "ds3231.h"
#include "rtc_ds3231.h"

#define BUFF_MAX 128

uint8_t time[8];
char recv[BUFF_MAX];
char buff[BUFF_MAX];
float temp;
File myFile;
const int chipSelect = 10;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);
  Serial.println("GET time & temp");

  //Inicializacion del SD
  inicializaSD();

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
  snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d %ld", t.year, t.mon, t.mday, t.hour, t.min, t.sec, t.unixtime);
#else
  snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
#endif

  Serial.println(buff);
  Serial.println(temp);

  //save data
  myFile = SD.open("test.txt", FILE_WRITE);
  //Almacenar fecha y hora para testing
  almacenar_fecha(myFile);
  myFile.close();
  //delay(2000); //Se espera 2 segundos para seguir leyendo datos

}

/////----Inicializacion SD----/////
void inicializaSD() {
  pinMode(chipSelect, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card - initialization failed!");
    return;
  }
  Serial.println("SD Card - initialization done");
}

/////----Almacenar fecha y hora----/////
void almacenar_fecha(File myFile) {
  if (myFile) {
    // if the file opened okay, write to it:
    myFile.print(buff);
    myFile.print(temp);
    //Serial.println("Save OKKK!!");
    Serial.println(buff);
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
