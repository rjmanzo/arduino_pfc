///Include Shield SD RTC
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>
 
//Define SD RTC
File myFile;
RTC_DS1307 rtc;
const int chipSelect = 8;
  
////////////--Setup--///////////
void setup() {
  Serial.begin(9600); //Se inicia la comunicación serial 
 
 //Inicializacion de RTC
 inicializaRTC();
 
 //Inicializacion del SD
 inicializaSD();
 
 Serial.println("Lectura y escritura de la fecha en formato: dd/mm/yyy HH:MM:SS");  
 delay(2000);
}
 
 
////////////--loop--/////////// 
void loop() {
    //Imprimir fecha y hora
    imprimir_fecha();
 
    myFile = SD.open("test14.txt", FILE_WRITE);
 
   //Almacenar fecha y hora
    almacenar_fecha(myFile);
 
   myFile.close();
   delay(2000); //Se espera 2 segundos para seguir leyendo datos
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
 
 
/////----Inicializacion SD----/////
void inicializaSD(){
  pinMode(10, OUTPUT);
 
  if (!SD.begin(chipSelect)) {
        Serial.println("SD Card - initialization failed!");
        return;
  }
  Serial.println("SD Card - initialization done");
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
 
/////----Almacenar fecha y hora----/////  
void almacenar_fecha(File myFile){
if (myFile) {
    // if the file opened okay, write to it:
    DateTime now = rtc.now();
    myFile.print(now.year());
    myFile.print('/');
    myFile.print(now.month());
    myFile.print('/');
    myFile.print(now.day());
    myFile.print(' ');
    myFile.print(now.hour());
    myFile.print(':');
    myFile.print(now.minute());
    myFile.print(':');
    myFile.print(now.second());
    myFile.print(", ");
    Serial.println("Save OKKK!!");
   }
  else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
  }
}
