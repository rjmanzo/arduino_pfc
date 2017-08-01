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

// Global objects and variables   ******************************
#define POWA 9 // pin 9 supplies power to microSD card breakout and SHT15 sensor
#define MISOpin 11 // Pin Miso for SPI
#define MOSIpin 12 // Pin Miso for SPI
#define SDcsPin 10 // pin 9 is CS pin for MicroSD breakout

// Launch Variables   ******************************
uint8_t time[8];
char filename[15] = "LOGPOSTA.csv";    // Set filename Format: "12345678.123". Cannot be more than 8 characters in length, contain spaces or begin with a number
int minutos = 1; //minutos en modo sleep
File myFile; //File for SD

//*********************************************************************************************
// FUNCIONES

/////----Inicializacion SD----/////

void inicializaSD() {
  
  // pulling up the SPI lines
  //pinMode(SDcsPin, OUTPUT); digitalWrite(SDcsPin, HIGH); //pullup the CS pin
  //pinMode(MOSIpin, OUTPUT); digitalWrite(MOSIpin, HIGH);//pullup the MOSI pin
  //pinMode(MISOpin, INPUT); digitalWrite(MISOpin, HIGH); //pullup the MISO pin
  //delay(1);


  if (!SD.begin(SDcsPin)) {
    return;
  }
  else {
  }
}

/////----Almacenar datos en SD----/////
void almacenar_datos(File MyFile) {

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

}

// loop ****************************************************************

void loop()
{  
  //MODO SLEEP: CONFIGURACION------
  delay(500);

  //MODO SLEEP: CONFIGURACION------
  
  digitalWrite(POWA, LOW);  // turn off microSD card to save power
  delay(1);  // give  delay for SD card and RTC to be low before processor sleeps to avoid it being stuck

  //el tiempo maximo que la libreria permite poner el Arduino en modo sleep es
  //de 8seg, por lo que se hacen varios ciclos para lograr los minutos requeridos
  //(por ej: para 10 minutos se hacen 10*60/8=75 ciclos)   
  
  int cont=(minutos*60)/8;
  for (int i = 0; i < cont; i++) { 
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }
  //MODO DESPIERTO: CONFIGURACION,SENSADO Y ALMACENAMIENTO------

  // code will resume from here once the processor wakes up =============== //
  //LowPower.powerDown_on(ADC_ON,BOD_ON,TIMER2_ON,TIMER1_ON,TIMER0_ON,SPI_ON,USART0_ON,TWI_ON); 
  delay(50);    // important delay to ensure SPI bus is properly activated

  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH);  // turn on SD card power
  delay(50);    // give delay to let the SD card and SHT15 get full powa

  //SPI.begin();
  //Wire.begin();
 
  //Reinicializar el modulo SD
  inicializaSD();
  delay(50);

  //Reinicializo una instancia del reloj DS3231
  DS3231_init(DS3231_INTCN);
  delay(50);  

  //save data
  myFile = SD.open(filename, FILE_WRITE);
  //Almacenar fecha, hora y datos de los sensores
  almacenar_datos(myFile);

  myFile.close();  

}
