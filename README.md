[![Build Status](https://travis-ci.org/opentomb/OpenTomb.svg?branch=master)](https://travis-ci.org/opentomb/OpenTomb)

OpenTomb — an open-source Tomb Raider 1-5 engine remake
-------------------------------------------------------

### Table of contents ###

- [What is this?](#what-is-this)
- [Why create a new engine?](#why-create-a-new-engine)
- [Features](#features)
- [Supported platforms](#supported-platforms)
- [Setup](#setup)
- [Compiling](#compiling)
- [Running and Configuration](#running-and-configuration)
- [Licensing](#licensing)
- [Credits](#credits)


### What is this? ###
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

Links to forums and info:
* TR forum link: http://www.tombraiderforums.com/showthread.php?t=197508
* Discord channel: https://discord.gg/d8mQgdc

### Why create a new engine? ###
Many may ask — why develop another TR engine clone, while we have fully working Windows
builds of TR2-5, and TR1 is perfectly working through DosBox? The answer is simple - the
older engine gets, the less chance it'll become compatible with new systems; but in case of
OpenTomb, you can port it to any platform you wish.

Other people may ask — why are we developing it, if there are already patchers for existing
engines, like TREP, TRNG, etc.? The answer is simple — no matter how advanced your patcher
is, you are limited by the original binary — no new features, no graphic enhancements, no new
structures and functions. You are not limited in such ways with an open-source engine.

### Features ###
* OpenTomb has completely different collision approach. The engine uses a special terrain
  generator to make every room's optimized collisional mesh from so-called "floordata",
  which was a significant limiting factor in originals.  
* OpenTomb is not limited to 30 FPS, as any old engine did. Instead, a variable FPS
  rate is implemented, just like in any contemporary PC game.  
* OpenTomb uses common and flexible libraries, like OpenGL, OpenAL, SDL and Bullet Physics.  
* Lua scripting is a key gameplay feature in OpenTomb, as all entity functionality is not
  hardcoded, as it was in classic engines, but moved into plain-text files, which can be
  modified and extended any time.  
* Many abandoned and unused features from originals were enabled in OpenTomb. New animations,
  unused items, hidden PSX-specific structures inside level files, and so on! Also, original
  functionality is being drastically extended, while preserving original gameplay pipeline.

### Supported platforms ###
OpenTomb is a cross-platform engine — currently, you can run it on Windows, Mac or Linux.
No mobile implementations are made yet, but they are fully possible.

### Setup ###
You don't need to install OpenTomb, but you need all classic TR game resources. Problem is,
these resources (except level files themselves) tend to be in some cryptic formats, or
incompatible across game versions. Because of this, you'll need to convert some game resources yourself,
or get them from somewhere on the Net. Anyway, here is the list of all needed assets and where to get them:

 * Data folders from each game. Get them from your retail game CDs or Steam/GOG bundles.
   Just take data folder from each game's folder, and put it into corresponding
   /data/tr*/ folder. For instance, for TR3, the path would be OpenTomb/data/tr3/data/
   
 * CD audio tracks. OpenTomb only supports OGG audiotracks for a moment, so you should
   convert original soundtracks by yourself, or just download whole TR1-5 music package
   here: https://opentomb.earvillage.net/  
   PLEASE NOTE: Files may need to be renamed for this to work, please see: https://github.com/opentomb/OpenTomb/issues/447
   
 * Loading screens for TR1-3 and TR5. For TR3, get them from pix directory of your
   installed official game. Just put this pix directory into /data/tr3/ folder. As for
   other games, it's a bit tricky to get loading screens, as there were no loading
   screens for PC versions TR1-2, TR4 used level screenshots as loading screens, and TR5
   used encrypted format to store all loading graphics. So, to ease your life, you can
   simply download loading screen package here: http://trep.trlevel.de/temp/loading_screens.zip  
   Just put it right into OpenTomb directory, and that should do the trick. Note: engine supports 
   png and pcx format of screen images.
    
### Compiling ###
There is a CMakeLists.txt file provided with source code, so you can compile OpenTomb using
CMake. On Windows, you can also compile it from Code::Blocks IDE (project file is also provided).
Alternatively, you can manually compile it in Code::Blocks by recursively adding all source files
from /src directory, and adding these libraries in Linker Settings under Project Build options:

* libmingw32.a
* libSDL2main.a
* libSDL2.dll.a
* liblua.a
* libpng.a
* libz.a
* libpthread.a

On Linux, just download the source code and run in terminal:

    cmake . && make
    
Necessary dependencies are development headers for SDL2, png, LUA 5.2, ZLIB. You can install
them in Ubuntu-based distro with this command:

    sudo apt-get install libopenal-dev libsdl2-dev libpng12-dev liblua5.2-dev libglu1-mesa-dev zlib1g-dev

On Mac, use XCode project, which is also available in source code.

NB: Please note that OpenTomb requires C++11 (-std=c++11) flag to compile properly!
You may use CPU-specific optimization flags (-march=prescott, -march=i486, -march=core2),
as well as general optimization flags (-O1 and -O2), but DON'T USE -O3 flag, as Bullet tends to
crash with this optimization level (GCC 5.1+ may compile it without errors).

### Running and Configuration ###
To run OpenTomb, simply run the executable generated by the build. By default, no command line options 
are needed. Access the console by pressing `. This allows you to enter commands to select levels, change 
settings, and more. Enter 'help' to get a list of commands. Enter 'exit' to quit the engine.

Currently, all settings in OpenTomb are managed through config.lua and autoexec.lua.
Config.lua contains persistent engine and game settings, while autoexec.lua contains
any commands which should be executed on engine start-up.

Config.lua is divided into different sections: screen, audio, render, controls, console and system. 
In each of these sections, you can change numerous parameters, the names of which are usually fairly intuitive.  

Autoexec.lua is a simple list of commands which are ran at startup. Modifying existing commands may cause the engine to
function incorrectly.

To select a level, enter 'setgamef(game, level) into either autoexec.lua or in the console, where game is 1-5. Mansion 
levels are generally 0, and games which do not have a mansion begin from level 1. For example, to load level 2 of TR3, 
you would enter setgamef(3, 2).

### Licensing ###
OpenTomb is an open-source engine distributed under LGPLv3 license, which means that ANY part of
the source code must be open-source as well. Hence, all used libraries and bundled resources must
be open-source with GPL-compatible licenses. Here is the list of used libraries and resources and
their licenses:

* OpenGL — does not need licensing (https://www.opengl.org/about/#11)
* OpenAL Soft — LGPL
* SDL / SDL Image — zlib
* Bullet — zlib
* Freetype2 — GPL
* Lua — MIT
* ffmpeg rpl format and codecs (http://git.videolan.org/)

* Droid Sans Mono, Roboto Condensed Regular and Roboto Regular fonts — Apache
    
### Credits ###
NB: Please note that authors and contributors list is constantly extending, as there is more and
more people involved in project development, so someone may be missing from this list!

* TeslaRus: main developer.
* Cochrane: renderer rewrites and optimizing, Mac OS X support.
* Gh0stBlade: renderer add-ons, shader port, gameflow implementation, state control fix-ups, camera and AI programming.
* Lwmte: state and scripting fix-ups, controls, GUI and audio modules, trigger and entity system rewrites.
* Nickotte: interface programming, ring inventory implementation, camera fix-ups.
* pmatulka: Linux port and testing.
* Richard_trle: Github migration, Github repo maintenance, website design.
* Saracen: room and static mesh lighting.
* T4Larson: general stability patches and bugfixing.
* vobject: nightly builds, maintaining general compiler compatibility.
* vvs: testing, feedback, bug report.
* xproger: documentation updates.

Additional contributions from: Ado Croft (extensive testing), E. Popov (TRN caustics shader port),
godmodder (general help), jack9267 (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), Tonttu (console patch) and xythobuz (additional Mac compatibility patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian), SuiKaze Raider (Spanish).
