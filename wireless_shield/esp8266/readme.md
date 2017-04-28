# ESP8266 Libraries

## Sintesis

En esta sección se desarrollan los scketch para la utilización de la placa esp8266.

Se probaron sin exito las siguientes librerías:

1.
2. [Sparkfun AT Library](https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library)
3.
4.

El caso de la librería de Sparkfun se debe a una falta de conocimiento respecto a como generar correctamente las sentencias GET y POST Request con la sintaxis de la librería. Incluso es una librería facil de utilizar y que viene viene por defecto con los ejemplos de códigos que justamente requerimos para nuestro servicio web. De hecho utilizando dichos ejemplos, pudimos realizar un GET y POST request a sus servicios pero no así a los nuestros.

Finalmente y luego de probar estas alternativas fallidas, logramos encontrar una librería que si funcionara: [Sparkfun AT Library](https://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library)


## Pruebas

1. WiFiEsp
2. esp8266 Basic
3. Sparkfun ESP8266
