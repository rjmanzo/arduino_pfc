
/*  Based on rRF24Network/examples/meshping from James Coliz, Jr. <maniacbug@ymail.com> 
    and Updated in 2014 - TMRh20
    
    Wireless sensor network (WSN) - coordinator node code

    Coder.
    Manzo Renato José - Ceneha (UNL), Santa Fe, Argentina, 2017
    
*/

/*
 sen_id | sen_descrip | type_sen_id | type_sen_id | type_sen_descrip 
--------+-------------+-------------+-------------+------------------
      1 | DHT22       |           1 |           1 | Temperatura
      2 | DHT22       |           2 |           2 | Humedad
      3 | BMP183      |           4 |           4 | Presion
      4 | BMP183      |           1 |           1 | Temperatura
      5 | Tensión     |           5 |           5 | Batería
     99 | send data   |           - |           - | -
    100 | node_config |           - |           - | -
    101 | go to sleep |           - |           - | -

  (+) 99,100 & 101 doesnt exist. We use as flags
  
  nodo_id | nod_descrip 
---------+-------------
       1 | Nodo_0
       2 | Nodo_1
       3 | Nodo_2
       4 | Nodo_3
       5 | Nodo_4

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
#include "WiFiEsp.h"
// Emulate Serial1 on pins 3/2 if not present
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(3, 2); // RX, TX
//#endif

/*********************************STATIC VARIABLES************************/
#define POWA_W 6 //switch pin of wireless shield
#define POWA_D 9 //switch pin of datalogger shield
#define SDcsPin 10 // pin 10 is CS pin for MicroSD breakout

RF24 radio(7, 8);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio

/***********************************************************************
************* Set the Node Address *************************************
  /***********************************************************************/
// These are the Octal addresses that will be assigned
const uint16_t node_address_set[5] = { 00, 01, 011, 0111, 02};

//tree topology
// 0 = Master
// 1 y 4 (01 - 02)   = Children of Master(00)
// 2 (011) = Children of (01)
// 3 (0111) = Children of (011)

uint8_t NODE_ADDRESS = 0;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 0;  // Use numbers 0 through to select an address from the array

/*********************************SYSTEM VARIABLES ASSIGNATION************************/
const uint16_t this_node = node_address_set[NODE_ADDRESS];        // Address of our node in Octal format
const uint16_t other_node = node_address_set[MASTER_ADDRESS];       // Address of the other node in Octal format

const unsigned long interval = 1000; //ms  // How often to send 'hello world to the other unit

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
  byte node_config[9]; 
};

//char ssid[] = "rena";            // your network SSID (name)
//char pass[] = "123456789";        // your network password
char ssid[] = "rena";            // your network SSID (name)
char pass[] = "123456789";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

/*********************************PROTOTYPES FUNCTIONS************************/
bool send_T(uint16_t to, payload_t payload); // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);

void battery_voltage(float& voltage);  //Prototypes functions of sensor

void activate_wireless();   //other Prototypes functions
void activate_datalogger();
void init_radio();
void init_datalogger();
void sleep_mode (int sleep_minutes);
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec);

void setup(void)
{
  //protocol initialization
  SPI.begin(); //spi
  Wire.begin(); //i2c
  
  pinMode(10, OUTPUT);    // Even we dont use it, we've to setup Pin 10 as output. It's the reference for SPI channel.

  activate_wireless(); //wireless shield switch ON

  activate_datalogger(); //Datalogger shield switch ON
  
  Serial.begin(9600);
  Serial.println("\n\rRF24Network - Coordinator mote \n\r");

  init_radio();   // Init radio and assign the radio to the network  

  init_datalogger(); // set initial parameters for ds3231 & Catalex Micro SD

}

void loop() {

  network.update();                                      // Pump the network regularly

  while ( network.available() )  {                      // Is there anything ready for us?

    RF24NetworkHeader header;                            // If so, take a look at it
    network.peek(header);

    switch (header.type) {                             // Dispatch the message to the correct handler.
      case 'T': handle_T(header); break; //with this function we recive all the package from nodes
      case 'N': handle_N(header); break; //if this 
      default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"), header.type);
        network.read(header, 0, 0);
        break;
    };
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

void sleep_mode (int sleep_minutes) { // nrfl24l01 radio Init

  deactivate_datalogger();
  deactivate_wireless();

  //set the sleep period
  //el tiempo maximo que la libreria permite poner el Arduino en modo sleep es
  //de 8seg, por lo que se hacen varios ciclos para lograr los minutos requeridos
  //(por ej: para 10 minutos se hacen 10*60/8=75 ciclos)   
  int minutos = 1;
  int cont=(minutos*60)/8;
  for (int i = 0; i < cont; i++) { 
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  }

  //protocol initialization
  SPI.begin(); //spi
  Wire.begin(); //i2c

  activate_datalogger();
  activate_wireless();

  init_radio();

  init_datalogger(); // set initial parameters for ds3231 & Catalex Micro SD

  
  }

/**--------------------------------------------------------------------------
   Sensores: zona donde van las funciones para el calculo de las variables
  --------------------------------------------------------------------------*/

/*DS3231_get_time (Fecha y hora)
  The library has the function DS3231_get(&t) to get the timestamp. Its return a struct by reference variable.
  The Stuct ts its a especial type of struct that have this presetting values:
  t.year, t.mon, t.mday, t.hour, t.min, t.sec
*/

//DS3231_update_time (Fecha y hora)
void set_datetime(uint8_t years,uint8_t mon,uint8_t days,uint8_t hours,uint8_t minutes, uint8_t sec ) {

  struct ts t;
  t= {sec,minutes,hours,days,mon,years,0,0,0};
  DS3231_set(t); //set the rtc with the reference datetime

}

//MicroSD (Fecha y hora)

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
  payload_t payload;
  network.read(header,&payload,sizeof(payload));
  Serial.print("Received packet from Node ");
  Serial.print(header.from_node);
  Serial.print(" Data: ");
  Serial.print(payload.data);
  Serial.print(" - timestamp: ");
  Serial.print(String(payload.timestamp[0]));
  Serial.print("-");
  Serial.print(String(payload.timestamp[1]));
  Serial.print("-");
  Serial.print(String(payload.timestamp[2]));
  Serial.print("T");
  Serial.print(String(payload.timestamp[3]));
  Serial.print(":");
  Serial.print(String(payload.timestamp[4]));
  Serial.print(":");
  Serial.print(String(payload.timestamp[5]));
  Serial.print(" - node_red_id: ");
  Serial.print(payload.nod_red_id);
  Serial.print(" sen_id: ");
  Serial.println(payload.sen_id);
  delay(50);

  /*
  //pasos a seguir:
  - Guardar el paquete en la SD
  - 
  */

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
