OpenTomb — an open-source Tomb Raider 1-5 engine remake
-------------------------------------------------------

[![Build Status](https://drone.io/github.com/opentomb/OpenTomb/status.png)](http://sourceforge.net/projects/opentomb/files/NightlyBuilds/)

### Table of contents ###

1. What is this?
2. Why is it developed?
3. Features
4. System requirements
5. Supported platforms
6. Configuration and autoexec files
7. Installation and running
8. Compiling
9. Licensing
10. Credits


1. What is this?
----------------

OpenTomb is an open-source engine reimplementation project intended to play levels from all
classic-era Tomb Raider games (TR 1—5) including TRLE levels. The project does not use any of
the original Tomb Raider source code, because all attempts to retrieve sources from Eidos / Core
were in vain.

Instead, everything is being developed completely from scratch.
However, OpenTomb uses certain legacy routines from unfinished open-source projects such as
OpenRaider and VT project (found at icculus.org), plus it incorporates some code from
Quake Tenebrae.

All in all, OpenTomb is an attempt to recreate the original Tomb Raider experience, along with
contemporary updates, features and additions — to fully benefit from running on modern
PCs with powerful CPUs and graphic cards — unlike the original engines, which are getting older.
The original engine, on which all classics were based on will turn 20 next year!

2. Why it's developed?
----------------------

Many may ask — "Why develop another TR engine clone, while we have fully working Windows
builds of TR2-5, and TR1 is perfectly working through DosBox?". The answer is simple - the
older the engine gets, the lower the chance it'll be compatible with future systems; but in case of
OpenTomb, you can port it to any platform you wish due to usage of many cross-platform libraries.

Other people may ask — "Why we're developing it". If there are already patchers for existing
engines, like TREP, TRNG, etc.? The answer is simple — no matter how advanced your patcher
is, you are limited by the original binary meaning: no new features, no graphical enhancements and
no new structures or functions. You are not that limited with open-source engine.

3. Features
-----------

* OpenTomb has a completely different collision approach. The Engine uses a special terrain
  generation algorithm to convert every room's optimized collisional mesh from so-called "floordata",
  which was a significant limiting factor in the original engine.  
* OpenTomb does not run at fixed 30 FPS, as any old engine did. Instead, variable FPS
  rate is implemented, just like in any contemporary PC game.  
* OpenTomb uses common and flexible libraries such as OpenGL, OpenAL, SDL and Bullet Physics.  
* Lua scripting is a key feature in OpenTomb, all entity functionality is not hardcoded like 
  it was in the original engines. Lua scripts are game script files which can be
  modified and extended any time providing the ability to manipulate several OpenTomb level factors.
* Many abandoned and unused features from originals were enabled in OpenTomb. New animations,
  unused items, hidden PSX-specific structures inside level files, and so on! Also, original
  functionality is being drastically extended, while preserving original gameplay pipeline.

4. System requirements
----------------------

OpenTomb should run fine on any contemporary computer, but you **absolutely** need OpenGL 2.1
compliant videocard (with support of VBOs). Also, make sure you have latest drivers installed
for your videocard, as OpenTomb may use some other advanced OpenGL features.

5. Supported platforms
----------------------

OpenTomb is a cross-platform engine — currently, you can run it on Windows, Mac or Linux.
No mobile ports have been made yet, but they are fully possible.

6. Configuration and autoexec files
-----------------------------------

Currently, all settings in OpenTomb are managed through configuration and autoexec files.
Configuration file contains persistent engine and game settings, while autoexec contains
any commands which should be executed on engine start-up.

Configuration file (**config.lua**) is divided into different sections: screen, audio, render,
controls, console and system. In each of these sections, you can change numerous parameters,
which names are usually intuitive to understand.  
Autoexec file (**autoexec.lua**) is a simple command file which is executed at engine start-up,
just like you type them in the console. Basically, you shouldn't remove any existing commands
from autoexec, as most likely engine won't start properly then, but you can modify these
commands or add new ones — like changing start-up level by modifying setgamef() command.

7. Installation and running
---------------------------

You don't need to install OpenTomb, but you need the classic TR game resources for the specifc
games you'd like to play within OpenTomb. 
Problem is, these resources (except level files) tend to be in some cryptic formats or
are incompatible across game versions. Because of this, you need to convert some game resources
by yourself or get them from somewhere on the Net. Anyway, here is the list of all needed
assets and where to get them:

 * Data folders from each game. Get them from your retail game CDs or Steam/GOG bundles.
   Just take data folder from each game's folder, and put it into the corresponding
   /data/tr*/ folder. 
  
   An example level path is: "root/data/tr1/data/level1.phd".
   Where "root" is the folder containing OpenTomb.exe. GOG versions may have these files
   in a separate file called GAME.GOG. This can be simply renamed to GAME.ISO then mounted
   as a standard ISO file revealing the "/DATA/" folders.
   
 * CD audio tracks. OpenTomb supports OGG audiotracks (for TR1/TR2), CDAUDIO.WAD file (for TR3),
   PCM and MS-ADPCM wave files (for TRLE and TR4/5 respectively). For TR1/TR2, you can
   convert the original soundtracks by yourself or just download the whole TR1-2 music 
   package here: https://www.dropbox.com/s/fm3qpdhnbzntkml/tr1-2_soundtracks_for_opentomb.zip?dl=0
   
 * Loading screens for TR1-3 and TR5. For TR3, get them from pix directory of your officially
   installed game. Just copy or move the pix directory into /data/tr3/ within OpenTomb's folder.
   As for other games, it's a bit tricky to get loading screens. There were no loading screens for
   PC versions of TR1-2, and TR4 used level screenshots as loading screens. TR5 used an encrypted
   format to store all loading screen files. So, to ease your life, you can simply download the
   loading screen package here: https://www.dropbox.com/s/uycdw9x294ipc0r/loading_screens.zip?dl=0
   Just extract these files directly into the main OpenTomb directory, and that should do the trick.
    
8. Compiling
------------

To compile OpenTomb, primarily you need libs and defines for these external libraries:

* Bullet 2.83 (https://github.com/bulletphysics/bullet3/releases)
* Freetype 2.3.5 (http://www.freetype.org/download.html)
* GLEW 1.12.0 (http://glew.sourceforge.net/)
* libsndfile 1.0.25 (http://www.mega-nerd.com/libsndfile/#Download)
* libogg 1.3.2 (http://xiph.org/downloads/)
* libvorbis 1.3.5 (http://xiph.org/downloads/)
* Lua 5.3.1 (http://www.lua.org/download.html)
* OpenAL Soft 1.16.0 (http://kcat.strangesoft.net/openal.html#download)
* SDL 2.0.3 (http://libsdl.org/download-2.0.php)
* SDL_Image 2.0 - for Win/Linux (https://www.libsdl.org/projects/SDL_image/)
* zlib (http://www.zlib.net/ or http://xmlsoft.org/sources/win32/64bit/)

We recommend compiling using CMake. There is a CMakeLists.txt file provided with source code.
You can automatically generate Visual Studio, Eclipse and Code::Blocks projects with CMake as well.

On Linux, there is an easy way to compile engine just in three steps. First of all, install all the
necessary libraries with this command:

    sudo apt-get install libbullet-dev libfreetype6-dev libglu1-mesa-dev libglew-dev liblua5.2-dev libopenal-dev libogg-dev libvorbis-dev libsndfile1-dev libsdl2-dev libsdl2-image-dev
    
Then, download the code with all third-party submodules:

    git clone --recursive https://github.com/opentomb/OpenTomb.git
    
And then, compile it:

    cmake . && make

NB: Please note that OpenTomb requires GNU C++11 extensions (-std=gnu++11) flag to compile properly!
You may use CPU-specific optimization flags (-march=prescott, -march=i486, -march=core2),
as well as general optimization flags (-O1 and -O2), but DON'T USE -O3 flag, as Bullet tends to
crash with this optimization level.

9. Licensing
------------

OpenTomb is an open-source engine distributed under LGPLv3 license, which means that ANY part of
the source code must be open-source as well. Hence, all used libraries and bundled resources must
be open-source with GPL-compatible licenses. Here is the list of used libraries, resources and
their licenses:

* OpenGL — does not need licensing (http://opengl3.org/about/licensing/)
* GLEW — GLEW BSD
* OpenAL Soft — LGPL
* libsndfile — LGPL
* libogg/libvorbis — Xiph BSD
* SDL / SDL Image — zlib
* Bullet — zlib
* Freetype — GPL
* Lua — MIT
* zlib — zlib

* Droid Sans Mono, Roboto Condensed Regular and Roboto Regular fonts — Apache
    
10. Credits
----------

NB: Please note that the authors and contributors list is constantly extending! There are more and
more developers getting involved in the development of OpenTomb so some recent ones may be missing
from this list!

* TeslaRus: main developer.
* Cochrane: renderer rewrites and optimizing, Mac OS X support.
* Gh0stBlade: renderer add-ons, shader port, gameflow implementation, state fix-ups, camera.
* Lwmte: state fix-ups, controls, GUI and audio modules, trigger and entity scripts.
* Nickotte: interface programming, ring inventory implementation, camera fix-ups.
* pmatulka: Linux port and testing.
* Richard_trle: Github migration, Github repo maintenance, website design.
* Saracen: room and static mesh lighting.
* stohrendorf: CXX-fication, general code refactoring and optimizing.
* T4Larson: general stability patches and bugfixing.
* vobject: nightly builds, maintaining general compiler compatibility.
* vvs-: extensive testing and bug reporting.

Additional contributions from: Ado Croft (extensive testing), E. Popov (TRN caustics shader port),
godmodder (general help), jack9267 (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), Tonttu (console patch) and xythobuz (additional Mac patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian), SuiKaze Raider (Spanish).
