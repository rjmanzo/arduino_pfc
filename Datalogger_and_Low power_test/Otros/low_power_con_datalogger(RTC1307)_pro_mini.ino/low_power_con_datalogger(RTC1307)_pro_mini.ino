 //****************************************************************

// OSBSS T/RH datalogger code Mod for Ceneha purpose - v0.01
// Last edited on May 24, 2016

//****************************************************************
 
#include <EEPROM.h>
#include <Wire.h>
#include <SD.h>
#include <RTClib.h>
#include <LowPower.h>
#include <SPI.h>
//#include "ds3231.h"
//#include "rtc_ds3231.h"

// Launch Variables   ******************************
//uint8_t time[8];
char filename[15] = "log.csv";    // Set filename Format: "12345678.123". Cannot be more than 8 characters in length, contain spaces or begin with a number

// Global objects and variables   ******************************
#define POWA 4 // pin 4 supplies power to microSD and DS3231 RTC
#define LED 2  // pin 7 controls LED
int minutos = 1; //minutos en modo sleep
int SDcsPin = 8; // pin 9 is CS pin for MicroSD breakout
 
//PowerSaver chip;    // declare object for PowerSaver class
//File instant
File myFile;
//rtc instance
RTC_DS1307 rtc;

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
  //DS3231_init(DS3231_INTCN);
  //Inicializacion de RTC
  inicializaRTC();
 
 blink_led(1);
  
 // set output pins
 pinMode(POWA, OUTPUT);
 pinMode(LED, OUTPUT);

 digitalWrite(POWA, HIGH);    // turn on SD card
 delay(1);    // give some delay to ensure SD is initialized properly

 //Inicializacion del SD
 inicializaSD();
 delay(1);
}

// loop ****************************************************************

void loop()
{

  //MODO SLEEP: CONFIGURACION------

  digitalWrite(POWA, LOW);  // turn off microSD card to save power
  delay(1);  // give some delay for SD card and RTC to be low before processor sleeps to avoid it being stuck

 
  //blink LED 1 time to indicate sleep mode config set well
 x   blink_led(1);

  //el tiempo maximo que la libreria permite poner el Arduino en modo sleep es
  //de 8seg, por lo que se hacen varios ciclos para lograr los minutos requeridos
  //(por ej: para 10 minutos se hacen 10*60/8=75 ciclos) 
  
  
  int cont=(minutos*60)/8;
  for (int i = 0; i < cont; i++) { 
    //idle(period_t period, adc_t adc, timer2_t timer2,timer1_t timer1, timer0_t timer0,spi_t spi, usart0_t usart0,twi_t twi
    //LowPower.powerDown_off(SLEEP_8S,ADC_OFF,BOD_OFF,TIMER2_OFF,TIMER1_OFF,TIMER0_OFF,SPI_OFF,USART0_OFF,TWI_OFF); 
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }
  //MODO DESPIERTO: CONFIGURACION,SENSADO Y ALMACENAMIENTO------

  // code will resume from here once the processor wakes up =============== //
  //LowPower.powerDown_on(ADC_ON,BOD_ON,TIMER2_ON,TIMER1_ON,TIMER0_ON,SPI_ON,USART0_ON,TWI_ON); 
  delay(1);    // important delay to ensure SPI bus is properly activated

  pinMode(POWA, OUTPUT);
  pinMode(LED, OUTPUT);
  
  digitalWrite(POWA, HIGH);  // turn on SD card and DS3231
  delay(1);    // give delay to let the SD card and SHT15 get full powa

  //Reinicializo el RTC
  inicializaRTC();
 
  //Reinicializar el modulo SD
  inicializaSD();
  delay(10);
 

  //------SENSORES----///

  //------ALMACENAMIENTO DE DATOS----///
  myFile = SD.open(filename, FILE_WRITE);

  //myFile = sd.open(filename, O_WRITE | O_AT_END);  // open file in write mode
  //Almacenar fecha, hora y datos de los sensores
  almacenar_datos(myFile);

  myFile.close();

}

 
//*********************************************************************************************
// FUNCTIONES

/////----BLINK LED------/////
void blink_led(int n) {
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
     return;
  }
  else {
    //blink LED 2 time to indicate SD card init propperly
    blink_led(2);
  }
}

/////----Inicializacion RTC----/////
void inicializaRTC(){
 Wire.begin();//Establece la velocidad de datos del bus I2C
 rtc.begin(); //Establece la velocidad de datos del RTC
 
  if (! rtc.isrunning()) { // saber si el reloj externo esta activo o no
    blink_led(2);
    //Serial.println ("RTC is NOT running!");
    //Para sincronizar con el PC HACER ESTO UNA SOLA VEZ
  // RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}
 
////----Almacenar datos en SD----/////
 
void almacenar_datos(File myFile) {

  //Tomo el tiempo del reloj
  //-----------------------------------------------------------------------------------------
  //char in;
  //char buff[BUFF_MAX];
   //unsigned long now = millis();
  //struct ts t;

  //get time
  //En este caso el DS3231
  ///DS3231_get(&t);
  //-----------------------------------------------------------------------------------------
  // Almaceno los datos en la tarjeta SD
  //-----------------------------------------------------------------------------------------
   // si el archivo abrio exitosamente armo el string
  if (myFile) {
    //creando string
    String datastring = " "; //guardamos la lectura en un solo string
    // ** Fecha y Tiempo
    DateTime now = rtc.now();
    datastring +=String(now.year());
    datastring += ";";
    datastring +=String(now.month());
    datastring += ";";
     datastring +=String(now.day());
    datastring += ";";
    datastring +=String(now.hour());
    datastring += ";";
    datastring +=String(now.minute());
    datastring += ";";
    datastring +=String(now.second());
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

