-- HARDWARE --
En este apartado, se incorporo al nodo la posibilidad de leer la tensión (voltaje) de entrada del pin (RAW). Esto se logra mediante el uso de un puente resistivo
(resistor diverder) entre el pin analogico A0 y el pin de Entrada RAW (Ver esquema).

--SOFTWARE --
Se llevaron a cabo una serie de test para corroborar la varicación de tensión en carga producto del consumo de los diferentes shield.
En resumidas cuentas, la relación de tensión presentado en las distintas configuraciones es la siguiente:

Sin shield < Shield Datalogger < Shield Wireless (NRF2401) < Shield Wireless (ESP8266) < Shield Wireless + Datalogger

Donde "Sin shield" hace referencia al dispositivo alimentado sin ningún dispositivo externo conectado al Arduino

Debe aclararse que como prueba importante se verifico que el portapilas pese a tener una tensión de sobra para alimentar el ESP8266 no dispone del
output de corriente suficiente para alimentar todo el sistema. De hecho, la placa inalámbrica no logra encenderse correctamente, la tensión
de entrada cae por debajo del 1V y se reinicia constantemente la ESP8266. 

Aclaración: La tensión entregada por el portapilas cuando esta alimenta al sistema se conoce como tensión de carga. Es importante esta distinción,
ya que este es el valor real en trabajo y no así la medida con el voltimetro sobre el portapilas sin ningún dispositivo conectado.
