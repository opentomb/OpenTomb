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
OpenTomb is an open-source re-implementation of the classic Tomb Raider engine,
intended to play levels from all classic-era Tomb Raider games (1—5), as well as
custom TRLE levels. The project does not use any of the original Tomb Raider
code, as all attempts to retrieve source files from Eidos/Core were in vain.

Instead, everything is being re-developed completely from scratch. It should be
noted, however, that OpenTomb uses certain legacy routines from unfinished
open-source projects, such as OpenRaider and VT project (found at icculus.org),
plus some code from Quake Tenebrae.

OpenTomb tries to recreate the original Tomb Raider series experience, although
with contemporary updates, features and additions, being able to fully benefit
from running on modern PCs with powerful CPUs and graphic cards.

Links to forums and info:
* TR forum link: http://www.tombraiderforums.com/showthread.php?t=197508
* Discord channel: https://discord.gg/d8mQgdc

### Why create a new engine? ###
It's true that we have fully working Windows builds of TR2-5, and TR1 works
perfectly through DosBox. However, as time progresses the situation will only
worsen, with newer Operating Systems becoming increasingly unlikely to support
the games. OpenTomb will always be able to be ported to any platform you wish.

It is also true that there are patchers for the original engine, aiming to
improve and update it: TREP, TRNG, etc. The advantage with OpenTomb is that we
are not limited by the original Binary, a huge limitation when it comes to new
features, graphical enhancements, code modification and more. An open-source
engine removes these limitations.

### Features ###
* OpenTomb has a completely different collision approach to the original engine,
circumventing many of the limitations present. We use a terrain generator to
make an optimized collision mesh for each room from so-called "floordata".
* OpenTomb is capable of a variable frame rate, not limited to 30fps like the
original engine.  
* OpenTomb uses common and flexible libraries, such as OpenGL, OpenAL, SDL and
Bullet Physics.  
* OpenTomb implements a Lua scripting engine to define all entity functionality.
 This means that, again, unlike the original, much less is hardcoded into the
 engine itself, so functionality can be extended or modified without havng to
 modify and recompile the engine itself.
* Many abandoned and unused features from the original engine have been enabled
in OpenTomb. New animation, unused items, hidden PSX-specific structures inside
level files, and so on!

### Supported platforms ###
OpenTomb is a cross-platform engine: currently it can be ran on Windows, Mac or
Linux. No mobile implementations are in development yet, but they are indeed
possible.

### Setup ###
To run any of the levels from the original games, you will need the assets from
that respective game. These resources often tend to be in cryptic formats, with
variations across games. Because of this, you'll need to convert some game
resources to usable formats yourself, or get them from somewhere on the Net.

Here is the list of all needed assets and where to get them:

 * Data folders from each game. Get them from your retail game CDs or Steam/GOG
 bundles. Just take data folder from each game's folder, and put it into
 corresponding /data/tr*/ folder. For instance, for TR3, the path would be
 OpenTomb/data/tr3/data/

 * CD audio tracks. OpenTomb only supports OGG audiotracks for a moment, so you
 should convert original soundtracks by yourself, or just download whole TR1-5
 music package here: https://opentomb.earvillage.net
 PLEASE NOTE: Files may need to be renamed for this to work, please see
  https://github.com/opentomb/OpenTomb/issues/447

 * Loading screens for TR1-3 and TR5. For TR3, get them from pix directory of
 your installed official game. Just put this pix directory into /data/tr3/
 folder. As for other games, it's a bit tricky to get loading screens, as there
 were no loading screens for PC versions TR1-2, TR4 used level screenshots as
 loading screens, and TR5 used an encrypted format to store all loading
 graphics. So, to ease your life, you can simply download loading screen package
  here: http://trep.trlevel.de/temp/loading_screens.zip  
 Just put it right into OpenTomb directory, and that should do the trick. Note:
 the engine supports png and pcx format of screen images.

