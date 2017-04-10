En esta sección se encuentra el código Arduino referente a los dispositivos la red de sensores Inálambrica (WSN).
Resueltos ya el datalogger, la comunicación inalámbrica entre los nodos y la comunicación del coordinador con el servidor, se procede a generar
los casos de prueba para los diferentes nodos de la red.

Se generan 3 códigos diferente referentes a los roles: Coordinador, Router y Hoja

Por otra parte hay que tener presente que las funciones que cumplen
- El Coordinador cumple tres funciones:
  * Es un nodo concentrador de la información de toda la red. Los nodos Router le envian la información de sus nodos anexados.
    Dependiendo de la distancia y el salto un nodo Hoja puede enviarle directamente la información al coordinador.
  * Envia la configuración a todos los nodos Router.
  * Se conecta a internet y envia la información relevada con al servidor web.

- El router cumple tres funciones:
  * Recibe la información de sus nodos hojas y reenvia la misma hacia el nodo Coordinador.
  * Reenvia la configuración de la red comunicada por el Coordinador.
  * Puede cumplir las funciones de un nodo Hoja (Sensar y/o reenviar a otro Nodo Router: + de un salto).

- El Hoja cumple dos funciones:
  * Captar la información de los sensores anexados.
  * Enviar al Router y/o Coordinar la información relevada por el nodo.
