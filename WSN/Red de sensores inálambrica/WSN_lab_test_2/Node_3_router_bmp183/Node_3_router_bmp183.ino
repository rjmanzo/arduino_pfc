/*  Based on rRF24Network/examples/meshping from James Coliz, Jr. <maniacbug@ymail.com> 
    and Updated in 2014 - TMRh20
    
    Wireless sensor network (WSN) - Router/leaft node

    Coder.
    Manzo Renato José - Ceneha (UNL), Santa Fe, Argentina, 2017
    
*/
/*********************************LIBRARY*********************************/
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LowPower.h> //LowPower - Sleep mode
#include <SD.h> //SD card
#include <ds3231.h> //DS3231

// SENSORS LIBRARY
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP183.h>

//#include "DHT.h"

/*********************************STATIC VARIABLES************************/
#define POWA_W 6 //switch pin of wireless shield
#define POWA_D 9 //switch pin of datalogger shield

#define SDcsPin 10 // pin 10 is CS pin for MicroSD breakout


// We have conflict between the datalogger shield and the BMP183 sensor, so me decide to use a use diferent SPI configuration
#define BMP183_CLK  5 // CLK
#define BMP183_SDO  4  // AKA MISO
#define BMP183_SDI  3  // AKA MOSI
#define BMP183_CS   2 // CS 

//#define DHTPIN 2    // what digital pin we're connected to
//#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321

/*********************************SENSOR OBJECT CREATION******************/
// initialize with hardware SPI
Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);

RF24 radio(7, 8);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio
uint8_t NODE_ID = 3;                // This is the id we have to change for webapp

/***********************************************************************
************* Set the Node Address *************************************
  /***********************************************************************/
// These are the Octal addresses that will be assigned
const uint16_t node_address_set[6] = { 00, 01, 011, 0111, 02};

//tree topology
// 0 = Master
// 1 y 4 (01 - 02)   = Children of Master(00)
// 2 (011) = Children of (01)
// 3 (0111) = Children of (011)

uint8_t NODE_ADDRESS = 3;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 0;  // Use numbers 0 through to select an address from the array

/*********************************SYSTEM VARIABLES ASSIGNATION************************/
const uint16_t this_node = node_address_set[NODE_ADDRESS];        // Address of our node in Octal format
const uint16_t other_node = node_address_set[MASTER_ADDRESS];       // Address of the other node in Octal format

const unsigned long interval = 40000; //ms  // How long will seek for package ?
//const unsigned long config_internal = 5000; //ms  // How long will seek for package ?
const unsigned long config_interval = 0; //ms  // How long will seek for package ?

unsigned long last_time_sent;

const short max_active_nodes = 5;            // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;

// Structure of our payload
struct payload_t {
  float data;
  int timestamp[6];
  unsigned int node_id;
  unsigned int sen_id;
  byte node_config[12]; 
};

/*********************************PROTOTYPES FUNCTIONS************************/
bool send_T(uint16_t to, payload_t payload); // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);

void battery_voltage(float& voltage);  //Prototypes functions of sensor
void bmp183(float& preassure, float& bmp_temp);

void activate_wireless();   //other Prototypes functions
void activate_datalogger();
void init_node();
void init_radio();
void init_sensor();
void init_datalogger();
void sleep_mode (int sleep_minutes);
//void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec);
//void set_alarm(uint8_t wake_HOUR, uint8_t wake_MINUTE, uint8_t wake_SECOND);
void save_data(struct ts t, float data, uint8_t node_id, uint8_t sen_id);
bool send_data(struct ts t, float data, uint8_t node_id, uint8_t sen_id,uint16_t to);

void setup(void)
{

  Serial.begin(9600);  
  //Serial.println("\n\rRF24Network - Simply Router mote \n\r");

  //This function set the init function, port and serial comunications of the node
  init_node();
  
}