### Compiling ###
There is a CMakeLists.txt file provided with source code, so you can compile
OpenTomb using CMake. On Windows, you can also compile it from Code::Blocks IDE
(project file is also provided). Alternatively, you can manually compile it in
Code::Blocks by recursively adding all source files from /src directory, and
adding these libraries in Linker Settings under Project Build options:

* libmingw32.a
* libSDL2main.a
* libSDL2.dll.a
* liblua.a
* libpng.a
* libz.a
* libpthread.a

On Linux, just download the source code and run in terminal:

    cmake . && make

The required dependencies are the development headers for SDL2, png, LUA 5.2,
ZLIB. You can install them in an Ubuntu-based distro with this command:

    sudo apt-get install libopenal-dev libsdl2-dev libpng12-dev liblua5.2-dev libglu1-mesa-dev zlib1g-dev

On Mac, use XCode project, which is also available in source code.

NB: Please note that OpenTomb requires C++11 (-std=c++11) flag to compile
properly! You may use CPU-specific optimization flags (-march=prescott,
-march=i486, -march=core2), as well as general optimization flags (-O1 and -O2),
 but DON'T USE -O3 flag, as Bullet tends to crash with this optimization level
 (GCC 5.1+ may compile it without errors).

### Running and Configuration ###
To run OpenTomb, simply run the executable generated by the build. By default,
no command line options are needed. Access the console by pressing `. This
allows you to enter commands to select levels, change settings, and more. Enter
'help' to get a list of commands. Enter 'exit' to quit the engine.

Currently, all settings in OpenTomb are managed through config.lua and
autoexec.lua. Config.lua contains persistent engine and game settings, while
autoexec.lua contains any commands which should be executed on engine start-up.

Config.lua is divided into different sections: screen, audio, render, controls,
console and system. In each of these sections, you can change numerous
parameters, the names of which are usually fairly intuitive.  

Autoexec.lua is a simple list of commands which are ran at startup. Modifying
existing commands may cause the engine to function incorrectly.

To select a level, enter 'setgamef(game, level) into either autoexec.lua or in
the console, where game is 1-5. Mansion levels are generally 0, and games which
do not have a mansion begin from level 1. For example, to load level 2 of TR3,
you would enter setgamef(3, 2).

### Licensing ###
OpenTomb is an open-source engine distributed under LGPLv3 license, which means
that ANY part of the source code must be open-source as well. Hence, all used
libraries and bundled resources must be open-source with GPL-compatible
licenses. Here is the list of used libraries and resources and their licenses:

* OpenGL — does not need licensing (https://www.opengl.org/about/#11)
* OpenAL Soft — LGPL
* SDL / SDL Image — zlib
* Bullet — zlib
* Freetype2 — GPL
* Lua — MIT
* ffmpeg rpl format and codecs (http://git.videolan.org/)

* Droid Sans Mono, Roboto Condensed Regular and Roboto Regular fonts — Apache

### Credits ###
NB: Please note that authors and contributors list is constantly extending, as
there is more and more people involved in project development, so someone may be
 missing from this list!

* TeslaRus: main developer.
* Cochrane: renderer rewrites and optimizing, Mac OS X support.
* Gh0stBlade: renderer add-ons, shader port, gameflow implementation, state
control fix-ups, camera and AI programming.
* Lwmte: state and scripting fix-ups, controls, GUI and audio modules, trigger
and entity system rewrites.
* Nickotte: interface programming, ring inventory implementation,
camera fix-ups.
* pmatulka: Linux port and testing.
* Richard_trle: Github migration, Github repo maintenance, website design.
* Saracen: room and static mesh lighting.
* T4Larson: general stability patches and bugfixing.
* vobject: nightly builds, maintaining general compiler compatibility.
* vvs: testing, feedback, bug report.
* xproger: documentation updates.

Additional contributions from: Ado Croft (extensive testing),
E. Popov (TRN caustics shader port), godmodder (general help),
jack9267 (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), Tonttu (console patch) and
xythobuz (additional Mac compatibility patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian),
SuiKaze Raider (Spanish).
