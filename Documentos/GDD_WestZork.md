# GDD de referencia — West Zork / mi version del juego Zork

Este documento tiene solo la información relevante al gameplay de mi version.

`West Zork` es el titulo del juego. `Yellowville` es el nombre del pueblo en el que se desarrolla la historia.

## 1. El jugador, el mundo y la historia

### 1.1 El jugador

El jugador encarna a un antiguo alguacil retirado, Willian Munny, que vuelve a Yellowville tras una visita a un amigo en un pueblo vecino y se da cuenta de que su hermano no aparece.

### 1.2 El mundo

La historia se desarrolla en Yellowville, un pueblo del Oeste, Texas, EEUU, en el año 1870. 

### 1.3 La historia

Yellowville era un pueblo en el que había mucha delincuencia, lo que acabó con la vida de varios Sheriffs anteriormente. El Sheriff actual, Little Bill, 
estaba al límite combatiendo la delincuencia y viendo como su pueblo se vaciaba debido a esto mismo. El hermano de Willian, Ned Munny, no era menos y
tenía un historial de delincuencia abultado aunque recientemente estaba muy tranquilo. Un día ocurrio un robo a mano armada en un bar, Little Bill lo investigó
y concluyó que el principal sospechoso era Ned. Por ello, se lo llevó silenciosamente a la Cripta Bajo la Iglesia para terminar con su vida. Willian, que volvía
ese día de visitar a un amigo en un pueblo vecino, llegó al pueblo y: ahí comienza el juego. 

La idea del juego es:

- Entrar al Establo y coger el farol necsario para ver en habitaciones oscuras
- Entrar al Saloon y leer la nota del tabernero. Lo que te redirige a la oficina del Sheriff. 
- Entrar a la oficina del Sheriff coger el revolver. Entrar en la Celda y coger la cruz de Plata. 
- Entrar a la Iglesia rompiendo las cadenas con la cizalla.
- Entrar a la Cripta con la cruz de Plata y disparar a Little Bill. 

## 2. Objetivo principal

El objetivo es averiguar que esta ocurriendo en el pueblo y buscar pistas para encontrar a su hermano.

## 3. Comandos principales

### 3.1 Comandos de movimiento

El jugador puede moverse con abreviaturas o palabras completas:

- Norte / N
- Sur / S
- Este / E
- Oeste / O

### 3.2 Comandos de observación

Mirar -> (Muestra información sobre el lugar en el que se encuentra el jugador)
Examinar [objeto] -> (Muestra inforamacion sobre el objeto que menciona el jugador, si es un objeto contendor se muestra informacion de todos los objetos dentro)
Mirar/Examinar mapa -> (Muestra las conexiones de la habitacion actual)

### 3.3 Comandos de inventario

Inventario -> (muestra todos los objetos que tiene el jugador)
Coger [objeto] -> (Mete un objeto a la mochila)
Soltar [objeto] -> (Saca un objeto de la mochila y lo deja en el lugar en el que este)
Meter [objeto] en [objeto contendor] -> (Hay objeto que permiten llevar otros objetos)
Sacar [objeto] de [objeto contendor]

Regla de contenedores: un objeto contenedor no puede meterse dentro de otro objeto contenedor.

### 3.4 Comandos de uso

Abrir caja fuerte con llave
Encender farol
Cargar revolver
Romper cadenas con cizalla

No hay un botón genérico de “usar”. El jugador debe expresar una acción concreta.

### 3.5 Diagrama resumen de comandos

movimiento:
  norte / n / sur / s / este / e / oeste / o

sin objetivo:
  mirar / m
  inventario / i
  ayuda / help
  terminar / quit

un objetivo:
  mirar [objeto]
  examinar [objeto]
  coger [objeto]
  soltar [objeto]
  abrir [objeto]
  encender [objeto]
  cargar [objeto]
  disparar [objetivo]

dos objetivos:
  meter [objeto] en [contenedor]
  sacar [objeto] de [contenedor]
  romper [objeto] con [herramienta]

opcionalmente útil:
  abrir [objeto] con [llave]
  cargar [arma] con [municion]

### 3.6 Filosofía de la ayuda durante la partida

El comando `ayuda` funciona como una referencia breve de las familias de
comandos, no como un listado exhaustivo de todas las frases aceptadas por el
parser ni como un walkthrough.

De forma deliberada, la ayuda no incluye las órdenes exactas para entrar en la
iglesia ni para abrir el acceso a la cripta. El jugador debe deducirlas mediante
la exploración, las descripciones de los objetos y las pistas del mundo.

