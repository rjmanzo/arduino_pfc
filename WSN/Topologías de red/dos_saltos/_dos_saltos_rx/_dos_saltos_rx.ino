/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/***********************************************************************
************* Set the Node Address *************************************
/***********************************************************************/
// These are the Octal addresses that will be assigned
const uint16_t node_address_set[6] = { 00, 01, 02, 03, 04, 05};
 
// 0 = Master
// 1-5 (01,02,03,04,05)   = Children of Master(00)
//uint8_t NODE_ADDRESS = 2;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 0;  // Use numbers 0 through to select an address from the array
/***********************************************************************/
/***********************************************************************/


RF24 radio(7,8);                // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);      // Network uses that radio
const uint16_t this_node = MASTER_ADDRESS;    // Address of our node in Octal format ( 04,031, etc)
//const uint16_t other_node = 01;   // Address of the other node in Octal format

//Paquete de datos
struct payload_t {                  // Structure of our payload
  float data;  
  int timestamp[6];
  unsigned int nod_red_id;
  unsigned int sen_id;
};

void setup(void)
{
  //Wireless lines UP!
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  //pinout por las dudas
  pinMode(10, OUTPUT);
  //Datalogeer lines DOWN!
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);

  Serial.begin(9600);
  Serial.println("RF24Network/examples/helloworld_rx/");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
}

void loop(void){
  
  network.update();                  // Check the network regularly
  
  while ( network.available() ) {     // Is there anything ready for us?
    
    RF24NetworkHeader header;        // If so, grab it and print it out
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    Serial.print("Received packet from Node ");
    Serial.print(header.from_node);
    Serial.print(" Data: ");
    Serial.print(payload.data);
    Serial.print(" - node_red_id: ");
    Serial.println(payload.nod_red_id);

  }
}

