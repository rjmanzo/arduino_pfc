/*  Based on rRF24Network/examples/meshping from James Coliz, Jr. <maniacbug@ymail.com>
 *  and Updated in 2014 - TMRh20
 *  This code is for simply Router mote of a wireless sensor network (WSN)
 *  Coder: Manzo Renato José - Ceneha (UNL), Santa Fe, Argentina, 2017
 */

/*********************************LIBRARY*********************************/
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SD.h> //
#include <LowPower.h> //LowPower - Sleep mode
#include "ds3231.h" //DS3231
//#include "rtc_ds3231.h"

// SENSORS LIBRARY 

/*********************************STATIC VARIABLES************************/
#define POWA_W 6 //switch pin of wireless shield
#define POWA_D 9 //switch pin of datalogger shield

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
//uint8_t NODE_ADDRESS = 0;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 00;  // Use numbers 0 through to select an address from the array

/*********************************OBJECT CREATION************************/
/*Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CS); // BMP183
DHT dht(DHTPIN, DHTTYPE); // DHT22 
Adafruit_MCP9808 mcp = Adafruit_MCP9808(); // MCP9808 */

RF24 radio(7,8);                    // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);          // Network uses that radio

/*********************************SYSTEM VARIABLES************************/
const uint16_t this_node = node_address_set[00];        // Address of our node in Octal format
const uint16_t other_node = node_address_set[MASTER_ADDRESS];       // Address of the other node in Octal format

const unsigned long interval = 10; //ms  // How often to send 'hello world to the other unit

unsigned long last_time_sent;

const short max_active_nodes = 5;            // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;

//Paquete de datos
struct payload_t {                  // Structure of our payload
  float data;  
  int timestamp[6];
  unsigned int nod_red_id;
  unsigned int sen_id;
};

//bool send_T(uint16_t to);                      // Prototypes for functions to send & handle messages
bool send_T(uint16_t to,payload_t payload);
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
//void handle_T(RF24NetworkHeader& header, payload_t& payload);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);

void battery_voltage(float& voltage);       //Prototypes functions of sensor sample             


void setup(void)
{
  //Wireless lines UP!
  pinMode(POWA_W, OUTPUT);
  digitalWrite(POWA_W, HIGH);

  pinMode(10, OUTPUT);    // Even we dont use it, we've to setup Pin 10 as output. It's the reference for SPI channel.
  pinMode(POWA_D, OUTPUT);
  digitalWrite(POWA_D, LOW);

  Serial.begin(9600);
  Serial.println("\n\rRF24Network - Simply Coordinator mote/");

  //Object initialization
  SPI.begin();
  radio.begin();

  radio.setPALevel(RF24_PA_MAX); // Establesco la potencia de la señal 
  network.begin(/*channel*/ 90, /*node address*/ this_node ); // todos los nodos deben estar en el mismo channel

  /*
   * DEBEMOS DEFINIR INICIALMENTE EN LA MEMORIA EEPROM CUALES SENSORES ESTARAN ACTIVOS Y POR QUE PERIODO DE TIEMPOS
   */
   
}

void loop(){
    
  network.update();                                      // Pump the network regularly

   while ( network.available() )  {                      // Is there anything ready for us?
     
    RF24NetworkHeader header;                            // If so, take a look at it
    network.peek(header);
    //payload_t payload;   
    
      switch (header.type){                              // Dispatch the message to the correct handler.
        case 'T': handle_T(header); break;
        case 'N': handle_N(header); break;
        default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"),header.type);
                  network.read(header,0,0);
                  break;
      };
    }

  /* TIPS: Primero se sensa
   *  Luego se guarda en SD
   *  Posteriormente se envia el dato (si corresponde)
   *  --> Si corresponde: Se busca en la EEPROM cada cuando se mide la variable. Si el ciclo 
   *  actual (en numero) corresponde al ciclo donde corresponde medir se llama a la funcion, se mide la variable 
   *  y se guarda la info en la sd. 
   *  
   * ****/
  
  unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
  if ( now - last_time_sent >= interval ){
    last_time_sent = now;

    //NODO DONDE QUEREMOS MANDAR NUESTRO MSJ
    uint16_t to = other_node;                                   // Who should we send to? By default, send to base
    
    
    if ( num_active_nodes ){                            // Or if we have active nodes,
        to = active_nodes[next_ping_node_index++];      // Send to the next active node
        if ( next_ping_node_index > num_active_nodes ){ // Have we rolled over?
      next_ping_node_index = 0;                   // Next time start at the beginning
      to = 00;                                    // This time, send to node 00.
        }
    }

    bool ok;
    
    if ( this_node > 00 || to == 00 ){                    // Normal nodes send a 'T' ping
      float voltage;
      //battery_voltage(voltage);
      //Serial.println(voltage);
      //payload_t payload = {voltage,{2017,7,16,15,42,00},NODE_ADDRESS,6}; //simulo enviar un paquete con la información de la tensión de entrada*/ 
      payload_t payload = {5.5,{2017,7,16,15,42,00},MASTER_ADDRESS,6}; //simulo enviar un paquete con la información de la tensión de entrada*/ 
      ok = send_T(to,payload);   
    }else{                                                // Base node sends the current active nodes out
        ok = send_N(to);
    }
    
    if (ok){                                              // Notify us of the result
        printf_P(PSTR("%lu: APP Send ok\n\r"),millis());
    }else{
        printf_P(PSTR("%lu: APP Send failed\n\r"),millis());
        last_time_sent -= 100;                            // Try sending at a different time next time
    }
  }

}

