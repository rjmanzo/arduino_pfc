En esta sección se encuentra el código Arduino referente a las pruebas de la red de sensores Inálambrica (WSN).
Resueltos el datalogger, la comunicación inalámbrica entre los nodos y la comunicación del coordinador con el servidor, se procede a generar
los casos de prueba para los diferentes nodos de la red.

En función de las puebas de topología de red realizadas, se crearon dos códigos referentes a las funciones de los distintos roles: Coordinador, Router y Hoja.

- El Coordinador cumple tres funciones:
  * Es un nodo concentrador de la información de toda la red. Los nodos Router le envian la información de sus nodos.
    Dependiendo de la distancia y el salto un nodo Hoja puede enviarle directamente la información al coordinador.
  * Envia la configuración a todos los nodos Router.
  * Se conecta a internet y envia la información relevada con al servidor web.

- El router cumple tres funciones:
  * Recibe la información de sus nodos hojas y reenvia la misma hacia el nodo Coordinador.
  * Reenvia la configuración de la red comunicada por el Coordinador.
  * Puede cumplir las funciones de un nodo Hoja (Sensar y/o reenviar a otro Nodo Router: + de un salto).

- El Hoja cumple dos funciones:
  * Captar la información de los sensores.
  * Enviar al Router y/o Coordinar la información relevada por el nodo.
  
El nodo router y el hoja tienen practicamente las mismas funcionalizadas pero con la diferencia de que router debe redericcionar la información a travez
de la red, sea bien enviando la información al coordinador o a otro router.

La topología de red que utilizaremos para testear la funcionalidad del sistema es la siguiente:


-- 				00			= Master
-- 			01 		02 		= Children of Master (00)
-- 	  	011 				= Children of (01)
-- 0111						= Children of (011)

## Sketck

1. WSN_simple
Al igual que la prueba 1_a_1 de 'Topología de Red' se utilizan dos nodos, el Maestro y un router/hoja.  

La prueba consiste en un test completo que incluye distintos pasos en funciona del tipo de nodo.

*Router

- Despertar
- Sensar
- Switch on: Datalogger y Wireless shield
- Guardar datos en SD
- Esperar confirmación de envio de datos
	- Enviar datos al Coordinador --------------------------------------------------------------------------------------------------------------
	- Esperar por nuevas configuraciones (timeout) ------------ ESTAS TRES TAREAS SE REALIZAN SIEMPRE Y CUANDO SE TENGA CONFIRMACIÓN DEL MAESTRO
	- Reconfigurar (Si es necesario) -----------------------------------------------------------------------------------------------------------
- Switch off: Datalogger y Wireless shield
- Dormir (15 min)

*Coordinador

- Despertar
- Switch on: Datalogger y Wireless shield
- Chequeo de nodo
- Sincronizacion 
	* busca los que no estan
- Enviar confirmación: "Empezando a transmitir datos..."
- Mantener activo el proceso network hasta no recibir más paquetes
	* Por cada paquete recibido guardar el dato en la SD
- Crear JSON post request 
- Envio de datos al servidor (Post request)
- Pedidos de configuraciones (get request)
- Enviar configuraciones a los nodos 
- Reconfigurar (Si es necesario) 
- Switch off: Datalogger y Wireless shield
- Dormir (15 min)

En ambos nodos el ciclo se repite en periodos de 15 minutos.

2. WSN_pruebas_de_laboratorio
Test completo con la topología de red planteada para el testeo del sistema en su conjunto. La información es alocada en un servidor local del Ceneha.

funcionanes incorporadas
a) Inicialización del sistema. Los nodos entran en sleepmode en una hora determinada y desde este punto empieza el proceso.
b)Se incorpora en la código el proceso de reconexión de los nodos ante la perdida de la sincronización con la red. 

3. WSN_pruebas_de_campo