/*  
    WSN start point
    main coder: Manzo Renato José - Ceneha (UNL), Santa Fe, Argentina, 2017
*/

/* Initial setting flags for node configuration

  node_config[0] - first boot. Its use by the sleep_mode function. When the rtc trigger the alarm this function its active, not before. 
  Sensor flags -------------------
  node_config[1] - dht22 humidity
  node_config[2] - dht22 temp 
  node_config[3] - bmp183 pressure
  node_config[4] - bmp183 temp
  *************-------------------
  node_config[5] - package counter. How many package we had saved in SD. If all the package had been sending this number is 0.   
  node_config[6] - loop cicle counter. This number its important to know witch sensor package has to be sent.   
  node_config[7] - Send package. This flags changes from state 0 to 1. When the node recive the send package change from state 0 to 1.   
  node_config[8] - sleep_minutes. This flag it`s use for setting the sleep time minutes of each node.
  

  Sensor flags: Each byte represent time measure of the sensor sample 
    (0) - Off        (1) - 15 minutes 
    (2) - 30 minutes (3) - 45 minutes 
    (4) - 1 hour     (8) - 2 hours
    (16) - 4 hours    (24) - 6 hrs 
    (48) - 12 hrs     (96) - 24 hrs 
*/

/*  sleep_minutes flag.
    The default sleep time is 15 minutes. If the node dont recive the sending package the sleep time goes to half 
*/

/*********************************LIBRARY*********************************/
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <LowPower.h> //LowPower - Sleep mode
#include <SD.h> //SD card
#include <ds3231.h> //DS3231

/*********************************STATIC VARIABLES************************/

#define POWA_D 9 //switch pin of datalogger shield
#define SDcsPin 10 // pin 10 is CS pin for MicroSD breakout

/*********************************SYSTEM VARIABLES************************/
//Sleep time 
//int sleep_minutes = 1; //period on sleepMode
  
/*first boot(0) - dht22 (1,2) - bmp183 (3,4) - package counter(5) - loop cicle counter(6) - Send package(7) - sleep_minutes(8)  */
byte node_config[9] = { 0, 1, 1, 1, 1, 0, 0, 1, 15}; 
//byte node_config[9] = { 0, 1, 1, 1, 1, 0, 0, 0, 15};

/*********************************PROTOTYPES FUNCTIONS************************/
void activate_datalogger();
void init_datalogger();
void set_datetime();
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec );
void sleep_mode(int sleep_minutes);
void set_alarm(uint8_t wake_HOUR, uint8_t wake_MINUTE, uint8_t wake_SECOND);

void setup(void)
{
  //protocol initialization
  Wire.begin(); //i2c
  SPI.begin(); //Spi
  
  pinMode(10, OUTPUT);    // Even we dont use it, we've to setup Pin 10 as output. It's the reference for SPI channel.
    
  Serial.begin(9600);
  //Serial.println("WSN Init configuration \n\r");
  
  activate_datalogger(); //Datalogger shield switch ON

  init_datalogger(); // set initial parameters for ds3231 & Catalex Micro SD instance

  //Setting datetime  
  
  //set_datetime(); //set PC datetime 
  //set_datetime(2017,10,2,20,18,00); //Manual datetime

  //Setting alarm
  DS3231_clear_a1f(); //clear alarm 
  set_alarm(17, 56, 00);

  //clear the EEPROM memory of residual values
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  
  //setting flags for first use 
  for (int i=0;i < 9; i++){
    EEPROM.write(i, node_config[i]); 
  }
    
}

void loop() {
  /*
  if (EEPROM.read(7) == 0 && EEPROM.read(0)!= 0){ //after first boot and if we dont recive start package
    int half_time = EEPROM.read(8) / 2;
    EEPROM.write(8,half_time);
  } */

  /*
  //sleep_time?
  Serial.println(EEPROM.read(8));
  delay(10);*/

  /*
  struct ts t;
  DS3231_get(&t);

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
  delay(1); */

  //check trigger alarm
  if (DS3231_triggered_a1()) {
    // INT has been pulled low
    DS3231_clear_a1f();   // clear a1 alarm flag and let INT go into hi-z
    EEPROM.write(0,1); //set first boot flags UP! (Time to sleep) 
  }
 
  //only if the alarm has triggered Pro Mini goes to sleep
  if(EEPROM.read(0)){ //check the setting flags 
    //Serial.println("Its time for a nap!... bye :)");
    int sleep_minutes = EEPROM.read(8); //get the time from the flag
    sleep_mode(sleep_minutes);   //go to sleep for a while
    delay(500);
    EEPROM.write(7,0);
   }    
}

/**--------------------------------------------------------------------------
   Sensor & modules: init, activate and deactivate functions for sensors & modules
   --------------------------------------------------------------------------*/

void init_datalogger() { // DS3231 & Catalex MicroSD Init

  SD.begin(SDcsPin);   //SD init on Pin SDcsPin CS (10)
  DS3231_init(DS3231_INTCN);  //clock init

}

void activate_datalogger () { //switch ON datalogger shield
  pinMode(POWA_D, OUTPUT);
  digitalWrite(POWA_D, HIGH); //Datalogger lines UP!
}

void deactivate_datalogger () { //switch OFF datalogger shield
  digitalWrite(POWA_D, LOW); //Datalogger lines UP!
}

void sleep_mode(int sleep_minutes) { // nrfl24l01 radio Init

  deactivate_datalogger(); // ds3231 

  //setting the sleep time period
  //el tiempo maximo que la libreria permite poner el Arduino en modo sleep es
  //de 8seg, por lo que se hacen varios ciclos para lograr los minutos requeridos
  //(por ej: para 10 minutos se hacen 10*60/8=75 ciclos)   
  //int cont=(sleep_minutes*60)/8;
  if (sleep_minutes != 0){ //if  
    int c_cloop=(sleep_minutes*60)/4; //We change the 8second sleep for 4seconds. We get better result in the division   
  
    // if c_loop is zero the Pro Mini dont sleep
    for (int i = 0; i < c_cloop; i++) { 
        LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
    }  
  }   

  //protocol initialization
  SPI.begin(); //spi
  Wire.begin(); //i2c

  activate_datalogger();
  init_datalogger(); // set initial parameters for ds3231 & Catalex Micro SD instance  
  
 }

/**--------------------------------------------------------------------------
   DS3231: zona donde van las funciones del modulo
----------------------------------------------------------------------------*/
  
//DS3231_get_time (Fecha y hora)
//we use the library function

//DS3231_update_time (Fecha y hora)

//this function sync the PC datetime with the rtc 
void set_datetime(){ //setting by computer clock  

  //this function use the Computer DateTime for settings rtc 
  DS3231_set(__DATE__, __TIME__);
  
}

//this function set the datetime with the variables 
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec ) { 
  
  struct ts t;
  t= {sec,minutes,hours,days,mon,years,0,0,0};
  DS3231_set(t); //set the rtc with the reference 111111datetime

}

void set_alarm(uint8_t wake_HOUR, uint8_t wake_MINUTE, uint8_t wake_SECOND){
  
    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm - see datasheet
    // A1M1 (seconds) (0 to enable, 1 to disable)
    // A1M2 (minutes) (0 to enable, 1 to disable)
    // A1M3 (hour)    (0 to enable, 1 to disable) 
    // A1M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[5] = { 0, 0, 0, 1, 1 };

    // set Alarm1
    DS3231_set_a1(wake_SECOND, wake_MINUTE, wake_HOUR, 0, flags);

    // activate Alarm1
    DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
}
