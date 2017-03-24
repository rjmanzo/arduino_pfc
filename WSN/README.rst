En esta sección esta el código referente a la red de sensores Inálambrica (WSN) para Arduino. Resueltos ya el datalogger, la comunicación inalámbrica entre Nodos y la comunicación del coordinador con el
servidor, se procede a generar los casos de prueba.

Se generan 3 diferentes roles: Coordinador, Router y Hoja

- El Coordinador cumple tres funciones:
  * Es un nodo concentrador de la información de toda la red. Los Router le envian la información de sus hojas
  *
- El router cumple tres funciones:
  * Recibe la información de sus nodos hojas.
  * Reenvia la configuración de la red comunicada por el Coordinador.
  * Puede cumplir funciones de nodo Hoja como sensar una variable o bien reenviar la información a otro nodo Router.
- El Hoja cumple dos funciones:
