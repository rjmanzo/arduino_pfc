#include <Wire.h>
#include <SD.h>
#include <SPI.h>

//define global
#define POWA 9 // pin 9 power suplite line
File myFile;
const int chipSelect = 10;

void setup()
{
    Serial.begin(9600);
     
    //POWER LINE CONFIG 
    pinMode(POWA, OUTPUT);
    digitalWrite(POWA, HIGH); //HABILITO LA LINEA POWER ENVIANDO UN HIGH A LA BASE DEL TRANSISTOR
    delay(1);
    
    //Inicializacion del SD
    inicializaSD(); 

    //save data  
    myFile = SD.open("test.txt", FILE_WRITE); 
    //Almacenar valores de testing
    delay(50);
    almacenar_datos(myFile); 
    myFile.close();      

    }

void loop()
{
   //NULL
}
 
/////----Inicializacion SD----/////
void inicializaSD(){
  pinMode(chipSelect, OUTPUT);
 
  if (!SD.begin(chipSelect)) {
        Serial.println("SD Card - initialization failed!");
        return;
  }
  Serial.println("SD Card - initialization done");
} 
 
/////----Almacenar valores de testing----/////  
void almacenar_datos(File myFile){
if (myFile) {
    // if the file opened okay, write to it:
    myFile.print("TESTING 1,2,3....");
    //    myFile.print();
    Serial.println("TESTING 1,2,3....");
    Serial.println("Save OKKK!!");
   }
  else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
  }
}
