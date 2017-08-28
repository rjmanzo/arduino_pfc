
/*  Based on rRF24Network/examples/meshping from James Coliz, Jr. <maniacbug@ymail.com> 
    and Updated in 2014 - TMRh20
    
    Wireless sensor network (WSN) - Router mote code

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
#include <Adafruit_BMP183.h> //

#include "DHT.h"

/*********************************STATIC VARIABLES************************/
#define POWA_W 6 //switch pin of wireless shield
#define POWA_D 9 //switch pin of datalogger shield

#define SDcsPin 10 // pin 10 is CS pin for MicroSD breakout

// We have conflict between the datalogger shield and the BMP183 sensor, so me decide to use a use diferent SPI configuration
#define BMP183_CLK  5 // CLK
#define BMP183_SDO  4  // AKA MISO
#define BMP183_SDI  3  // AKA MOSI
#define BMP183_CS   2 // CS

#define DHTPIN 2    // what digital pin we're connected to
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321

/*********************************SENSOR OBJECT CREATION******************/
// initialize with hardware SPI
Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);
DHT dht(DHTPIN, DHTTYPE); // DHT22

RF24 radio(7, 8);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio

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
uint8_t NODE_ADDRESS = 1;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 0;  // Use numbers 0 through to select an address from the array

/*********************************SYSTEM VARIABLES ASSIGNATION************************/
const uint16_t this_node = node_address_set[NODE_ADDRESS];        // Address of our node in Octal format
const uint16_t other_node = node_address_set[MASTER_ADDRESS];       // Address of the other node in Octal format

const unsigned long interval = 500; //ms  // How often to send 'hello world to the other unit

unsigned long last_time_sent;

const short max_active_nodes = 5;            // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;

// Structure of our payload
struct payload_t {
  float data;
  int timestamp[6];
  unsigned int nod_red_id;
  unsigned int sen_id;
  byte node_config[12]; 
};

//Activate or not sensor lines 
//int sensorline = 1; // bmp sensor line
int sensorline = 2; // dht22 sensor line
//int sensorline = 3; // bmp183 & dht22 sensor line

//Sleep time in minutes
int sleep_minutes = 1;

/*********************************PROTOTYPES FUNCTIONS************************/
bool send_T(uint16_t to, payload_t payload); // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);

void battery_voltage(float& voltage);  //Prototypes functions of sensor
void bmp183(float& preassure, float& bmp_temp);
void dht22(float& humidity, float& dht_temp);

void activate_wireless();   //other Prototypes functions
void activate_datalogger();
void init_radio();
void init_sensor();
void init_datalogger();
void sleep_mode (int sleep_minutes);
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec);
void save_data(struct ts t, float payload, uint8_t node_id, uint8_t sensor_id);

void setup(void)
{
  //protocol initialization
  SPI.begin(); //spi
  Wire.begin(); //i2c
  
  pinMode(10, OUTPUT);    // Even we dont use it, we've to setup Pin 10 as output. It's the reference for SPI channel.

  activate_wireless(); //wireless shield switch ON

  activate_datalogger(); //Datalogger shield switch ON
  
  Serial.begin(9600);
  Serial.println("\n\rRF24Network - Simply Router mote \n\r");

  init_radio();   // Init radio and assign the radio to the network  

  init_datalogger(); // set initial parameters for ds3231 & Catalex Micro SD

  init_sensor();  // its set by the node_config flags

}

