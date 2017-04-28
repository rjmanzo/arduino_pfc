# ESP8266 Basic
En esta sección se encuentran los Sketch Arduino para la placa ESP8266 hechos sin librerías, utilizando nada más que los comandos AT y la librería para la comunicación serial por software.

Cabe aclarar que las pruebas sobre la placa si hicieron utilizando el shield wireless, de modo tal que en todos los sktech se incorpora la linea POWA_W 6 (Pin digital - Activación en HIGH) y si correspondiente linea de activación. 

## Pruebas
1. esp8266_inicial
En este sketch se configura el puerto serial del Arduino  con la entrada / salida (RX /TX)  de la placa.
De esta manera podemos comunicarnos mediante la consola serial con la placa esp8266 y enviarle mediante instrucciones
AT pedidos a la misma.
2. esp8266_basic_wifi_conection.
Establecimiento básico de conexión wifi.
3. esp8266_get_request.
Get request al servidor web del ceneha. Resultados no satisfactorias. Llegamos correctamente a conectar al wifi y a establecer un conexión tcp exitosa con el servidor, no así a obtener una respuesta en la solicitud.