void loop() {

  EEPROM.write(6,0);                  //Start sending flag to default
  
  if(EEPROM.read(0)){                 // After first boot
   
    bool ok[3] = {false,false,false}; //we set the send package flag to false. We dont have send the packages yet
    
    unsigned long now = millis(); 
    last_time_sent = now;                     
    while ( now - last_time_sent <= interval){ // any start sending package available ?
                
      now = millis();
      Serial.println(now - last_time_sent);

      network.update();                                      // Pump the network regularly
      
      while ( network.available() ) {                      // Is there anything ready for us?
  
      RF24NetworkHeader header;                            // If so, take a look at it
      network.peek(header);
  
      switch (header.type) {                             // Dispatch the message to the correct handler.
        case 'T': handle_T(header); break;
        case 'N': handle_N(header); break;
        default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"), header.type);
          network.read(header, 0, 0);
          break; };      
      }
      
      uint16_t to = other_node;                                   // Who should we send to? By default, send to base
      if ( num_active_nodes ) {                           // Or if we have active nodes,
        to = active_nodes[next_ping_node_index++];      // Send to the next active node
        if ( next_ping_node_index > num_active_nodes ) { // Have we rolled over?
          next_ping_node_index = 0;                   // Next time start at the beginning
          to = 00;                                    // This time, send to node 00.
        }
      }
  
      if ( this_node > 00 || to == 00 ) {                   // Normal nodes send a 'T' ping
      //get sensor data  -->  send data to coordinator --> save to SDr

      if(EEPROM.read(6) == 1){ //can we send data? 

        struct ts t;
        DS3231_get(&t); //Get time
        
        if(ok[0]==false) {    //sending Voltage measure
      
          float voltage; 
          battery_voltage(voltage); //get voltage

          //save_data(struct ts t, float data, uint8_t node_id, uint8_t sen_id);
          //send_data(struct ts t, float data, uint8_t node_id, uint8_t sen_id,uint16_t to);

          ok[0] = send_data(t,voltage,NODE_ID,5, to); //send data to coordinator
          delay(100);
        
          if(ok[0]){
            save_data(t,voltage,NODE_ID,5); //save data in SD  (datetime ,payload ,node_red_id ,sensor_id)
            delay(50);       
          }
        }

        float pressure, bmp_temp;
        bmp183 (pressure, bmp_temp);

        if(ok[1]==false && EEPROM.read(7)%EEPROM.read(3) == 0){  //sending temp measure
                  
          ok[1] = send_data(t,pressure,NODE_ID,3, to); //send data to coordinator
          delay(100);
          
          if(ok[1]){
            save_data(t,pressure,NODE_ID,3); //save data in SD  (datetime ,payload ,node_red_id ,sensor_id)
            delay(50);
          }            
        }
        if(ok[2]==false && EEPROM.read(7)%EEPROM.read(4) == 0){  //sending humidity measure
        
          ok[2] = send_data(t,bmp_temp,NODE_ID,4, to); //send data to coordinator
          delay(100);  
          
          if(ok[2]){
            save_data(t,bmp_temp,NODE_ID,4); //save data in SD  (datetime ,payload ,node_red_id ,sensor_id)
            delay(50);
          }
        }           
      } else {                                               // Base node sends the current active nodes out
        //ok = send_N(to);
        }
      }
    }

    //loop cicle
    if(EEPROM.read(5) == 15){   //only if we dont be in sinc mode 
      int loop_cicle = EEPROM.read(7) + 1;
      EEPROM.write(7,loop_cicle);
    }
   
    if(96%EEPROM.read(7) ==0){   //if cicle_counter reach 96 the counter start over on 1
      EEPROM.write(7,1);
    }
    
    //sinc procress
    if (EEPROM.read(6) == 0){     // if we dont recive any start sending package the sinc procress start
      int half_time = EEPROM.read(5) / 2;     //The sleep time goes to half
      EEPROM.write(5,half_time);
    } else {
      EEPROM.write(5,15); //we set the sleep_time to default
      //EEPROM.write(5,1); //we set the sleep_time to default
    }

    // node_config and sleep time period
    now = millis(); 
    last_time_sent = now;         
    
    while ( now - last_time_sent <= config_interval){ // any sleeping or node_config package available ?
                
      now = millis();
      Serial.println(now - last_time_sent);

      network.update();                                      // Pump the network regularly
      
      while ( network.available() ) {                      // Is there anything ready for us?
  
      RF24NetworkHeader header;                            // If so, take a look at it
      network.peek(header);
  
      switch (header.type) {                             // Dispatch the message to the correct handler.
        case 'T': handle_T(header); break;
        case 'N': handle_N(header); break;
        default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"), header.type);
          network.read(header, 0, 0);
          break; };      
      }
    }   
  } 
          
  if (DS3231_triggered_a1()) {   //check trigger alarm. First boot incomming!
    // INT has been pulled low
    DS3231_clear_a1f();   // clear a1 alarm flag and let INT go into hi-z
    EEPROM.write(0,1); //set first boot flags UP! (Time to sleep) 
  }
 
  //If the alarm has triggered Pro Mini goes to sleep
  if(EEPROM.read(0)){ //check the setting flags 
    int sleep_minutes = EEPROM.read(5); //get the sleeptime from the flag
    sleep_mode(sleep_minutes);   //go to sleep for a while
  }
}       

/**--------------------------------------------------------------------------
   Sensor & modules: init, activate and deactivate functions for sensors & modules
   --------------------------------------------------------------------------*/

void init_radio () { // nrfl24l01 radio Init

  radio.begin(); //nrfl24
  radio.setPALevel(RF24_PA_MAX); // Establesco la potencia de la señal
  network.begin(/*channel*/ 90, /*node address*/ this_node ); // todos los nodos deben estar en el mismo channel
}

