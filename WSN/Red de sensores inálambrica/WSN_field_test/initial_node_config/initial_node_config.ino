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
  node_config[5] - sleep_minutes. This flag mark the sleeping time of each node. 
  node_config[6] - Start sending package. This flags changes from state 0 to 1 when the node recive this package.
  node_config[7] - loop cicle counter. With this counter we know witch sensor package has to be sent. [max= 96 (24hrs)]  
  node_config[8,9] - save package counter. This flag count how many package had been saved on SD card.
  node_config[10,11] - send package counter. How many package we have sent to coordinator.   

  Sensor flags: Each byte represent time measure of the sensor sample 
    (0) - Off        (1) - 15 minutes 
    (2) - 30 minutes (3) - 45 minutes 
    (4) - 1 hour     (8) - 2 hours
    (16) - 4 hours    (24) - 6 hrs 
    (48) - 12 hrs     (96) - 24 hrs 
*/

/*  sleep_minutes flag.
    The default sleep time is 15 minutes. If the node dont recive the Start sending package the sleep time goes to half 
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
  
/*first boot(0) - dht22 (1,2) - bmp183 (3,4) - sleep_minutes(5) - Start sending package(6) - loop cicle counter(7) - save package counter(8,9) - save package counter(10,11) */
//byte node_config[12] = { 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}; // Init flags - TESTING!
//byte node_config[12] = { 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0}; // Init flags - Lab_test_nro_1 & field_test_nro_1
byte node_config[12] = { 1, 1, 1, 1, 1, 15, 0, 0, 0, 0, 0, 0}; // Init flags - Lab_test_nro_2 & field_test_nro_2


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
  set_datetime(); //automatic - Compalite datetime (PC) 
  //set_datetime(2017,8,30,17,27,00); //Manual datetime

  //Setting alarm
  DS3231_clear_a1f(); //clear alarm 
  //set_alarm(17, 40, 00);

  //clear the EEPROM memory of residual values
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  
  //setting flags for first use 
  for (int i=0;i < 12; i++){
    EEPROM.write(i, node_config[i]); 
  }
    
}

void loop() {

  /*
   * Nothing to do here
   */
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

/**--------------------------------------------------------------------------
   DS3231: zona donde van las funciones del modulo
----------------------------------------------------------------------------*/
  
//DS3231_get_time (Fecha y hora)
//we use the library function

//DS3231_update_time (Fecha y hora)

//this function sync the PC datetime with the rtc 
void set_datetime(){ //setting by computer clock  

  //this function use the compiler DateTime for settings rtc 
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
