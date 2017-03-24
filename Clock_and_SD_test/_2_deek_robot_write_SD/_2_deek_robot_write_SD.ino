///Include Shield SD RTC
#include <SD.h>
#include <SPI.h>
 
//Define SD RTC
File myFile;
const int chipSelect = 8;
  
////////////--Setup--///////////
void setup() {
 Serial.begin(9600); //Se inicia la comunicación serial
 
 //Inicializacion del SD
 inicializaSD(); 

 myFile = SD.open("test3.txt", FILE_WRITE); 
 //Almacenar fecha y hora para testing
 almacenar_fecha(myFile); 
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
 
/////----Almacenar fecha y hora----/////  
void almacenar_fecha(File myFile){
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