void init_datalogger () { // DS3231 & Catalex MicroSD Init

  SD.begin(SDcsPin);   //SD init on Pin SDcsPin CS (10)
  DS3231_init(DS3231_INTCN);  //clock init

}

void init_sensor () { //bmp183 & dht22 sensor Init

  //if(EEPROM.read(1) > 0 || EEPROM.read(2) > 0 ){ dht.begin(); } //dht22 sensor flag
  if(EEPROM.read(3) > 0 || EEPROM.read(4) > 0 ){ bmp.begin(); } //bmp183 sensor flag
  
}

void activate_datalogger () { //switch ON datalogger shield
  pinMode(POWA_D, OUTPUT);
  digitalWrite(POWA_D, HIGH); //Datalogger lines UP!
}

void deactivate_datalogger () { //switch OFF datalogger shield
  digitalWrite(POWA_D, LOW); //Datalogger lines UP!
}

void activate_wireless () { //switch ON wireless shield
  pinMode(POWA_W, OUTPUT);
  digitalWrite(POWA_W, HIGH); //Wireless lines UP!
}

void deactivate_wireless () { //switch OFF wireless shield
  digitalWrite(POWA_W, LOW); //Wireless lines UP!
}

//This function trigger initilization of all protocols, modules ,sensor, etc. 
void init_node(){

  //protocol initialization
  SPI.begin(); //spi
  Wire.begin(); //i2c
  
  pinMode(10, OUTPUT);    // Even we dont use it, we've to setup Pin 10 as output. It's the reference for SPI channel.

  activate_wireless(); //wireless shield switch ON

  activate_datalogger(); //Datalogger shield switch ON

  init_radio();   // Init radio and assign the radio to the network  

  init_datalogger(); // set initial parameters for ds3231 & Catalex Micro SD

  init_sensor();  // its set by the node_config flags
  
}

/**--------------------------------------------------------------------------
   Sleep mode & Data manipulation: sleep mode function, datetime manipulation, send and save data
   --------------------------------------------------------------------------*/

void sleep_mode (int sleep_minutes) { // 

  //setting sleep time period
  /* the microcontroler only allow max sleeptime of 8 seconds. So, if you
  *  want to sleep for ,i.e 1 minute, we need to make a loop of sleeping time cicle.
  *  In our case we use the 4seconds sleeptime. We get better result in the division.  
  *  c_loop = (sleep_minutes*60)/4;
  */  
  if (sleep_minutes != 0){  // Only if sleep minutes >= 1 min the node goes to sleep

    //deactivate all the modules
    deactivate_datalogger();
    deactivate_wireless();
    
    int c_cloop=(sleep_minutes*60)/4;

    //sleep for c_loop times
    for (int i = 0; i < c_cloop; i++) { 
        LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
    }
     
    init_node();    //after sleep mode we need to reinicializar the node 
         
  } //endif
 } 

/*DS3231_get_time (Fecha y hora)
  The library has the function DS3231_get(&t) to get the timestamp. Its return a struct by reference variable.
  The Stuct ts its a especial type of struct that have this presetting values:
  t.year, t.mon, t.mday, t.hour, t.min, t.sec */