## 4. Sistema de inventario

### 4.1 Objetos portables

- Botella whisky vacia
- Mapa rasgado
- Cizalla oxidada
- Saco usado que apesta a patatas podridas (objeto contenedor)
- Llave pequeña
- Caja fuerte (objeto contenedor) (SI, PUEDE SER PORTABLE)
- Revolver sin municion
- Municion
- Diario del sheriff
- Cruz de plata
- Farol apagado
- Cerillas
- Sombrero de Ned Munny
- Nota del tabernero

Cada objeto puede tener:

- Nombre principal.
- Descripción.
- Si es contenedor.
- Si emite luz.
- Si es arma.

### 4.2 Objetos con estados

El farol puede estar:

- Encendido
- Apagado

*Decision*: Por simplicidad, no hay comando para apagar farol

No tiene limite de duracion. 

El revolver puede estar:

- Cargado
- Descargado

*Decision*: Por simplicidad, no hay comando para descargar el revolver. Una vez cargado se consume el item Municion

## 5. Sistema de luz y oscuridad

Uno de los sistemas más importantes de Zork es la oscuridad.

### 5.1 Habitaciones oscuras

La celda trasera y la cripta al norte de la iglesia estan oscuras. El jugador
necesita que haya una fuente de luz encendida en su inventario o en la sala
para localizar o manipular objetos presentes en ellas. Por tanto, un farol
encendido que se deje en el suelo sigue iluminando la estancia y puede
recuperarse.

Los objetos que ya estan en el inventario pueden manipularse por tacto. Sin
embargo, el mapa, el diario del sheriff y la nota del tabernero necesitan luz
para poder leerse.

Una fuente de luz encendida no puede guardarse dentro de un contenedor.

## 6. Sistema de vida, daño y muerte

En West Zork el unico combate que existe es con el sheriff. O bien el jugador le mata o bien el jugador muere. Todo esto ocurre en la cripta al norte de la iglesia. 

## 7. Zonas principales y los items que contienen

- Entrada al pueblo: mapa rasgado y botella whisky vacia. 
- Establo abandonado: farol apagado y cerillas. 
- Calle Principal: cizalla oxidada y saco usado que apesta a patatas podridas.
- Saloon: llave pequeña y caja fuerte. La nota del tabernero está dentro de la caja fuerte.
- Oficina del Sheriff: revolver descargado, municion y diario del Sheriff. 
- Celda trasera: cruz de plata.
- Iglesia vieja: sombrero de Ned Munny.
- Cripta al norte de la iglesia: NPC del sheriff.

## 8. Puzles

- La llave pequeña abre la caja fuerte y la celda
- La puerta de la iglesia tiene cadenas que se rompen con la cizalla.
- El acceso del muro norte de la iglesia que conduce a la cripta tiene una cerradura en forma de cruz que se abre con la cruz de plata.

## 9. NPC

Sheriff Little Bill: se encuentra en la sala final (cripta al norte de la iglesia). No es un NPC como tal, solo se le menciona en la
descripcion de la Cripta bajo la Iglesia. 

## 10. Diseño del mundo

Salas interconectadas. Por simplicidad todas las conexiones son bidireccionales.

## 11. Ruta óptima (menor numero de comandos requeridos)

```text
este
coger farol
coger cerillas
encender farol
oeste
norte
coger cizalla
oeste
coger llave
este
este
coger revolver
coger municion
cargar revolver
abrir celda con llave
este
coger cruz
oeste
oeste
romper cadenas
norte
abrir cerradura
norte
disparar sheriff
```

## 12. Victoria/Derrota

El jugador gana si dispara al Sheriff con Revolver cargado
El jugador pierde si intenta disparar al Sheriff con Revolver descargado

## 13. Sistemas

### 13.1 Sistema de salas

Cada sala contiene:

- Nombre.
- Descripción.
- Salidas.
- Flags: oscura.
- Objetos.

### 13.2 Sistema de objetos

Cada objeto contiene:

- Nombre.
- Sinónimos.
- Descripción.
- Si es Contenedor.
- Si es Fuente de luz.
- Si es Arma.

La localizacion de un objeto no se guarda como una propiedad propia. Se deduce
de la coleccion que lo contiene: el vector de objetos de una sala, el
inventario del jugador o el contenido de otro objeto.

## 14. Extra features

- Sistema de oscuridad y luz
- Conexiones bloqueadas que requieren herramientas.
- Items especiales con comportamientos específicos
- Sistema de victoria/derrota
