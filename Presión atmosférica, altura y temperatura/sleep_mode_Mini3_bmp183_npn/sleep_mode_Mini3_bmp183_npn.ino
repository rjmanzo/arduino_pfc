//****************************************************************

// OSBSS T/RH datalogger code Mod for Ceneha purpose - v0.01
// Last edited on May 24, 2016

//****************************************************************

#include <EEPROM.h>
#include <Wire.h>
#include <SD.h>
#include <LowPower.h>
#include <SPI.h>
#include "ds3231.h"
#include "rtc_ds3231.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP183.h>

// Global objects and variables   ******************************
#define POWA 9 // pin 9 supplies power to microSD card breakout and SHT15 sensor
#define MISOpin 11 // Pin Miso for SPI
#define MOSIpin 12 // Pin Miso for SPI
#define SDcsPin 10 // pin 9 is CS pin for MicroSD breakout

// We have conflict between the datalogger shield and the BMP183 sensor, so me decide to use a use diferent SPI configuration
#define BMP183_CLK  5 // CLK
#define BMP183_SDO  4  // AKA MISO
#define BMP183_SDI  3  // AKA MOSI
#define BMP183_CS   2 // CS

// Launch Variables   ******************************
uint8_t time[8];
char filename[15] = "LOGPOSTA.csv";    // Set filename Format: "12345678.123". Cannot be more than 8 characters in length, contain spaces or begin with a number
int minutos = 1; //minutos en modo sleep
File myFile; //File for SD

// initialize with hardware SPI
Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);

//*********************************************************************************************
// FUNCTIONES

/////----Inicializacion SD----/////

void inicializaSD() {

  if (!SD.begin(SDcsPin)) {
    Serial.println("SD not working...!");
  }
}

/////----Almacenar datos en SD----/////
void almacenar_datos(File MyFile) {

  Wire.begin();

  //Tomo el tiempo del reloj
  //-----------------------------------------------------------------------------------------
  struct ts t;
  //get time
  //En este caso el DS3231
  DS3231_get(&t);
  //-----------------------------------------------------------------------------------------
  // Almaceno los datos en la tarjeta SD
  //-----------------------------------------------------------------------------------------
  // si el archivo abrio exitosamente armo el string
  if (myFile) {
    //creando string
    String datastring = " "; //guardamos la lectura en un solo string

    // Fecha y Tiempo
    datastring += String(t.year);
    datastring += ";";
    datastring += String(t.mon);
    datastring += ";";
    datastring += String(t.wday);
    datastring += ";";
    datastring += String(t.hour);
    datastring += ";";
    datastring += String(t.min);
    datastring += ";";
    datastring += String(t.sec);
    //
    // ** Sensores: Los datos son pasados por referencia a la función

    //
    // Escribo en la SD
    myFile.println(datastring);
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("DS3231 not working...!");
  }
  //-----------------------------------------------------------------------------------------
}

// setup ****************************************************************
void setup()
{
  Serial.begin(9600); // open serial at 9600 bps
  Wire.begin();

  // set output pins
  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH);    // turn on SD card
  delay(50);    // give some delay to ensure SD is initialized properly

  //Inicializacion del SD
  inicializaSD();
  delay(50);

  //Inicializo una instancia del reloj DS3231
  DS3231_init(DS3231_INTCN);
  delay(50);

  //Inicializacion de sensores
  if (!bmp.begin()) {
    Serial.println("BMP183 not working...!");
  }

}

// loop ****************************************************************

void loop()
{

  //------Sensores----///
  /* Display atmospheric pressue in Pascals */
  Serial.print(bmp.getPressure() / 100);
  Serial.println(" millibar (hPa)");

  float temperature;
  temperature = bmp.getTemperature();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  //float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA; // should be ~1000
  float seaLevelPressure = 1019.443; // Conociendo la altura respecto al nivel del mar , ajustamos seaLevelPressure para que al calcular la altura obtener 25 m.  
  Serial.print("Sea level pressure: ");
  Serial.print(SENSORS_PRESSURE_SEALEVELHPA);
  Serial.println(" millibar/hPa");

  Serial.print("Altitude:    ");
  Serial.print(bmp.getAltitude(seaLevelPressure));
  Serial.println(" m");
  Serial.println("");

  delay(500);
  //MODO SLEEP: CONFIGURACION------
  
  digitalWrite(POWA, LOW);  // turn off microSD card to save power
  delay(1);  // give  delay for SD card and RTC to be low before processor sleeps to avoid it being stuck

  int cont = (minutos * 60) / 8;
  for (int i = 0; i < cont; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  
  //MODO DESPIERTO: CONFIGURACION,SENSADO Y ALMACENAMIENTO------

  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH);  // turn on SD card power
  delay(50);    // give delay to let the SD card and SHT15 get full powa
  
  //Reinicializar el modulo SD
  inicializaSD();
  delay(50);
  //It`s not necessary to initiate the SD module again

  //Reinicializo una instancia del reloj DS3231
  DS3231_init(DS3231_INTCN);
  delay(50);

  //save data
  myFile = SD.open(filename, FILE_WRITE);
  //Almacenar fecha, hora y datos de los sensores
  almacenar_datos(myFile);

  myFile.close();

  digitalWrite(POWA, LOW);

  if (!bmp.begin()) {
    Serial.println("BMP183 not working...!");
  }
  
}