void loop() {

  if(EEPROM.read(0) == 0){

    //wait x second for network boot

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

    /* TIPS: Primero se sensa
        Luego se guarda en SD
        Posteriormente se envia el dato (si corresponde)
        --> Si corresponde: Se busca en la EEPROM cada cuando se mide la variable. Si el ciclo
        actual (en numero) corresponde al ciclo donde corresponde medir se llama a la funcion, se mide la variable
        y se guarda la info en la sd.
     * ****/

    unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
    if ( now - last_time_sent >= interval ) {
      last_time_sent = now;
  
      //NODO DONDE QUEREMOS MANDAR NUESTRO MSJ
      uint16_t to = other_node;                                   // Who should we send to? By default, send to base
  
      if ( num_active_nodes ) {                           // Or if we have active nodes,
        to = active_nodes[next_ping_node_index++];      // Send to the next active node
        if ( next_ping_node_index > num_active_nodes ) { // Have we rolled over?
          next_ping_node_index = 0;                   // Next time start at the beginning
          to = 00;                                    // This time, send to node 00.
        }
      }
  
      bool ok;
  
      if ( this_node > 00 || to == 00 ) {                   // Normal nodes send a 'T' ping

        //get sensor data
        //save to SD
        //send the 
        
        struct ts t;
        DS3231_get(&t); //Get time
              
        //ACA EL METODO SE TIENE QUE DAR CUENTA QUE SENSORES ESTAN ACTIVOS, POR AHORA QUEDA ASI
        float voltage;
        battery_voltage(voltage);
        payload_t payload_volt = {voltage, {t.year, t.mon, t.mday, t.hour, t.min, t.sec}, NODE_ADDRESS, 7, {0,0,0,0,0,0,0,0,0,0,0,0}};  // sending voltage measure sample
        ok = send_T(to, payload_volt);

        //datetime,payload,node_id,sensor_id
        save_data(t,voltage,2,7); //save data in SD 
        
        delay(50);
  
        //dht22 sensor
        if(EEPROM.read(1) > 0 || EEPROM.read(2) > 0 ){
          float humidity, dht_temp;
          dht22 (humidity,dht_temp);
  
          if(1%EEPROM.read(1) == 0){ 
            payload_t payload_hum = {humidity, {t.year, t.mon, t.mday, t.hour, t.min, t.sec}, NODE_ADDRESS, 2 , {0,0,0,0,0,0,0,0,0,0,0,0}};
            ok = send_T(to, payload_hum);
            delay (50);
          }
          if(1%EEPROM.read(2) == 0){ 
            payload_t payload_htemp = {dht_temp, {t.year, t.mon, t.mday, t.hour, t.min, t.sec}, NODE_ADDRESS, 1, {0,0,0,0,0,0,0,0,0,0,0,0}}; 
            ok = send_T(to, payload_htemp);
            delay(50);
          }      
        }     
        //bmp183 sensor
        if(EEPROM.read(3) > 0 || EEPROM.read(4) > 0 ){
          float pressure, bmp_temp;
          bmp183 (pressure, bmp_temp);
  
          if(1%EEPROM.read(3) == 0){ // sending preasure sampler
            payload_t payload_pres = {pressure, {t.year, t.mon, t.mday, t.hour, t.min, t.sec}, NODE_ADDRESS, 6, {0,0,0,0,0,0,0,0,0,0,0,0}}; 
            ok = send_T(to, payload_pres);
            delay(50);
          }
          if(1%EEPROM.read(4) == 0){ // sending preasure sampler
            payload_t payload_ptemp = {bmp_temp, {t.year, t.mon, t.mday, t.hour, t.min, t.sec}, NODE_ADDRESS, 6, {0,0,0,0,0,0,0,0,0,0,0,0}}; 
            ok = send_T(to, payload_ptemp);
            delay(50);
          }     
        }          
  
      } else {                                               // Base node sends the current active nodes out
        ok = send_N(to);
      }
  
      //delay(500);
  
      //Time to sleep!
      //sleep_mode(sleep_minutes);    
  
      //VER DE LLEVAR ESTA FUNCIONALIDAD ARRIBA, AL ENVIO DE CADA PAQUETE
      /*
      if (ok) {                                             // Notify us of the result
        printf_P(PSTR("%lu: APP Send ok\n\r"), millis());
      } else {
        printf_P(PSTR("%lu: APP Send failed\n\r"), millis());
        last_time_sent -= 100;                            // Try sending at a different time next time
      }*/   
    }
  }

  //check trigger alarm for first boot 
  if (DS3231_triggered_a1()) {
    // INT has been pulled low
    DS3231_clear_a1f();   // clear a1 alarm flag and let INT go into hi-z
    EEPROM.write(0,1); //set first boot flags UP! (Time to sleep) 
  }
 
  //If the alarm has triggered Pro Mini goes to sleep
  if(EEPROM.read(0)){ //check the setting flags 
    int sleep_minutes = EEPROM.read(8); //get the sleeptime from the flag
    sleep_mode(sleep_minutes);   //go to sleep for a while
    delay(50);
    //EEPROM.write(7,0);
  } 

}

/**--------------------------------------------------------------------------
   Sensor & modules: init, activate and deactivate functions for sensors & modules
   --------------------------------------------------------------------------*/

void init_radio () { // nrfl24l01 radio Init

  radio.begin(); //nrfl24
  //radio.setPALevel(RF24_PA_HIGH); // Establesco la potencia de la señal
  radio.setPALevel(RF24_PA_MAX); // Establesco la potencia de la señal
  network.begin(/*channel*/ 90, /*node address*/ this_node ); // todos los nodos deben estar en el mismo channel
}

void init_datalogger () { // DS3231 & Catalex MicroSD Init

  SD.begin(SDcsPin);   //SD init on Pin SDcsPin CS (10)
  DS3231_init(DS3231_INTCN);  //clock init

}

