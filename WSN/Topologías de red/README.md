# Arduino

En esta sección se presentan los sketch Arduino desarrollados para la implementación de la WSN.

Como librerías base utilizamos [RF24](https://github.com/nRF24/RF24) y [RF24Network](https://github.com/nRF24/RF24Network), donde RF24Network implementa un wrapper sobre RF24 pensado para la construcción de redes.

Opcional: Probar el uso de la librería [RF24Mesh](https://github.com/nRF24/RF24Mesh). Implementa un wrapper sobre RF24Network donde el nodo master
funciona como servicio DHCP, brindando así direcciones a los nodos. Por solicitud, en base a un NodeID y la posición geográfica se le asigna una dirección octal en el árbol
de red. Al igual que en RF24Network el address de cada nodo es un número octal.   

## Sketck

1.  helloworld_rx_pro
Adaptación del Ej. de recepción que viene con la librería RF24Network para nuestro sistema.
2.  helloworld_tx_pro
Adaptación del Ej. de transmisión que viene con la librería RF24Network para nuestro sistema.
3. 1_a_1
Topología de red utilizando dos nodos. El receptor informa quien es el nodo que le mando el mensaje
4. muchos_a_1
Topología de red de muchos nodos (max=5) a un unico receptor. El receptor informa de cuales nodos a recibido un mensaje
5. un_salto
Topología de red con tres nodos y un salto. El nodo hoja envia un mensaje al coordinador pasando a travez del router (un salto)
6. dos_saltos
Topología de red con cuatro nodos y dos saltos. El nodo hoja envia un mensaje al coordinador pasando a travez del router (dos salto)
