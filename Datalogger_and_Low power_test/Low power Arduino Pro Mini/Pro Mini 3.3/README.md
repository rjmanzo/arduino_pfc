# Arduino

En este carpeta se encuentran las pruebas del datalogger Shield en estado activo y sleep mode de la placa Arduino Pro Mini de 3.3V.

## Sketch

1.  sleep_mode_posta_Mini3

En este sketch se prueban el modulo DS3231 y Catalex MicroSD en modo activo y sleep. La mayor parte del tiempo el dispositivo duerme pero cuando se despierta toma el timestamp del clock
y escribe este dato en un archivo que guarda en la tarjeta SD. De esta manera, podemos determinar el consumo del datalogger en modo activo y sleep mode, fundamental para determinar cuando es el 
consumo final.

2.  sleep_mode_posta_Mini3_npn

Este sketch es el mismo que el sleep_mode_posta_Mini3 pero incorpora en el sketch el código para activar / desactivar un switch externo. Con este pequeño tweak, logramos cortar la administración de 
energía hacia los modulos y por ende bajar el consumo.

3.  sleep_mode_posta_Mini3_npn_HIGH





