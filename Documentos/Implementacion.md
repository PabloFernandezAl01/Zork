# Implementacion y arquitectura de West Zork

Este documento recoge informacion tecnica que no deberia vivir en el GDD:
arquitectura, responsabilidades de clases, ownership, decisiones de
implementacion y deuda tecnica. El contenido jugable concreto: salas, objetos,
ruta ideal, puzles, comandos disponibles y condiciones de victoria/derrota: se
mantiene en el GDD.

## Enfoque general

El proyecto esta planteado como una aventura conversacional C++ sencilla, sin
librerias externas, con una separacion clara entre:

- Bucle de aplicacion.
- Parsing de texto.
- Estado persistente del mundo.
- Reglas que modifican ese estado.

La arquitectura prioriza legibilidad y bajo acoplamiento frente a sistemas
genericos mas complejos. Para el alcance de una prueba de 7 dias, la decision
principal es mantener pocas clases, responsabilidades explicitas y datos faciles
de inspeccionar en debugger.

## Flujo de ejecucion

El flujo runtime es:

1. `main()` crea `WestZork`.
2. `WestZork` contiene `GameWorld` y `Parser`.
3. El constructor de `GameWorld` solicita a `WorldBuilder::Build()` un
   `WorldData` con las salas, los items y la sala inicial.
4. `GameWorld` mueve esos datos a sus mapas y coloca al jugador en la sala
   inicial.
5. `WestZork::Run()` muestra el estado inicial del mundo.
6. Cada linea de input se convierte en un `Command` mediante `Parser::Parse()`.
7. `GameWorld::ExecuteCommand()` interpreta ese `Command`, modifica el mundo si
   procede y devuelve un `GameResult`.

`WestZork` no contiene reglas de gameplay. Su responsabilidad es ser la capa de
aplicacion: entrada, salida y control del bucle principal.

El bucle continua mientras el resultado sea `GameResult::Running`. Los
resultados terminales distinguen victoria, derrota, salida voluntaria y error
interno, por lo que las acciones no necesitan modificar un booleano por
referencia.

## Capas y responsabilidades

### `Entity`

Clase base comun para entidades con identidad textual:

- `id`
- nombre
- descripcion

Es una clase abstracta: `PrintInformation()` es virtual pura y cada clase
derivada decide como completa su representacion. Tiene destructor virtual porque
se usa como base de `Room`, `Item` y `Player`. Los getters devuelven referencias
constantes para evitar copias innecesarias. El identificador, el nombre y la
descripcion quedan fijados en el constructor y no tienen setters. Esta
inmutabilidad evita desincronizar el nombre de una sala respecto a la copia
almacenada en sus conexiones.

### `Room`

Modelo de datos de una sala. Guarda:

- Salidas como `std::map<Direction, Exit>`. Cada salida contiene el `id` y el
  nombre visible de la sala destino, ademas de su estado de bloqueo.
- Objetos presentes como `std::vector<std::shared_ptr<Item>>`.
- Un flag de oscuridad.

`Room` no tiene un flag global de "sala bloqueada": el bloqueo pertenece a una
salida concreta. Tampoco decide reglas globales ni sabe si el jugador tiene el
objeto necesario para abrir un paso. Solo expone estado y operaciones locales
como anadir o retirar items.

### `Item`

Modelo de datos de un objeto interactivo. Se usa una unica clase cuyas
capacidades y estados se expresan mediante enums:

- `ContainerState`: `NotApplicable`, `Open`, `Closed` o `Locked`.
- `LightState`: `NotApplicable`, `Off` u `On`.
- `LoadState`: `NotApplicable`, `Unloaded` o `Loaded`.

Un estado distinto de `NotApplicable` indica que el item tiene esa capacidad.
Por ejemplo, el farol tiene `LightState::Off` al comenzar y el revolver tiene
`LoadState::Unloaded`. El saco empieza vacio y cerrado, y la caja fuerte
bloqueada.

Los contenedores guardan su contenido en un
`std::vector<std::shared_ptr<Item>>`. Solo permiten buscar, anadir o retirar
objetos cuando estan abiertos; `GameWorld` valida las reglas y evita introducir
un contenedor dentro de otro.

La propiedad `RequiresLightToExamine` marca los objetos legibles del GDD que no
se pueden interpretar a oscuras: mapa, diario del sheriff y nota del tabernero.

Cada item tambien guarda alias explicitos y sabe:

