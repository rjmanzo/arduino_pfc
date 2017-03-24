///Include Libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>

//LCD settings
#define  LED_OFF  0
#define  LED_ON  1

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif

// Hardcoded the LCD address to 0x3f for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3f, 16, 2);
//Variable para refresco de pantalla lcd
int lcd_loop = 1;

//DC current measure
Adafruit_INA219 ina219;
//Voltaje y Corriente
float busvoltage;
float current_mA;

//SD y RTC del deek-robot shield
File myFile;
RTC_DS1307 rtc;
const int chipSelect = 8; // pinmode CS para SD
char filename[15] = "datalogUNO.csv";    // Set filename 

void setup(){
//setup settings  
#ifndef ESP8266
  while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
#endif
  uint32_t currentFrequency;

  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  ina219.begin();
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  //Inicializacion de RTC
  inicializaRTC();
 
  //Inicializacion del SD
  inicializaSD();
  
  //initialize the LCD
  lcd.begin();
  // Activamos la retroiluminacion
  lcd.setBacklight(LED_ON);
  //lcd.setCursor(0, 0);
  //lcd.print("DC Volt/Current");
  //lcd.setCursor(0, 1);
  //lcd.print("Adafruit INA219");
  //delay(2000);   
}

void loop(){
  //Voltaje y Corriente
  busvoltage = 0;
  current_mA = 0;
  
  ///-----------------LECTURA DE VOLTAJE / CORRIENTE -------------------
  //tomo los valores de voltaje y corriente del INA219
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  ///-------------------------------------------------------------------

  ///------------------ESCRITURA EN SD----------------------------------
  //Abro el archivo para guardar los datos calculados
  myFile = SD.open(filename, FILE_WRITE);
 
  //Almacenar los datos con fecha y hora
  almacenar_datos(myFile);

  //cierra el archivo
   myFile.close();
  ///------------------------------------------------------------------

  ///-----------------------IMPRIMO DATO SOBRE LCD---------------------
  //escribo cada dos ciclos de 500ms. el lcd no responde a refrescos tan altos.
  if (lcd_loop) {
  //Escribiendo en el display
   lcd.clear();
   //Turn on the blacklight and print a message.
   lcd.backlight(); //Backlight ON
   //lcd.setCursor(0, 0);
   //voltaje
   //lcd.print("Volt: ");
   //lcd.print(busvoltage);
   //lcd.print(" V");
   lcd.setCursor(0, 0);
   //corriente
   lcd.print("Amps.: ");
   lcd.print(current_mA);
   lcd.print(" mA");
   lcd_loop = 0;
  }
  else {
   lcd_loop = 1;
  }
 //espero para la proxima lectura
  delay(500);
}

/////----Inicializacion RTC----/////
void inicializaRTC(){
 Wire.begin();//Establece la velocidad de datos del bus I2C
 rtc.begin(); //Establece la velocidad de datos del RTC
 
 if(! rtc.isrunning()) { // saber si el reloj externo esta activo o no
    Serial.println("RTC is NOT running!");
    //Para sincronizar con el PC HACER ESTO UNA SOLA VEZ
    // RTC.adjust(DateTime(__DATE__, __TIME__));
  }
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
 
/////----Almacenar fecha y hora----/////  
void almacenar_datos(File myFile){

if (myFile) {
    // si el archivo abrio exitosamente armo el string 
    //lectura de clk
    DateTime now = rtc.now();
    //creando string
    String datastring = " "; //guardamos la lectura en un solo string
    datastring += String(now.year());
    datastring += ";";
    datastring += String(now.month());
    datastring += ";";
    datastring += String(now.day());
    datastring += ";";
    datastring += String(now.hour());
    datastring += ";";
    datastring += String(now.minute());
    datastring += ";";
    datastring += String(now.second());
    datastring += ";";
    datastring += String(lcd_loop); //tengo dos muestras por segundo. Uso la bandera del loop para diferenciar
    datastring += ";";
    //timestamp
    
    myFile.print(datastring); //timestamp
    //values
    myFile.print(busvoltage);
    myFile.print(";");
    datastring += (print(current_mA));
    datastring +=";";
    //push data to sd
    myFile.print(datastring);
   }
   
  else {
        // if the file didn't open, print an error:
        Serial.println("error opening file");
  }
}

