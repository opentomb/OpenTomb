OpenTomb — an open-source Tomb Raider 1-5 engine remake
-------------------------------------------------------

 Table of contents

1. What is this?
2. Why it's developed?
3. Features
4. Supported platforms
5. Configuration and autoexec files
6. Installation and running
7. Compiling
8. Licensing
9. Credits


1. What is this?
----------------

OpenTomb is an open-source engine reimplementation project intended to play levels from all
classic-era Tomb Raider games (TR 1—5) and custom TRLE levels. The project does not use any
old Tomb Raider source code, because all attempts to retrieve sources from Eidos / Core were
in vain.

Instead, everything is being developed completely from scratch.
However, OpenTomb uses certain legacy routines from such unfinished open-source projects as
OpenRaider and VT project (found at icculus.org), plus it incorporates some code from
Quake Tenebrae.

All in all, OpenTomb tries to recreate original Tomb Raider games experience, although with
contemporary updates, features and additions — to fully benefit from being running on modern
PCs with powerful CPUs and graphic cards — unlike original engines, which are getting older
and older (original engine, on which all classics were based, will turn 20 next year).

2. Why it's developed?
----------------------

Many may ask — why develop another TR engine clone, while we have fully working Windows
builds of TR2-5, and TR1 is perfectly working through DosBox? The answer is simple - the
older engine gets, less chance it'll become compatible with further systems; but in case of
OpenTomb, you can port it to any platform you wish.

Other people may ask — why we're developing it, if there are already patchers for existing
engines, like TREP, TRNG, etc.? The answer is simple — no matter how advanced your patcher
is, you are limited by original binary — no new features, no graphic enhancements, no new
structures and functions. You are not that limited with open-source engine.

3. Features
-----------

* OpenTomb has completely different collision approach. Engine uses special terrain
  generator to make every room's optimized collisional mesh from so-called "floordata",
  which was a significant limiting factor in originals.

* OpenTomb does not run at fixed 30 FPS speed, as any old engine did. Instead, variable FPS
  rate is implemented, just like in any contemporary PC game.

* OpenTomb uses common and flexible libraries, like OpenGL, OpenAL, SDL and Bullet Physics.

* Lua scripting is a key gameplay feature in OpenTomb, as all entity functionality is not
  hardcoded, as it was in classic engines, but moved into plain-text files, which can be
  modified and extended any time.

* Many abandoned and unused features from originals were enabled in OpenTomb. New animations,
  unused items, hidden PSX-specific structures inside level files, and so on! Also, original
  functionality is being drastically extended, while preserving original gameplay pipeline.

4. Supported platforms
----------------------

OpenTomb is a cross-platform engine — currently, you can run it on Windows, Mac or Linux.
No mobile implementations are made yet, but they are fully possible.

5. Configuration and autoexec files
-----------------------------------

Currently, all settings in OpenTomb are managed through configuration and autoexec files.
Configuration file contains persistent engine and game settings, while autoexec contains
any commands which should be executed on engine start-up.

Configuration file (config.lua) is divided into different sections: screen, audio, render,
controls, console and system. In each of these sections, you can change numerous parameters,
which names are usually intuitive to understand.

Autoexec file (autoexec.lua) is a simple command file which is executed at engine start-up,
just like you type them in the console. Basically, you shouldn't remove any existing commands
from autoexec, as most likely engine won't start properly then, but you can modify these
commands or add new ones — like changing start-up level by modifying setgamef(*,*) command.

6. Installation and running
---------------------------

