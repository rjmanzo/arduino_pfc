#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>

//DC current measure
Adafruit_INA219 ina219;

// Hardcoded the LCD address to 0x3f for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3f, 16, 2);

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

#define  LED_OFF  0
#define  LED_ON  1

//Variable para refresco de pantalla lcd
int lcd_loop=1;

void setup()
{
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

  //initialize the LCD
  lcd.begin();
  // Activamos la retroiluminacion
  lcd.setBacklight(LED_ON);
  lcd.setCursor(0,0);
  lcd.print("Measuring Volt");
  lcd.setCursor(0,1);
  lcd.print("&Current INA219");
  delay(2000);  
}

void loop()
{

  //float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  //float loadvoltage = 0;

  //shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  //loadvoltage = busvoltage + (shuntvoltage / 1000);
    
  //guardo las lecturas en la SD


  ///
  //escribo cada dos ciclos de 500ms. el lcd no responde a refrescos tan altos.
  if(lcd_loop){
    //Escribiendo en el display
    lcd.clear();
    //Turn on the blacklight and print a message.  
    lcd.backlight(); //Backlight ON 
    lcd.setCursor(0,0);
    //voltaje
    lcd.print("Volt: ");
    lcd.print(busvoltage);
    lcd.print(" V");
    lcd.setCursor(0,1);
    //corriente
    lcd.print("Amps.: "); 
    lcd.print(current_mA);
    lcd.print(" mA");
    lcd_loop=0;
  }
  else{
   lcd_loop=1;
  }

  //espero para la proxima lectura
  delay(500);
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
    datastring += String(busvoltage);
    datastring += ";";
    datastring += String(current_mA);
    datastring +=";";
    //push data to sd
    myFile.println(datastring);
   }
  else {
        // if the file didn't open, print an error:
        Serial.println("error opening file");
  }
}