- Comparar un objetivo del jugador mediante `MatchesTarget()`.
- Imprimir nombre y descripcion.
- Mostrar su estado de luz o carga.
- Mostrar si un contenedor esta bloqueado, cerrado, vacio o que contiene.

### `Player`

Guarda estado propio del jugador:

- `currentRoomId`
- inventario

No ejecuta comandos ni conoce el mapa. El jugador es una entidad de datos con
operaciones locales sobre inventario y posicion. Las reglas que conectan
jugador, salas e items pertenecen a `GameWorld`.

### `Parser`

Convierte texto crudo en un `Command`.

El parser normaliza el input con trim y minusculas, detecta la intencion
principal y rellena objetivos textuales. No consulta el mundo, no valida si un
objeto existe y no decide si una accion es posible.

Esta separacion permite evolucionar el lenguaje aceptado sin tocar las reglas de
gameplay. Tambien permite que `GameWorld` concentre el feedback semantico:
objeto inexistente, salida bloqueada, contenedor invalido, etc.

El parser conserva objetivos compuestos, elimina articulos iniciales y separa
comandos con conectores como `en`, `de` o `con` sin consultar el mundo. Usa la
ultima aparicion del conector, por lo que `sacar cruz de plata de caja fuerte`
se divide correctamente. Tambien reconoce acciones especiales concretas sin
introducir un comando generico de "usar".

Ademas de las formas documentadas en el GDD, acepta `x` como abreviatura de
`examinar`, `tomar` como sinonimo de `coger` y `tirar` como sinonimo de
`soltar`.

Los comandos con herramienta opcional aceptan ambas formas. Por ejemplo:
`abrir caja fuerte` y `abrir caja fuerte con llave`, `cargar revolver` y
`cargar revolver con municion`, `romper cadenas` y `romper cadenas con cizalla`,
o `disparar sheriff` y `disparar sheriff con revolver`.

Si se omite la herramienta, `GameWorld` intenta inferir el item correcto desde
el inventario. El GDD y la ayuda runtime mantienen las formas explicitas como
sintaxis canonica de los puzles.

### `Command`

Objeto de datos que representa la intencion parseada:

- tipo de comando
- direccion, si aplica
- primer objetivo textual
- segundo objetivo textual, si aplica

Se mantiene sin comportamiento para que el parsing y la ejecucion sigan
desacoplados.

### `GameWorld`

Es el propietario del estado principal y el coordinador de reglas.

Guarda:

- `Player m_player`
- `std::map<std::string, std::shared_ptr<Room>> m_rooms`
- `std::map<std::string, std::shared_ptr<Item>> m_items`

Ejecuta las acciones porque casi todas afectan a varias entidades a la vez. Por
ejemplo, recoger un objeto modifica sala e inventario; mover al jugador consulta
sala actual, salida y sala destino; operar con contenedores puede implicar
inventario, sala actual y objeto destino.

El constructor obtiene el contenido fijo mediante `WorldBuilder::Build()`,
coloca al jugador usando `WorldData::initialRoomId` y mueve los mapas
resultantes a su estado persistente. `GameWorld` no instancia directamente las
salas ni los items.

El sheriff del GDD no se instancia como `Entity` ni existe una clase `NPC`.
Actualmente se representa como una regla de `ShootTarget()`: el objetivo textual
debe coincidir con uno de sus aliases contextuales y el jugador debe encontrarse
en la cripta. Esta decision es suficiente mientras sea el unico personaje y no
tenga comportamiento autonomo.

### `WorldBuilder` y `WorldData`

`WorldBuilder` traduce el contenido fijo del GDD a objetos C++ sin contener
reglas de gameplay. Sus responsabilidades son:

- Instanciar y registrar salas e items.
- Configurar oscuridad y estados iniciales.
- Crear las conexiones bidireccionales y sus bloqueos de entrada.
- Colocar los items en las salas o contenedores indicados por el GDD.
- Indicar la sala inicial.

`WorldData` es un objeto de transferencia temporal con los mapas de salas e
items y el `id` inicial. Tras construirlo, `GameWorld` mueve sus colecciones y
se convierte en el propietario del estado runtime.

`GameWorld` no permite copia. Copiar sus mapas de `shared_ptr` produciria dos
mundos aparentemente independientes que compartirian salas e items mutables. El
constructor y el operador de asignacion de copia estan eliminados
explicitamente. `WestZork` aplica la misma restriccion porque contiene un
`GameWorld`.

## Ownership y acceso

