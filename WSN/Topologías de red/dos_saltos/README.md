## Tree test topology-------
## 00			= Master
##   01 		= Children of Master (00)
##     011 		= Children of (01)
##        0111	= Children of (011)

## Sketck

1. _dos_saltos_rx
Código del nodo Master / Coordinador (00). Toma los mensajes, parsea el struct y muestra la información por el puerto serial. 
2. _dos_saltos_router_01
Código del nodo router 01. Toma los mensajes que viene de sus hijos (Nodos: 011) y los reenvia hacia el nodo master. Tambié reenvia sus propios mensajes a su padre. 
3. _dos_saltos_router_011
Código del nodo 011. Toma los mensajes que viene de sus hijos (nodos: 0111) y los reenvia hacia el nodo router 01. También reenvia sus propios mensajes a su padre. 
4. _dos_saltos_hoja
Código del nodo hoja. Genera el struct con la información relevada y la envia a su nodo padre.

## Funciones

El código base de los sketck 2 al 4 es el mismo, solo cambian un par de datos respecto a la correspondencia con cada nodo.

a) Seteamos de manera estática todos los nodos de la red. Si un mensaje necesita recorrer el árbol completo para encontrar su destino,
es necesario contar con la correspondencia entre las ramas y los nodos de toda la red. 

/***********************************************************************
************* Set the Node Address *************************************
/***********************************************************************/
// These are the Octal addresses that will be assigned
const uint16_t node_address_set[6] = { 00, 01, 011, 0111};

b) NODE_ADDRESS corresponde en el listado al nodo actual y MASTER_ADDRESS es el nodo destino. En nuestro caso, por defecto es el nodo master pero puede enviarse
un mensaje a cualquier otro nodo de la red.

uint8_t NODE_ADDRESS = 1;  // This is the number we have to change for every new node
uint8_t MASTER_ADDRESS = 0;  // Use numbers 0 through to select an address from the array

c) Se define el numero maximo de nodos en la red (posibles saltos) [node_address_set]

const short max_active_nodes = 4;            // Array of nodes we are aware of

bool send_T(uint16_t to);                      // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);
void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void add_node(uint16_t node);
