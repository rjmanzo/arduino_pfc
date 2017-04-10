 #include <SoftwareSerial.h>
 #define POWA_W 6//WIRELESS power line
 SoftwareSerial SSoft(3, 2); // RX Pin 3 Receptor | TX Pin 2 Transmisor
 char caracter;
 int baudios=9600;
 //int baudios=9600;
  
 void setup() {

 //wireless shield line up!
 pinMode(POWA_W,OUTPUT);
 digitalWrite(POWA_W,HIGH);

 //seteo la velocidad de la consola y el puerto serial para comunicarme con la esp8266
 Serial.begin(baudios);
 SSoft.begin(baudios);
 
 }
 
 void loop() {
   if (SSoft.available())
   {
   caracter = SSoft.read();
   Serial.print(caracter);
   }
   if (Serial.available())
   {
   caracter = Serial.read();
   SSoft.print(caracter);
   }
 }
