// Programa Receptor RF433 de DAFR ELECTRONICS
// Autor: Daniel Arturo Fernandez Raygoza
//Prohibida su distribucion para fines comerciales
//Compra tu material en www.dafr-electronics.mex.tl

#include <VirtualWire.h>
#define POWA_W 6
#define RX_PIN 2
#define TX_PIN 3


int count;

void setup()
{
    //Power lines Up!
    pinMode(POWA_W,OUTPUT);
    digitalWrite(POWA_W,HIGH);
    //Establesco la comunicacion serial
    Serial.begin(9600);	// abre el puerto de serie
    Serial.println("Receptor listo"); //Imprime "listo" en el monitor serial
 
    pinMode(13, OUTPUT); //configura el pin 13 como salida, se usara el led integrado.
    
    vw_set_rx_pin(RX_PIN);       //Configura el pin RX
    vw_set_ptt_inverted(true); //Requerido para DR3100
    vw_setup(4000);	     //  Bits por segundo
    vw_rx_start();       // Inciar el receptor
}

void loop()
{
    digitalWrite(13, HIGH);      // Parpadeo para mostrar actividad
    uint8_t buf[VW_MAX_MESSAGE_LEN]; //Almacenar en buffer los datos recibidos
    uint8_t buflen = VW_MAX_MESSAGE_LEN; //Determina el numero de caracteres recibidos
    if (vw_get_message(buf, &buflen)) // Si hay un mensaje recibido ejecuta...
    {
	int i; 
       	Serial.print("Rx: "); //imprime Rx en el monitor serial
	
	for (i = 0; i < buflen; i++)  //rutina para impresion del mensaje recibido
	{
            char c = (buf[i]); // guarda el caracter recibido para despues imprimirlo
            Serial.print(c); //imprime el caracter recibido en el monitor serial
	    Serial.print(" "); //deja un espacio en el monitor serial
	}
        count++;  //incrementa contador
        Serial.print(count);  //imprime el contador en el monitor serial
	Serial.println("");   // salto de linea monitor serial
    }
    
delay(125);                 // Pausa antes repetir el ciclo
digitalWrite(13, LOW);      // Parpadeo para mostrar actividad
delay(125);                 // Pausa antes repetir el ciclo
}
