///Include Shield SD
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
 
//Define global  
File myFile;
const int chipSelect = 8; 
    
////////////--Setup--///////////
void setup() {
  Serial.begin(9600); //Se inicia la comunicación serial 
 
 //Inicializacion del SD
 inicializaSD();
 delay(3000); //Espero 3 sg para mostrar los datos

 // open the file named test3.txt
 myFile = SD.open("test3.txt");

 //leo los datos almacenados en la SD
 leer_datos(myFile);

 myFile.close();
} 
 
////////////--loop--/////////// 
void loop() {

  //NULL
  
   }
 
/////----Inicializacion SD----/////
void inicializaSD(){
  pinMode(10, OUTPUT);
 
  if (!SD.begin(chipSelect)) {
        Serial.println("SD Card - initialization failed!");
        return;
  }
  Serial.println("SD Card - initialization done");
} 
//////////////-----------------------///////////////
/////----Imprimir fecha y hora----///// 
void leer_datos(File myFile){
    
  // if the file is available, read the file
  if (myFile) 
  {
    while (myFile.available())
    {
      Serial.write(myFile.read());
    }    
  }  
  // if the file cannot be opened give error report
  else {
    Serial.println("error opening the text file");
  } 
}
  
 

