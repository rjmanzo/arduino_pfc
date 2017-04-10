# Sparkfun WiFi Shield

## Sintesis

En este caso apostamos por el uso de la librería de Sparkfun para probar con nuestro shield. Es facil de utilizar y ya viene por defecto con los ejemplos que necesitamos. La librería podemos descargarla desde el repo oficial: [Sparkfun AT Library](https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library)

Para poder utilizarla modificamos dos lineas del header principal "SparkFunESP8266WiFi.h".
    /////////////////////
    // Pin Definitions //
    /////////////////////
    #define ESP8266_SW_RX	2	// ESP8266 UART0 RXI goes to Arduino pin 9
    #define ESP8266_SW_TX	3	// ESP8266 UART0 TXO goes to Arduino pin 8

Cambiamos el pin 9 y 8 por el 2 y 3, respectivamente. De este modo el pin Rx y Tx de la conexión serial por software queda definido para el uso de nuestro shield.

Otros datos interesantes.
* Si utilizas el Shield de Sparkfun su Firmware oficial se puede encontrar en su repositorio oficial: [Sparkfun ESP8266_WiFi_Shield](https://github.com/sparkfun/ESP8266_WiFi_Shield)

* Fueron inteligentes en Sparkfun. Sabiendo el alto consumo que tiene el chip esp8266, desarrollaron una placa con la posibilidad de alimentar via usb o bien mediante un conector para LiPO, de modo tal que podemos utilizar una batería recargable. Más datos en el siguiente link:
[ESP8266 sparkfun Thing](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide)

## Pruebas

1.
