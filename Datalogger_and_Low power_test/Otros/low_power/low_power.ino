//****************************************************************

// OSBSS T/RH datalogger code Mod for Ceneha purpose - v0.01
// Last edited on May 24, 2016

//****************************************************************

#include <EEPROM.h>
#include <Wire.h>
#include <SD.h>
#include <PowerSaver.h>
#include <stdio.h>
//#include <SdFat.h>
#include <SPI.h>
#include "ds3231.h"
#include "rtc_ds3231.h"

// Launch Variables   ******************************
uint8_t time[8];
char filename[15] = "log.csv";    // Set filename Format: "12345678.123". Cannot be more than 8 characters in length, contain spaces or begin with a number

// Global objects and variables   ******************************
#define POWA 8    // pin 4 supplies power to microSD card breakout and SHT15 sensor
#define LED 2  // pin 7 controls LED
int minutos = 1; //minutos en modo sleep
int SDcsPin = 8; // pin 9 is CS pin for MicroSD breakout

PowerSaver chip;    // declare object for PowerSaver class
File myFile;
//DS3234 RTC;    // declare object for DS3234 class
//SdFat sd;     // declare object for SdFat class
//SdFile myFile;    // declare object for SdFile class

// ISR ****************************************************************
//ISR(PCINT0_vect)  // Interrupt Vector Routine to be executed when pin 8 receives an interrupt.
//{
//PORTB ^= (1<<PORTB1);
//asm("nop");
//}


// setup ****************************************************************
void setup()
{
  Serial.begin(9600); // open serial at 19200 bps
  Wire.begin();

  //Inicializo una instancia del reloj DS3231
  DS3231_init(DS3231_INTCN);

  // set output pins
  pinMode(POWA, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(POWA, HIGH);    // turn on SD card
  delay(1);    // give some delay to ensure SD is initialized properly

  //Inicializacion del SD
  inicializaSD();
  delay(1);

  chip.sleepInterruptSetup();    // setup sleep function. Power-down mode in 8 seconds mode
}

// loop ****************************************************************

void loop()
{

  //MODO SLEEP: CONFIGURACION------
  
  digitalWrite(POWA, LOW);  // turn off microSD card to save power
  delay(1);  // give some delay for SD card and RTC to be low before processor sleeps to avoid it being stuck

  chip.turnOffADC();    // turn off ADC to save power
  chip.turnOffSPI();  // turn off SPI bus to save power
  //chip.turnOffWDT();  // turn off WatchDog Timer to save power (does not work for Pro Mini - only works for Uno)
  chip.turnOffBOD();    // turn off Brown-out detection to save power

  //blink LED 1 time to indicate sleep mode config set well
  blink_led(1);
    
  //int cont=(minutos*60)/8;
  //for (int i = 0; i < cont; i++) { 
  chip.goodNight(); 
  //}

  //MODO DESPIERTO: CONFIGURACION,SENSADO Y ALMACENAMIENTO------
  
  // code will resume from here once the processor wakes up =============== //
  chip.turnOnADC();    // enable ADC after processor wakes up
  chip.turnOnSPI();   // turn on SPI bus once the processor wakes up
  delay(1);    // important delay to ensure SPI bus is properly activated

  pinMode(POWA, OUTPUT);
  digitalWrite(POWA, HIGH);  // turn on SD card power
  delay(1);    // give delay to let the SD card and SHT15 get full powa

  //Reinicializar el modulo SD
  inicializaSD();
  delay(10);

  //save data
  myFile = SD.open(filename, FILE_WRITE);
  
  //myFile = sd.open(filename, O_WRITE | O_AT_END);  // open file in write mode
  //Almacenar fecha, hora y datos de los sensores
  almacenar_datos(myFile);
  
  
  //------SENSORES----///
  
  myFile.close();

}

//*********************************************************************************************
// FUNCTIONES

/////----BLINK LED------/////
void blink_led(int n){
pinMode(LED, OUTPUT);

    for (int i = 1; i < n; i++) // blink LED n times to indicate SD card write error
    {
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
}

/////----Inicializacion SD----/////
void inicializaSD() {
  pinMode(SDcsPin, OUTPUT);

  if (!SD.begin(SDcsPin)) {
       blink_led(3);
  }
  else {
    //blink LED 2 time to indicate SD card init propperly
    blink_led(2);
  }
}

/////----Almacenar datos en SD----/////

void almacenar_datos(File myFile) {

  //Tomo el tiempo del reloj
  //-----------------------------------------------------------------------------------------
  //char in;
  //char buff[BUFF_MAX];
  //unsigned long now = millis();
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

    // ** Fecha y Tiempo
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
    // **//
    // ** Sensores: Los datos son pasados por referencia a la función

    // **//
    // Escribo en la SD
    myFile.println(datastring);
  }
  else {
    // if the file didn't open, print an error:
    //LED ERROR!!!
    blink_led(2);
  }
  //-----------------------------------------------------------------------------------------
}