void init_sensor () { //bmp183 & dht22 sensor Init

  if(EEPROM.read(1) > 0 || EEPROM.read(2) > 0 ){ dht.begin(); } //dht22 sensor flag
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

void sleep_mode (int sleep_minutes) { // 

  deactivate_datalogger();
  deactivate_wireless();

  //setting sleep time period
  /* the microcontroler only allow max sleeptime of 8 seconds. So, if you
  *  want to sleep for ,i.e a minute, we need to make a loop of sleeping time cicle.
  *  In our case we use the 4seconds sleeptime. We get better result in the division.  
  *  c_loop = sleep_minutes*60)/4;
  */  
  if (sleep_minutes != 0){  // if c_loop is zero the Pro Mini dont sleep
    int c_cloop=(sleep_minutes*60)/4;
    
    for (int i = 0; i < c_cloop; i++) { 
        LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); 
    }  
  }
  
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

//save data into SD card
void save_data(struct ts t, float payload, uint8_t node_id, uint8_t sensor_id){
  
  char fname[13];
  String sfname = String(t.year) + String(t.mon) + String(t.mday) + ".CSV"; //AAAAMMDD.CSV
  sfname.toCharArray(fname,15);
  File myfile = SD.open(fname, FILE_WRITE); //if the file dont exist a new instance 'll be created

  int save_counter = word(EEPROM.read(9), EEPROM.read(8)); // how many package we have saved? (0-653325) 
  
  if (myfile){
    String datastring = " ";
    datastring += String(t.year) + "; " + String(t.mon) + "; " + String(t.mday); + "; " + String(t.hour) + String(t.min); + "; " + String(t.sec) + "; ";
    datastring += String(payload) + "; " + String(node_id) + "; " + String(sensor_id); 
    myfile.println(datastring);    //save the datastring
    myfile.close();    //close the file
   
    save_counter++; //we increase the save package counter
    EEPROM.write(8, lowByte(save_counter));
    EEPROM.write(9, highByte(save_counter));
    
  } else {
      /*nothing for now*/
    }
}

/**--------------------------------------------------------------------------
   Sensores: zona donde van las funciones para el calculo de las variables
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
    voltage += (sensorValue * 4.675) / 1024.0;
    delay(1);
  }
  voltage = voltage / 10.0;   // average sample
}

//DHT22 (Humedad y temperatura)
void dht22 (float& humidity, float& dht_temp) {

  //Read humidity
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  dht_temp = dht.readTemperature();

}

//BMP183 (Presión atmosferica, altura y temperatura)
void bmp183 (float& pressure, float& bmp_temp) {

  //millibar (hPa)
  pressure = bmp.getPressure() / 100.0;
  //temperature
  bmp_temp = bmp.getTemperature();

}

/**--------------------------------------------------------------------------
   Funciones para el envio y transmision de paquetes
   en la red
   --------------------------------------------------------------------------/
  /**
   Send a 'T' message, the current time
*/

bool send_T(uint16_t to, payload_t payload)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);

  // The 'T' message that we send is just a ulong, containing the time
  //unsigned long message = millis();
  //CUANDO MANDAMOS LOS DATOS AL NODO
  //payload_t payload = {voltaje(),{2017,7,16,15,42,00},NODE_ADDRESS,6}; //simulo enviar un paquete con la información de la tensión de entrada*/
  //printf_P(PSTR("---------------------------------\n\r"));
  //printf_P(PSTR("%lu: APP Sending %lu to 0%o...\n\r"),millis(),message,to);
  //return network.write(header,&message,sizeof(unsigned long));
  return network.write(header, &payload, sizeof(payload));
}

/**
   Send an 'N' message, the active node list
*/
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);
  //printf_P(PSTR("---------------------------------\n\r"));
  //printf_P(PSTR("%lu: APP Sending active nodes to 0%o...\n\r"),millis(),to);
  //return network.write(header,active_nodes,sizeof(active_nodes));
  return network.write(header, active_nodes, sizeof(active_nodes));
}

/**
   Handle a 'T' message
   Add the node to the list of active nodes
*/
void handle_T(RF24NetworkHeader& header) {

  //ACA SE SUPONE QUE DEBERIAMOS LEER EL STRUCT CON NUESTROS DATOS?
  //unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  //network.read(header,&message,sizeof(unsigned long));
  //printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"),millis(),message,header.from_node);
  payload_t payload;
  network.read(header, &payload, sizeof(payload));
  //dependiendo del tipo de dato recibido es la accion a realizar

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
  printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"), millis(), header.from_node);

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
    printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"), millis(), node);
  }
}