`WorldBuilder` registra inicialmente salas e items en mapas por `id`, y
`GameWorld` recibe esos registros mediante movimiento. Durante la partida, las
salas, el inventario y los contenedores comparten items mediante
`std::shared_ptr<Item>`.

Las busquedas internas que devuelven entidades, como `FindRoomById()`, usan
punteros raw no propietarios:

- Pueden devolver `nullptr`.
- Permiten acceso puntual a entidades registradas.
- No transfieren ownership.
- No autorizan al llamador a destruir la entidad.

Se evita devolver `std::shared_ptr` en consultas simples para no comunicar
ownership compartido cuando solo se necesita observar o modificar una entidad
que ya pertenece al mundo.

## Identificadores y busqueda de objetivos

Las entidades tienen un `id` estable y textual. El `id` se usa para:

- Registrar entidades en los mapas de `GameWorld`.
- Conectar salas mediante salidas.
- Mover referencias entre sala, inventario y contenedores.
- Aplicar reglas concretas de los puzles sin depender del nombre visible.

Los identificadores internos estan centralizados en `WorldIds.h` mediante
`PlayerIds`, `RoomIds` e `ItemIds`.

Las acciones sobre objetos resuelven el texto del jugador contra una coleccion
concreta: objetos de la sala, inventario o contenido de un contenedor. Esa
busqueda contextual vive en `GameWorld`, pero la equivalencia entre texto e item
pertenece a `Item`.

`FindAccessibleItem()` busca primero en el inventario y despues en la sala
actual, siempre que haya visibilidad. El contenido de un contenedor solo se
consulta mediante el comando `sacar`; no se realiza una busqueda recursiva.

Cada `Item` puede declarar alias explicitos ademas de su nombre visible.
`Item::MatchesTarget()` normaliza el texto y exige coincidencia exacta contra
una de esas formas validas. El `id` no se compara con input del jugador porque
es informacion interna del sistema. Se evita la busqueda parcial por substring
porque podia producir falsos positivos y hacer que el primer objeto parecido
capturase la accion.

El parser entrega objetivos completos a `GameWorld`, por lo que los alias
conviven con nombres visibles compuestos. Esto permite comandos cortos como
`coger mapa` o `examinar caja`, pero tambien formas completas como
`coger mapa rasgado`, `examinar caja fuerte` o `coger cruz de plata`.

Los elementos de escenario que no son `Item` se resuelven mediante
`ResolveScenarioTarget()`. Este helper consulta primero la sala actual y solo
despues compara el texto con los aliases validos en ese contexto. Asi,
`cerradura` puede identificar la caja fuerte en el saloon y el acceso a la
cripta en la iglesia sin introducir una interpretacion global ambigua. Sus
colecciones de texto estan centralizadas en `WorldAliases.h`, separadas de los
identificadores internos estables de `WorldIds.h`.

## Salidas y direcciones

Las conexiones entre salas se representan como datos en `Room`, no como punteros
directos. Cada salida guarda el `id` y el nombre visible de la sala destino.
`GameWorld` resuelve el `id` durante el movimiento y `Room` usa el nombre
almacenado para listar las conexiones al examinar el mapa.

Ventajas:

- Evita referencias circulares entre salas.
- Hace sencillo construir el mapa en cualquier orden.
- Permite detectar destinos mal configurados con un `nullptr`; se consideran una
  violacion interna de invariantes y terminan con `GameResult::FatalError`.

El estado de bloqueo tambien pertenece a la salida. De este modo puede
bloquearse un acceso concreto sin convertir toda la sala destino en una
habitacion bloqueada desde cualquier direccion.

El mapa esta construido con conexiones de ida y vuelta, como indica el GDD, pero
cada direccion es un `Exit` independiente. Por eso los tres bloqueos iniciales
se aplican solo al acceso de entrada:

- Calle Principal -> Iglesia vieja.
- Oficina del Sheriff -> Celda trasera.
- Iglesia vieja -> Cripta.

`Direction`, el parser y el mundo solo admiten conexiones cardinales: norte,
sur, este y oeste. Los comandos sin conexiones reales se han excluido del
modelo, del parser y del GDD para que toda direccion documentada sea utilizable.

## Luz y oscuridad

`Room::IsDark()` representa una propiedad permanente del entorno. Encender el
farol no modifica la sala; `GameWorld::CanPlayerSee()` combina la oscuridad de
la sala con el estado del inventario para decidir si el jugador puede verla.

