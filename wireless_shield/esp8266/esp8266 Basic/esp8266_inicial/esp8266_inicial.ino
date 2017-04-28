#define POWA_W 6 //WIRELESS power line
# define baudios 9600
char caracter; //variable para control de com. entre 

void setup()
  {  
    //wireless shield line up!
    pinMode(POWA_W,OUTPUT);
    digitalWrite(POWA_W,HIGH);  
    //seteo la velocidad de la consola y el puerto serial para comunicarme con la esp8266
    Serial.begin(baudios);
    esp.begin(baudios); 
    
  }

 void loop() {
   if (esp.available())
   {
   caracter = esp.read();
   Serial.print(caracter);
   }
   if (Serial.available())
   {
   caracter = Serial.read();
   esp.print(caracter);
   }
   
 }
