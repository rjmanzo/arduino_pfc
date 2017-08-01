/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network 
 *
 * TRANSMITTER NODE
 * Every 2 seconds, send a payload to the receiver node.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/***********************************************************************
************* Set the Node Address *************************************
/***********************************************************************/
// These are the Octal addresses that will be assigned
const uint16_t node_address_set[6] = { 00, 01, 011};
 
// 0 = Master
// 1 (01)   = Children of Master(00)
// 2 (011) = Children of (01)
uint8_t NODE_ADDRESS = 1;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 0;  // Use numbers 0 through to select an address from the array
/***********************************************************************/
/***********************************************************************/

RF24 radio(7,8);                    // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);          // Network uses that radio

const uint16_t this_node = node_address_set[NODE_ADDRESS];        // Address of our node in Octal format
const uint16_t other_node = node_address_set[MASTER_ADDRESS];       // Address of the other node in Octal format

const unsigned long interval = 1000; //ms  // How often to send 'hello world to the other unit

unsigned long last_time_sent;

const short max_active_nodes = 3;            // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;

bool send_T(uint16_t to);                      // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);


//Paquete de datos
struct payload_t {                  // Structure of our payload
  float data;  
  int timestamp[6];
  unsigned int nod_red_id;
  unsigned int sen_id;
};

float voltaje (){
 // the loop routine runs over and over again forever
  // read the input on analog pin 0:
  analogReference(INTERNAL);
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = (sensorValue * 4.675) / 1024.0;
  // print out the value you read:
  return voltage;
}

void setup(void)
{
  //Wireless lines UP!
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);

  //pinout por las dudas
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);

  Serial.begin(9600);
  Serial.println("\n\rRF24Network/examples/meshping/\n\r");
 
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH); // Establesco la potencia de la señal 
  network.begin(/*channel*/ 90, /*node address*/ this_node );
}

void loop(){
    
  network.update();                                      // Pump the network regularly

   while ( network.available() )  {                      // Is there anything ready for us?
     
    RF24NetworkHeader header;                            // If so, take a look at it
    network.peek(header);

    
      switch (header.type){                              // Dispatch the message to the correct handler.
        case 'T': handle_T(header); break;
        case 'N': handle_N(header); break;
        default:  printf_P(PSTR("*** WARNING *** Unknown message type %c\n\r"),header.type);
                  network.read(header,0,0);
                  break;
      };
    }

  
  unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
  if ( now - last_time_sent >= interval ){
    last_time_sent = now;

    //NODO DONDE QUEREMOS MANDAR NUESTRO MSJ
    uint16_t to = 00;                                   // Who should we send to? By default, send to base
    
    
    if ( num_active_nodes ){                            // Or if we have active nodes,
        to = active_nodes[next_ping_node_index++];      // Send to the next active node
        if ( next_ping_node_index > num_active_nodes ){ // Have we rolled over?
      next_ping_node_index = 0;                   // Next time start at the beginning
      to = 00;                                    // This time, send to node 00.
        }
    }

    bool ok;

    
    if ( this_node > 00 || to == 00 ){                    // Normal nodes send a 'T' ping
        ok = send_T(to);   
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

/**
 * Send a 'T' message, the current time
 */
bool send_T(uint16_t to){
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);
  
  // The 'T' message that we send is just a ulong, containing the time
  unsigned long message = millis(); /*EN VEZ DE PASARLE SOLAMENTE EL TIEMPO DEBERIAMOS PASARLE DIRECTAMENTE EL STRUCT PAYLOAD CON NUESTROS DATOS*/
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending %lu to 0%o...\n\r"),millis(),message,to);
  return network.write(header,&message,sizeof(unsigned long));
}

/**
 * Send an 'N' message, the active node list
 */
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);
  
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending active nodes to 0%o...\n\r"),millis(),to);
  return network.write(header,active_nodes,sizeof(active_nodes));
}

/**
 * Handle a 'T' message
 * Add the node to the list of active nodes
 */
void handle_T(RF24NetworkHeader& header){

  unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  network.read(header,&message,sizeof(unsigned long));
  printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"),millis(),message,header.from_node);


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
/*
void loop() {
  
  network.update();                          // Check the network regularly

  
  unsigned long now = millis();              // If it's time to send a message, send it!
  if ( now - last_sent >= interval  )
  {
    last_sent = now;
    float volt = voltaje(); //midiendo el voltaje
    
    Serial.print("Sending...");
    payload_t payload = {volt,{2017,7,16,15,42,00},NODE_ADDRESS,6}; //simulo enviar un paquete con la información de la tensión de entrada*/ 
    //RF24NetworkHeader header(/*to node*/ other_node);
    /*bool ok = network.write(header,&payload,sizeof(payload));
    //Serial.println(sizeof(payload)); - Aparentemente el size del payload es de 20 bytes
    if (ok)
      Serial.println("ok.");
    else
      Serial.println("failed.");
  }
}*/