Una sala es visible si no es oscura, si el jugador lleva una fuente de luz
encendida o si hay una fuente encendida entre los objetos de la propia sala. La
consulta se reparte entre `Player::HasTurnedOnLightSource()` y
`Room::HasTurnedOnLightSource()`, y `GameWorld::CanPlayerSee()` combina ambos
resultados. Esta regla permite recuperar un farol encendido despues de dejarlo
en el suelo de una sala oscura.

Los objetos que ya estan en el inventario se pueden examinar y manipular por
tacto a oscuras. Los objetos legibles —el mapa, el diario del sheriff y la nota
del tabernero— son la excepcion y requieren luz incluso dentro del inventario.
Esta propiedad se modela mediante un flag de `Item`, sin deducirla a partir de
su ID ni del texto introducido por el jugador. En cambio, la oscuridad impide
descubrir, examinar, recoger, abrir o usar como contenedor los objetos de la
sala. Al encender una fuente de luz dentro de una sala oscura se vuelve a
ejecutar `Look()` para mostrar inmediatamente lo que acaba de quedar visible.

Una fuente de luz encendida no puede introducirse en un contenedor. La regla
evita ocultar la unica luz disponible dentro de un saco y crear otro estado sin
posibilidad de recuperar visualmente el farol.

## Correspondencia entre sistemas del GDD y codigo

- La caja fuerte empieza bloqueada, contiene la nota del tabernero y se abre con
  la llave pequena. La herramienta puede indicarse o inferirse del inventario.
- El saco usado que apesta a patatas podridas es un contenedor portable que
  empieza cerrado y se puede abrir sin herramienta.
- La misma llave desbloquea la salida este de la Oficina del Sheriff hacia la
  celda.
- La cizalla desbloquea la salida norte de Calle Principal hacia la iglesia.
- La cruz de plata desbloquea la salida norte de la iglesia hacia la cripta.
- Las cerillas son necesarias para encender el farol; no se consumen y el farol
  no se puede apagar.
- La municion se consume al cargar el revolver.
- La caja fuerte es portable, igual que el resto de items colocados en salas.
- `examinar mapa` muestra las salidas de la sala actual mediante
  `Room::PrintExits()`.
- Disparar al sheriff en la cripta con el revolver cargado devuelve
  `GameResult::Victory`. Intentarlo con el revolver descargado devuelve
  `GameResult::Defeat`.
- No existe un sistema general de combate, vida o daño, de acuerdo con el
  alcance reducido del GDD.

## Decisiones de implementacion

### `GameWorld` ejecuta comandos

Las reglas se concentran en `GameWorld` porque es la unica clase que conoce
jugador, salas, items registrados y estado global. Esto evita que `WestZork`
crezca como clase dios y evita que `Player`, `Room` o `Item` necesiten conocer
demasiado del resto del sistema.

Cada accion devuelve un `GameResult`. Los fallos de invariantes devuelven
`FatalError`, mientras que los comandos que no terminan la partida devuelven
`Running`. `WestZork` es la unica clase que decide si el loop debe continuar y
que mensaje final mostrar.

### Salida desacoplada mediante `std::ostream`

`WestZork::Run()` decide que la salida actual del juego es `std::cout` y se la
pasa a `GameWorld::ExecuteCommand()` como `std::ostream&`. `GameWorld` usa ese
stream para todo el feedback de gameplay, en vez de escribir directamente en la
salida estandar.

Esta decision reduce el acoplamiento entre reglas de juego y consola. Permite
redirigir la salida a otros destinos, como un archivo, un transcript de partida
o un `std::ostringstream` para pruebas, sin cambiar la logica del mundo. Las
violaciones de invariantes se apoyan en `assert` durante desarrollo y se
propagan como `GameResult::FatalError` cuando existe una comprobacion defensiva
equivalente.

### Parser sin estado de mundo

`Parser` solo expresa intencion. No valida existencia ni disponibilidad de
entidades. Esta decision mantiene el parser testeable y evita dependencias
circulares con `GameWorld`.

### Entrada limitada a ASCII simple

Los comandos y objetivos introducidos por el jugador se definen usando ASCII
simple, sin tildes, eñes ni otros caracteres especiales. Por ejemplo, la forma
esperada es `municion`, no `munición`.

Esta restriccion es una decision consciente para el alcance de la prueba. La
normalizacion actual mediante trim y conversion a minusculas es suficiente, y no
se añadira soporte Unicode, normalizacion de diacriticos ni alias acentuados. La
ayuda y el contenido que enseñe comandos al jugador deben mantener las formas
ASCII aceptadas por el parser.

