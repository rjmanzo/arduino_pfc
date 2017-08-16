/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(7,8);                    // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);          // Network uses that radio

const uint16_t this_node = 01;        // Address of our node in Octal format
const uint16_t other_node = 00;       // Address of the other node in Octal format

const unsigned long interval = 1000; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already

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
  Serial.println("RF24Network/examples/helloworld_tx/");
 
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH); // Establesco la potencia de la señal 
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
}

void loop() {
  
  network.update();                          // Check the network regularly

  
  unsigned long now = millis();              // If it's time to send a message, send it!
  if ( now - last_sent >= interval  )
  {
    last_sent = now;
    float volt = voltaje(); //midiendo el voltaje
    
    Serial.print("Sending...");
    payload_t payload = {volt,{2017,7,16,15,42,00},2,6}; //simulo enviar un paquete con la información de la tensión de entrada 
    RF24NetworkHeader header(/*to node*/ other_node);
    bool ok = network.write(header,&payload,sizeof(payload));
    if (ok)
      Serial.println("ok.");
    else
      Serial.println("failed.");
  }
}


