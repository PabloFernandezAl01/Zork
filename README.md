# West Zork

West Zork es una aventura conversacional desarrollada en C++ e inspirada en
los juegos clásicos de ficción interactiva.

Yellowville, Texas, 1870. El antiguo alguacil Willian Munny regresa al pueblo y
descubre que su hermano Ned ha desaparecido. Para encontrarlo deberá explorar
el pueblo, reunir pistas, manipular objetos y resolver los obstáculos que
protegen el acceso a la cripta bajo la iglesia.

El juego incluye ocho salas conectadas, inventario, contenedores, objetos con
estado, oscuridad y fuentes de luz, accesos bloqueados, puzles y condiciones de
victoria y derrota.

El diseño completo del juego se encuentra en el
[GDD de West Zork](Zork/Documentos/GDD_WestZork.md).

## Autor

Pablo Fernandez Alvarez

## Repositorio

Link: `https://github.com/PabloFernandezAl01/Zork`

## Compilación y ejecución

### Requisitos

- Windows.
- Visual Studio 2019 Community con la carga de trabajo **Desarrollo para el
  escritorio con C++**.
- Windows 10 SDK y toolset MSVC v142.

El proyecto utiliza C++14, plataforma x64 y nivel de advertencias `/W3`. No
requiere librerías externas.

### Desde Visual Studio

1. Abrir `Zork/Solution.sln`.
2. Seleccionar la plataforma `x64`.
3. Elegir la configuración `Debug` o `Release`.
4. Compilar la solución mediante **Compilar > Recompilar solución**.
5. Ejecutar `Zork/Ejecutables/ZorkDebug.exe` o
   `Zork/Ejecutables/ZorkRelease.exe`.

## Cómo jugar

Escribe una orden y pulsa Enter. Los comandos se introducen en español, en
minúsculas o mayúsculas indistintamente, pero sin tildes.

| Acción | Comandos |
| --- | --- |
| Movimiento | `norte`/`n`, `sur`/`s`, `este`/`e`, `oeste`/`o` |
| Observar | `mirar`/`m`, `examinar objeto`/`x objeto`, `examinar mapa` |
| Inventario | `inventario`/`i`, `coger objeto`, `soltar objeto` |
| Contenedores | `abrir contenedor`, `meter objeto en contenedor`, `sacar objeto de contenedor` |
| Acciones | `encender objeto`, `cargar objeto`, `romper objeto con herramienta`, `disparar objetivo` |
| Sistema | `ayuda`, `terminar` |

Algunas acciones permiten omitir la herramienta cuando el objeto correcto ya
está en el inventario. Por ejemplo, se aceptan tanto `cargar revolver con
municion` como `cargar revolver`.

### Imagen del mapa para orientarse

[Mapa de Yellowville](Zork/Documentos/Mapa.png).

## Guía para completar el juego

> **Aviso:** esta sección contiene la solución completa de la aventura.

La siguiente secuencia corresponde a una ruta óptima:

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

El farol debe permanecer encendido para explorar la celda y la cripta. Entrar
al enfrentamiento final con el revólver descargado conduce a la derrota.

## Implementación

West Zork está implementado como una aplicación de consola en C++14, sin
dependencias externas. La arquitectura separa el bucle de aplicación, el
análisis de comandos, el estado persistente del mundo y las reglas de juego:

- `WestZork` gestiona la entrada, la salida y el ciclo principal.
- `Parser` transforma el texto del jugador en comandos sin consultar el mundo.
- `GameWorld` mantiene el estado y coordina las reglas de gameplay.
- `WorldBuilder` construye las salas, los objetos y sus conexiones.
- `Entity`, `Room`, `Item` y `Player` modelan las entidades del juego.

Las salas se conectan mediante identificadores estables y los objetos se
comparten entre salas, inventario y contenedores mediante `std::shared_ptr`.
Las consultas internas utilizan punteros no propietarios. El contenido está
definido en código para mantener una entrega autocontenida, compatible con
C++11/C++14 y sin librerías de terceros.

Los detalles completos de arquitectura, ownership y decisiones técnicas se
encuentran en
[Implementación y arquitectura](Zork/Documentos/Implementacion.md).

## Licencia

El código fuente original de este proyecto se distribuye bajo la
[licencia MIT](LICENSE).

Este es un proyecto académico no oficial inspirado en *Zork* y no está afiliado
con sus propietarios.