You don't need to install OpenTomb, but ou need all classic TR game resources. Problem is,
these resources (except level files themselves) are tend to be in some cryptic formats or
incompatible across game versions. Because of this, you need to convert some game resources
by yourself or get them from somewhere on the Net. Anyway, here is the list of all needed
assets and where to get them:

 a) Data folders from each game. Get them from your retail game CDs or Steam/GOG bundles.
    Just take data folder from each game's folder, and put it into corresponding
    /data/tr*/ folder.
 b) CD audio tracks. OpenTomb only supports OGG audiotracks for a moment, so you should
    convert original soundtracks by yourself, or just download whole TR1-5 music package
    here: http://trep.trlevel.de/opentomb/files/tr_soundtracks_for_opentomb.zip
    PLEASE NOTE: script file bundled in this archive is outdated, so don't overwrite
    existing soundtrack.lua file with one provided in archive.
 c) Loading screens for TR1-3 and TR5. For TR3, get them from pix directory of your
    installed official game. Just put this pix directory into /data/tr3/ folder. As for
    other games, it's a bit tricky to get loading screens, as there were no loading
    screens for PC versions TR1-2, TR4 used level screenshots as loading screens, and TR5
    used encrypted format to store all loading graphics. So, to ease your life, you can
    simply download loading screen package here: http://trep.trlevel.de/temp/loading_screens.zip
    Just put it right into OpenTomb directory, and that should do the trick.
    
7. Compiling
------------

There is a CMakeLists.txt file provided with source code, so you can compile OpenTomb using
CMake. On Windows, you can also compile it from Code::Blocks IDE (project file is also provided).
Alternatively, you can manually compile it in Code::Blocks by recursively adding all source files
from /src directory, and adding these libraries in Linker Settings under Project Build options:

* libmingw32.a
* libglu32.a
* libopengl32.a
* libuser32.a
* libgdi32.a
* libwinmm.a
* libmm32.a
* libole32.a
* liboleaut32.a
* libversion.a
* libuuid.a
* libSDL2main.a
* libSDL2.a
* libSDL2_image.a
* libz.a
* libpthreadGC2.dll.a
* libpthread.a

On Linux, just download the source code and run in terminal:

    cmake . && make
    
Necessary dependencies are development headers for SDL2, SDL2 Image, GLU, ZLIB. You can install
them in Ubuntu-based distro with this command:

    sudo apt-get install libsdl2-dev libsdl2-image-dev libglu1-mesa-dev zlib1g-dev

You can also fully compile engine using build.sh file in /src folder.
On Mac, use XCode project, which is also available in source code.

NB: Please note that OpenTomb requires C++11 (-std=c++11) flag to compile properly!
You may use CPU-specific optimization flags ("-march=prescott", "march=i486", "march=core2"),
as well as general optimization flags (-O1 and -O2), but DON'T USE -O3 flag, as Bullet tends to
crash with this optimization level.

8. Licensing
------------

OpenTomb is an open-source engine distributed under LGPLv3 license, which means that ANY part of
the source code must be open-source as well. Hence, all used libraries and bundled resources must
be open-source with GPL-compatible licenses. Here is the list of used libraries and resources and
their licenses:

* OpenGL - does not need licensing (http://opengl3.org/about/licensing/)
* OpenAL Soft - LGPL
* SDL / SDL Image - zlib
* Bullet - zlib
* Lua - MIT
    
* Droid Sans Mono, Roboto Condensed Regular and Roboto Regular fonts - Apache
    
9. Credits
----------

NB: Please note that authors and contributors list is constantly extending, as there is more and
more people involved in project development, so someone may be missing from this list!

* TeslaRus: main developer.
* Cochrane: renderer rewrites and optimizing, Mac OS X support.
* Gh0stBlade: renderer add-ons, shader port, gameflow implementation, state control fix-ups, camera programming.
* Lwmte: state and scripting fix-ups, controls, GUI and audio modules, trigger and entity system rewrites.
* Nickotte: interface programming, ring inventory implementation, camera fix-ups.
* pmatulka: Linux port and testing.
* Richard_trle: Github migration, Github repo maintenance, website design.
* Saracen: room and static mesh lighting.
* T4Larson: general stability patches and bugfixing.
* vobject: nightly builds, maintaining general compiler compatibility.

Additional contributions from: Ado Croft (extensive testing), E. Popov (TRN caustics shader port),
godmodder (general help), jack9267 (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), Tonttu (console patch) and xythobuz (additional Mac compatibility patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian), SuiKaze Raider (Spanish).