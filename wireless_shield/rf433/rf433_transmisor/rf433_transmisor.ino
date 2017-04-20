// Programa Transmisor RF433 de DAFR ELECTRONICS
// Autor: Daniel Arturo Fernandez Raygoza
//Prohibida su distribucion para fines comerciales
//Compra tu material en www.dafr-electronics.mex.tl

#include <VirtualWire.h>

void setup()
{
    Serial.begin(9600);	  // abre el puerto de serie
    Serial.println("Transmisor listo"); // Imprime "listo" en el monitor serial
    pinMode(13, OUTPUT); //configura el pin 13 como salida, se usara el led integrado.
   
    vw_set_tx_pin(TX_PIN);          // Configura el pin TX
    vw_set_ptt_inverted(true);  // Requerido para DR3100
    vw_setup(4000);	        // Bits por segundo
}

void loop()
{
    digitalWrite(13, HIGH);      // Parpadeo para mostrar actividad
    char msg[] = "DAFR Electronics";   // Mensaje a enviar
    vw_send((uint8_t *)msg, strlen(msg)); //Enviando...
    vw_wait_tx(); // espera hasta que el mensaje se haya enviado
    Serial.println("Transmitiendo..."); // Imprime transmitiendo en el monitor serial
    delay(125);                 // Pausa antes repetir el ciclo
    digitalWrite(13, LOW);      // Parpadeo para mostrar actividad
    delay(125);                 // Pausa antes repetir el ciclo
         
} 