### Busquedas internas con punteros raw no propietarios

Los metodos internos de busqueda de `GameWorld`, como `FindRoomById()`,
devuelven punteros raw en vez de `std::shared_ptr`. La razon es semantica: esas
funciones no crean ownership nuevo ni comparten la vida util de la entidad; solo
ofrecen acceso puntual a un objeto que ya esta registrado y poseido por
`GameWorld`.

El puntero raw expresa dos cosas importantes:

- La busqueda puede fallar y devolver `nullptr`.
- El llamador no es propietario de la entidad encontrada.

Devolver `std::shared_ptr` en estas consultas comunicaria que quien llama
participa en la vida util del objeto, permitiria conservar entidades mas tiempo
del esperado y anadiria coste innecesario por incremento/decremento del contador
de referencias. Para este proyecto, `std::shared_ptr` se reserva para los
lugares donde si hace falta compartir referencias persistentes, como items que
se mueven entre sala, inventario y contenedores.

### Resolucion de nombres de items en `Item`

`Item` conoce las formas textuales validas por las que el jugador puede
referirse a ese objeto: nombre visible y alias explicitos. Por eso la
comparacion de objetivos se implementa en `Item::MatchesTarget()`, no en
`Parser` ni como logica interna de `GameWorld`.

El `id` queda reservado para uso interno: registro en mapas, conexiones,
busquedas tecnicas y movimiento de referencias entre sistemas. Renombrar un `id`
no deberia cambiar los comandos que entiende el jugador.

`Parser` no debe conocer objetos concretos del mundo; su trabajo termina al
producir un `Command` con objetivos textuales. `GameWorld` conserva la busqueda
contextual porque sabe si debe mirar en la sala actual, inventario o contenedor,
pero delega la equivalencia textual en cada item.

### Contenido hardcodeado por ahora

No se usa JSON ni otra carga externa para mantener la entrega autocontenida y
compatible con C++11/C++14 sin dependencias. El contenido esta centralizado en
`WorldBuilder`, separado de las reglas de `GameWorld`, aunque sigue compilado
directamente en el ejecutable. Para esta prueba es aceptable; si el proyecto
creciera, `WorldBuilder` podria convertirse en el adaptador de una fuente de
datos externa.

### Una unica clase `Item`

La clase `Item` con enums de capacidad y estado es suficiente para el alcance
actual. Crear subclases ahora anadiria mas estructura que comportamiento real.
Si los objetos empiezan a tener reglas propias complejas, las alternativas
serian:

- Subclases especializadas.
- Componentes pequenos para capacidades concretas.

Para la prueba, los enums actuales hacen explicitos los estados necesarios sin
introducir una jerarquia prematura.

### Se acepta la posibilidad de estados inconsistentes en `Item`

Los setters de `Item` permiten configurar de forma independiente
`ContainerState`, `LightState` y `LoadState`. Por tanto, tecnicamente seria
posible crear combinaciones sin sentido para el juego, como un revolver que
tambien fuese un contenedor o una fuente de luz.

Soy consciente de que una API mas restrictiva, una jerarquia de tipos o un
sistema de componentes podria impedir estas combinaciones por construccion. Sin
embargo, todos los objetos se crean y configuran en un unico punto controlado,
`WorldBuilder::InitializeItems()`, y actualmente existen muy pocos tipos de
objeto. Las configuraciones son faciles de revisar y no proceden de datos
externos.

Para esta prueba tecnica acepto este riesgo de forma consciente. Introducir
ahora una solucion mas estricta aumentaria la complejidad y el volumen de codigo
sin aportar una mejora perceptible al gameplay. Si el proyecto creciera o el
contenido se cargase desde archivos externos, esta decision deberia revisarse.

### Las salidas son value objects

Cada conexion se representa mediante un `Exit` ligero que contiene el `id` y el
nombre visible de la sala destino, ademas de un booleano de bloqueo. El nombre
evita resolver la sala para mostrar las conexiones del mapa; el `id` continua
siendo la referencia estable usada para moverse. No se ha creado una jerarquia
de puertas ni una clase con reglas propias porque las condiciones concretas para
abrir cada acceso siguen siendo reglas de gameplay coordinadas por `GameWorld`.

Esta decision permite modelar correctamente la puerta de la celda, las cadenas
de la iglesia y la cerradura de la cripta. Solo se bloquea la direccion de
entrada correspondiente; la sala destino y su salida inversa permanecen
independientes.