//DS3231_update_time (Fecha y hora)
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec ) {

  struct ts t;
  t= {sec,minutes,hours,days,mon,years,0,0,0};
  DS3231_set(t); //set the rtc with the reference datetime
  
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

//save data into SD card
void save_data(struct ts t, float data, uint8_t node_id, uint8_t sen_id){       
  
  char fname[15];
  String sfname = String(t.year) + String(t.mon) + String(t.mday) + ".CSV"; //AAAAMMDD.CSV
  sfname.toCharArray(fname,15);

  if(!SD.exists(fname)){ //New file ? save and send counter to zero 
    EEPROM.write(8,0); EEPROM.write(9,0);
    EEPROM.write(10,0); EEPROM.write(11,0); 
  }  
   
  File myfile = SD.open(fname, FILE_WRITE); //if the file dont exist a new instance 'll be created
  
  int save_counter = word(EEPROM.read(9), EEPROM.read(8)); // how many package we have saved? (0-653325) 

  if (myfile){   
    myfile.print(String(data)); myfile.print(";");
    myfile.print(String(node_id)); myfile.print(";");
    myfile.print(String(sen_id)); myfile.print(";");    
    myfile.print(String(t.year)); myfile.print(";");
    myfile.print(String(t.mon)); myfile.print(";");
    myfile.print(String(t.mday)); myfile.print(";");
    myfile.print(String(t.hour)); myfile.print(";");
    myfile.print(String(t.min)); myfile.print("; ");
    myfile.println(String(t.sec));
    myfile.close();    //close the file
   
    save_counter++; //we increase the save package counter
    EEPROM.write(8, lowByte(save_counter));
    EEPROM.write(9, highByte(save_counter));
    
  } else {
      /*nothing for now*/
    }
}

//send data to destination
bool send_data(struct ts t, float payload, uint8_t node_id, uint8_t sen_id, uint16_t to){

  bool ok;
  
  payload_t payload_t = {payload, {t.year, t.mon, t.mday, t.hour, t.min, t.sec}, node_id, sen_id, {0,0,0,0,0,0,0,0,0,0,0,0}};  // sending package to coordinator
  return ok = send_T(to, payload_t);  

}

/**--------------------------------------------------------------------------
   Sensor measure funtions zone
  --------------------------------------------------------------------------*/

//Input battery voltage
void battery_voltage(float& voltage) {
  // the loop routine runs over and over again forever
  // read the input on analog pin 0:
  analogReference(INTERNAL);
  int sensorValue = analogRead(A0);
  voltage = 0.0;
  for (int i = 0; i < 10; i++) {
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    voltage += (sensorValue * 4.85) / 1024.0;
    delay(1);
  }
  voltage = voltage / 10.0;   // average sample
}

//DHT22 (Humedad y temperatura)
/*void dht22 (float& humidity, float& dht_temp) {

  //Read humidity
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  dht_temp = dht.readTemperature();

}*/

//BMP183 (Presión atmosferica, altura y temperatura)
void bmp183 (float& pressure, float& bmp_temp) {

  //millibar (hPa)
  pressure = bmp.getPressure() / 100.0;
  //temperature
  bmp_temp = bmp.getTemperature();

}

/**--------------------------------------------------------------------------
   Functions for send and recive menssages from the wsn
   --------------------------------------------------------------------------/
   
/** This method is uses to Send the data package to coordinator */

bool send_T(uint16_t to, payload_t payload)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);
  return network.write(header, &payload, sizeof(payload));
}

/** Send an 'N' message, the active node list */
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);
  return network.write(header, active_nodes, sizeof(active_nodes));
}

/**
   Handle a 'T' message
   Add the node to the list of active nodes
*/
void handle_T(RF24NetworkHeader& header) {

  //unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  payload_t payload;
  network.read(header, &payload, sizeof(payload));
  /*first boot(0) - dht22 (1,2) - bmp183 (3,4) - sleep_minutes(5) - Start sending package(6) - loop cicle counter(7) - save package counter(8,9) - save package counter(10,11) */
  //byte node_config[12] = { 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0}; // Init flags for dht22 motes
  //Emulamos el msj del coordinador: Empeza a transmitir!
   
  if(payload.sen_id == 99){ // start sendind package!
    EEPROM.write(6,1);
  }
  if(payload.sen_id == 100){    //new node_config flag 
       
    for (int i=1;i < 4; i++){  //We only modificate sensor measure time
      EEPROM.write(i, payload.node_config[i]);
      }
  }
  if(payload.sen_id == 101){ // Everythink ok. Go to sleep!
    EEPROM.write(6,0);
  }
  if(payload.sen_id == 102){    //Sinc the whole network again. Setting the node to default settings 
       
    //set the clock with new parameters
    set_datetime(payload.timestamp[0],payload.timestamp[1],payload.timestamp[2],payload.timestamp[3],payload.timestamp[4],payload.timestamp[5]);
    
    //set to default this flags: first boot(0) - sleep_minutes(5) - Start sending package(6) - loop cicle counter(7) 
    EEPROM.write(0,1); EEPROM.write(1,1); //first boot & 15 min
    EEPROM.write(6,0); EEPROM.write(7,0); //both to zero
    
    //Set alarm1111
    DS3231_clear_a1f(); //clear alarm
    int minutes = payload.timestamp[5]; 
    set_alarm(17, 56, 00); //set alarm in 3 minutes  
    
  }

  if ( header.from_node != this_node || header.from_node > 00 )                                // If this message is from ourselves or the base, don't bother adding it to the active nodes.
    add_node(header.from_node);
}

/**
   Handle an 'N' message, the active node list
*/
void handle_N(RF24NetworkHeader& header)
{
  static uint16_t incoming_nodes[max_active_nodes];

  network.read(header, &incoming_nodes, sizeof(incoming_nodes));
  //printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"), millis(), header.from_node);

  int i = 0;
  while ( i < max_active_nodes && incoming_nodes[i] > 00 )
    add_node(incoming_nodes[i++]);
}

/**
   Add a particular node to the current list of active nodes
*/
void add_node(uint16_t node) {

  short i = num_active_nodes;                                    // Do we already know about this node?
  while (i--)  {
    if ( active_nodes[i] == node )
      break;
  }

  if ( i == -1 && num_active_nodes < max_active_nodes ) {        // If not, add it to the table
    active_nodes[num_active_nodes++] = node;
    //printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"), millis(), node);
  }
}