/**--------------------------------------------------------------------------
 * Sensores: zona donde van las funciones para el calculo de las variables
  *--------------------------------------------------------------------------*/
//DS3231 (Fecha y hora)

//MicroSD (Fecha y hora)

//Input battery voltage 
void battery_voltage(float& voltage){
 // the loop routine runs over and over again forever
  // read the input on analog pin 0:
  analogReference(INTERNAL);
  int sensorValue = analogRead(A0);
  voltage = 0.0;
  for (int i = 0; i<10; i++){
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage += (sensorValue * 4.675) / 1024.0;
  delay(1);
  }  
  voltage = voltage / 10.0;   // average of ten sampler
}

/**--------------------------------------------------------------------------
 * Funciones prototipo para el envio y transmision de paquetes 
 * en la red
  *--------------------------------------------------------------------------/

/**
 * Send a 'T' message, the current time
 */
bool send_T(uint16_t to,payload_t payload)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);
  
  // The 'T' message that we send is just a ulong, containing the time
  //unsigned long message = millis();
  //CUANDO MANDAMOS LOS DATOS AL NODO 
  //payload_t payload = {voltaje(),{2017,7,16,15,42,00},NODE_ADDRESS,6}; //simulo enviar un paquete con la información de la tensión de entrada*/ 
  //printf_P(PSTR("---------------------------------\n\r"));
  //printf_P(PSTR("%lu: APP Sending %lu to 0%o...\n\r"),millis(),message,to);
  //return network.write(header,&message,sizeof(unsigned long));
  return network.write(header,&payload,sizeof(payload));
}

/**
 * Send an 'N' message, the active node list
 */
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);
  //printf_P(PSTR("---------------------------------\n\r"));
  //printf_P(PSTR("%lu: APP Sending active nodes to 0%o...\n\r"),millis(),to);
  //return network.write(header,active_nodes,sizeof(active_nodes));
  return network.write(header,active_nodes,sizeof(active_nodes));
}

/**
 * Handle a 'T' message
 * Add the node to the list of active nodes
 */
void handle_T(RF24NetworkHeader& header){

  //ACA SE SUPONE QUE DEBERIAMOS LEER EL STRUCT CON NUESTROS DATOS?
  //unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  //network.read(header,&message,sizeof(unsigned long));
  //printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"),millis(),message,header.from_node);
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

  if ( header.from_node != this_node || header.from_node > 00 )                                // If this message is from ourselves or the base, don't bother adding it to the active nodes.
    add_node(header.from_node);
}

/**
 * Handle an 'N' message, the active node list
 */
void handle_N(RF24NetworkHeader& header)
{
  static uint16_t incoming_nodes[max_active_nodes];

  network.read(header,&incoming_nodes,sizeof(incoming_nodes));
  printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"),millis(),header.from_node);

  int i = 0;
  while ( i < max_active_nodes && incoming_nodes[i] > 00 )
    add_node(incoming_nodes[i++]);
}

/**
 * Add a particular node to the current list of active nodes
 */
void add_node(uint16_t node){
  
  short i = num_active_nodes;                                    // Do we already know about this node?
  while (i--)  {
    if ( active_nodes[i] == node )
        break;
  }
  
  if ( i == -1 && num_active_nodes < max_active_nodes ){         // If not, add it to the table
      active_nodes[num_active_nodes++] = node; 
      printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"),millis(),node);
  }
}
