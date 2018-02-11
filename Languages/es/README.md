[![Build Status](https://travis-ci.org/opentomb/OpenTomb.svg?branch=master)](https://travis-ci.org/opentomb/OpenTomb)

OpenTomb — una versión de código abierto del motor Tomb Raider 1-5
-------------------------------------------------------

### Tabla de contenidos ###

- [¿Qué es esto?](#¿qué-es-esto?)
- [¿Por qué crear un nuevo motor?](#¿por-qué-crear-un-nuevo-motor?)
- [Características](#características)
- [Plataformas compatibles](#plataformas-compatibles)
- [Configuración](#configuración)
- [Compilando](#compilando)
- [Ejecución y configuración](#ejecución-y-configuración)
- [Licencia](#licencia)
- [Créditos](#créditos)


### ¿Qué es esto? ###
OpenTomb es una reimplementación de código abierto del motor clásico de Tomb Raider, destinado a jugar niveles de todos los juegos de Tomb Raider de la era clásica (1—5), así como niveles TRLE personalizados. El proyecto no usa nada del código original de Tomb Raider, ya que todos los intentos de recuperar los archivos fuente de Eidos/Core fueron en vano.

En cambio, todo está siendo re-desarrollado completamente desde cero. Debería ser señalado, sin embargo, que OpenTomb utiliza ciertas rutinas heredadas desde proyectos inacabados de código abierto, como [OpenRaider](http://openraider.sourceforge.net/) y el proyecto de VT (que se encuentra en [icculus.org](https://icculus.org/)), más algún código de Quake Tenebrae.

OpenTomb intenta recrear la experiencia original de la serie Tomb Raider, aunque con actualizaciones contemporáneas, características y adiciones, pudiendo beneficiarse completamente de ejecutarse en PCs modernas con potentes CPUs y tarjetas gráficas.

Enlaces a foros e información:
* Enlace al foro TR: http://www.tombraiderforums.com/showthread.php?t=197508
* Canal de Discord: https://discord.gg/d8mQgdc

### ¿Por qué crear un nuevo motor? ###
Es cierto que tenemos versiones completas de Windows de TR2-5, y TR1 funciona perfectamente a través de [DosBox](https://www.dosbox.com/). Sin embargo, a como el tiempo progrese la situación solo empeorará, con Sistemas Operativos más nuevos cada vez es menos probable que apoye los juegos. OpenTomb siempre será capaz de ser portado a cualquier plataforma que desee.

También es cierto que hay parches para el motor original, con el objetivo de mejorarlo y actualizarlo: TREP, TRNG, etc. La ventaja con OpenTomb es que no están limitados por el Binario original, una gran limitación cuando se trata de nuevas características, mejoras gráficas, modificación de código y más. Un motor de fuente abierta elimina estas limitaciones.

### Características ###
* OpenTomb tiene una aproximación de colisión completamente diferente al motor original,
eludiendo muchas de las limitaciones presentes. Usamos un generador de terreno para
hacer una malla de colisión optimizada para cada habitación a partir de los llamados "datos de suelo".
* OpenTomb es capaz de una velocidad de fotogramas variable, no limitada a 30 fps como el motor original.
* OpenTomb utiliza bibliotecas comunes y flexibles, como OpenGL, OpenAL, SDL y
Bullet Physics.
* OpenTomb implementa un motor de scripts Lua para definir toda la funcionalidad de la entidad. Esto significa que, de nuevo, a diferencia del original, mucho menos está codificado en el motor en sí, por lo que la funcionalidad se puede ampliar o modificar sin necesidad de modificar y recompilar el motor en sí.
* Se han habilitado muchas características abandonadas y no utilizadas del motor original
en OpenTomb. Nueva animación, elementos no utilizados, estructuras ocultas específicas de PSX en el interior
archivos de nivel, ¡y así!.

### Plataformas compatibles ###
OpenTomb es un motor multiplataforma: actualmente se puede ejecutar en Windows, Mac oLinux. Aún no se han desarrollado implementaciones móviles, pero son de hecho posibles.

### Configuración ###
Para ejecutar cualquiera de los niveles de los juegos originales, necesitará los activos de ese juego respectivo. Estos recursos a menudo tienden a estar en formatos crípticos, con variaciones en los juegos. Debido a esto, necesitará convertir algunos recursos del juego a formatos utilizables usted mismo u obtenerlos de algún lugar de la red.

Aquí está la lista de todos los activos necesarios y dónde conseguirlos:

 * Carpetas de datos de cada juego. Obtenlos de tus CD de juegos al por menor o manojos Steam/GOG. Simplemente toma la carpeta de datos de la carpeta de cada juego y ponla en correspondiente carpeta `/data/tr*/`. Por ejemplo, para TR3, el camino sería `OpenTomb/data/tr3/data/`

 * CD pistas de audio. OpenTomb solo admite pistas de audio OGG por un momento, por lo que debería convertir las bandas sonoras originales ud. mismo, o simplemente descargar todo el paquete de música TR1-5 aquí: https://opentomb.earvillage.net TENGA EN CUENTA: es posible que sea necesario cambiar el nombre de los archivos para que funcionen, por favor vea https://github.com/opentomb/OpenTomb/issues/447

 * Pantallas de carga para TR1-3 y TR5. Para TR3, obténgalas del directorio de píxeles de tu juego oficial instalado. Solo coloque este directorio de píxeles en `/data/tr3/` carpeta. En cuanto a otros juegos, es un poco complicado obtener pantallas de carga, ya que no había pantallas de carga para las versiones de PC TR1-2, TR4 usaba capturas de pantalla de nivel como pantallas de carga, y TR5 utilizó un formato encriptado para almacenar toda la carga gráficos. Por lo tanto, para facilitar su vida, simplemente puede descargar el paquete de pantalla de carga aquí: http://trep.trlevel.de/temp/loading_screens.zip Solo póngalo directamente en el directorio de OpenTomb, y debería funcionar. Nota: el motor admite el formato png y pcx de imágenes de pantalla.

### Compilando ###
Hay un archivo CMakeLists.txt provisto con el código fuente, por lo que puede compilarOpenTomb usando CMake. En Windows, también puede compilarlo desde Code::Blocks IDE(el archivo del proyecto también se proporciona). Alternativamente, puede compilarlo manualmente en Code::Blocks recursivamente agregando todos los archivos fuente del directorio /src, y agregando estas bibliotecas en Configuraciones del Enlazador en Opciones de Compilación del proyecto:

* libmingw32.a
* libSDL2main.a
* libSDL2.dll.a
* liblua.a
* libpng.a
* libz.a
* libpthread.a

En Linux, solo descarga el código fuente y ejecuta en la terminal:

    cmake . && make

Las dependencias requeridas son los encabezados de desarrollo para SDL2, png, LUA 5.2,
ZLIB. Puede instalarlos en una distribución basada en Ubuntu con este comando:

    sudo apt-get install libopenal-dev libsdl2-dev libpng12-dev liblua5.2-dev libglu1-mesa-dev zlib1g-dev

En Mac, use el proyecto XCode, que también está disponible en el código fuente.

NB: tenga en cuenta que OpenTomb requiere el flag C++11 (`-std=c++11`) para compilar correctamente! Puede usar indicadores de optimización específicos de CPU (`-march=prescott`,`-march=i486`,`-march=core2`), así como los indicadores generales de optimización (`-O1` y `-O2`), pero NO USE el flag `-O3`, ya que Bullet tiende a bloquearse con este nivel de optimización (GCC 5.1+ puede compilarlo sin errores).

### Ejecución y configuración ###
Para ejecutar OpenTomb, simplemente corre el ejecutable generado por la compilación. Por defecto, no se necesitan opciones de línea de comando. Acceda a la consola presionando \`. Esta le permite ingresar comandos para seleccionar niveles, cambiar configuraciones y más. Entre a 'ayuda' para obtener una lista de comandos. Ingrese 'salir' para salir del motor.

Actualmente, todas las configuraciones en OpenTomb se administran a través de config.lua y
autoexec.lua. Config.lua contiene ajustes persistentes del motor y del juego,
autoexec.lua contiene los comandos que se deben ejecutar en el arranque del motor.

Config.lua se divide en diferentes secciones: pantalla, audio, render, controles,
consola y sistema. En cada una de estas secciones, puede cambiar numerosos
parámetros, los nombres de los cuales son generalmente bastante intuitivos.

Autoexec.lua es una lista simple de comandos que se ejecutan al inicio. Modificando los comandos existentes puede hacer que el motor funcione incorrectamente.

Para seleccionar un nivel, ingrese 'setgamef(juego, nivel) en autoexec.lua o en la consola, donde el juego es 1-5. Los niveles de mansión son generalmente 0, y los juegos que no tengan una mansión que comience en el nivel 1. Por ejemplo, para cargar el nivel 2 de TR3, usted ingresaría `setgamef(3, 2)`.

### Licencia ###
OpenTomb es un motor de código abierto distribuido bajo licencia LGPLv3, lo que significaque CUALQUIER parte del código fuente debe ser también de código abierto. Por lo tanto, todas las bibliotecas utilizadas y los recursos incluidos deben ser de código abierto con licencias compatibles con GPL. Aquí está la lista de bibliotecas y recursos usados y sus licencias:

* OpenGL — no necesita licencia (https://www.opengl.org/about/#11)
* OpenAL Soft — LGPL
* Imagen SDL / SDL — zlib
* Bullet — zlib
* Freetype2 — GPL
* Lua — MIT
* ffmpeg rpl formato y codecs (http://git.videolan.org/)

* Fuentes Droid Sans Mono, Roboto Condensed Regular y Roboto Regular — Apache

### Créditos ###
NB: tenga en cuenta que la lista de autores y colaboradores se extiende constantemente, como cada vez hay más personas involucradas en el desarrollo del proyecto, por lo que alguien puede estar faltando en esta lista!

* [TeslaRus](https://github.com/TeslaRus): desarrollador principal.
* [Cochrane](https://github.com/Cochrane): el renderizador reescribe y optimiza la compatibilidad con Mac OS X.
* [Gh0stBlade](https://github.com/Gh0stBlade): complementos de renderizador, puerto shader, implementación del flujo de juego, arreglos de control de estado, cámara y programación de IA.
* [Lwmte](https://github.com/Lwmte): correcciones de estado y secuencias de comandos, controles, GUI y módulos de audio, desencadenador y reescrituras del sistema de entidades.
* Nickotte: programación de interfaz, implementación de inventario de anillo,
arreglos de cámara.
* [pmatulka](https://github.com/pmatulka): Puerto de Linux y pruebas.
* [richardba](https://github.com/richardba): Migración de Github, mantenimiento de repositorios en Github, diseño de sitio web.
* [Saracen](https://github.com/Saracen): iluminación de malla estática y habitación.
* [T4Larson](https://github.com/T4Larson): parches de estabilidad general y corrección de errores.
* [vobject](https://github.com/vobject): construcciones nocturnas, manteniendo la compatibilidad general del compilador.
* [vvs-](https://github.com/vvs-): pruebas, comentarios, informe de errores.
* [xproger](https://github.com/xproger): actualizaciones de documentación.
* [Banderi](https://github.com/Banderi): documentación, corrección de errores.
* [gabrielmtzcarrillo](https://github.com/gabrielmtzcarrillo): trabajo del sombreado de la entidad.
* [filfreire](https://github.com/filfreire): documentación.


Contribuciones adicionales de: Ado Croft (pruebas extensas),
E. Popov (Puerto sombreador TRN caustics), [godmodder](https://github.com/godmodder) (ayuda general),
[jack9267](https://github.com/jack9267) (optimización del cargador vt), meta2tr (pruebas y seguimiento de errores),
shabtronic (arreglos de renderizador), [Tonttu](https://github.com/Tonttu) (parche de consola) y
[xythobuz](https://github.com/xythobuz) (parches adicionales de compatibilidad de Mac).

Traducciones por: [Joey79100](https://github.com/Joey79100) (Francés), Nickotte (Italiano), [Lwmte](https://github.com/Lwmte) (Ruso),
[SuiKaze Raider](https://twitter.com/suikazeraider) (Español), [filfreire](https://github.com/filfreire)
