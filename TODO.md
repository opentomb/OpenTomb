OpenTomb — TODO list for high-priority bugs / tasks
---------------------------------------------------

### Table of contents ###

1. The main plan
2. Build configuration
3. Code in main
4. Engine
5. Collision system
6. Character controller
7. Animation control
8. Camera control
9. Scripting
10. Audio


1. The main plan
----------------
First we need to implement TR1 gameplay, so TR1/2/3 functions tasks have higher priority over TR4/TR5 functions. The first objective is to make a simple, but stable and working version, then extend functionality to it step by step.

2. Build configuration
----------------------
* Todo:
	* Update internal image lib
	* Reduce number of dependecies
	* Make good autobuild system
	  

3. Code in main
---------------
* Todo:
	* `game.cpp`: many different logics in one place, needs to be refactored
	* make some modules (not all!) interface more abstract (hide internal realisation, like `physics.h`/`physics_bullet.cpp`)

4. Engine
-------------------
* Current situation:
	* Implemented entity spawning and safety deleting, projectiles, player switching...

* Todo:
	* Reduce globals using (shared between modules globals)
	* Move out the console.c rendering code 

5. Collision system
-------------------
* Current situation:
	* Fixed back/front-facing polygons orientation for physics geometry, now the engine has a working _Filtered Ray Test_ (skips back-faced polygons)
	* Collision margin is zero, otherwise normals in near edges become smooth and Lara slides down or stops in places she should not
	* Refactored collision callbacks implementation that allow to register hit damage and any other collisions
  
* Todo:
	* Fix moving after landing on sloped surface:
		* Find body parts that stop Lara
		* Tune collision form, or disable collision checking for them
		* Bind with 3
	* Make rigid body parts shapes tunable by config (partially done)
	* For future optimization, add switchable single ghost object for character
	* Add _Long Ray Test_ (pierces rooms portals and builds room list for collisional checking) - needed for long range shooting and AI
	* Re-implement Character_FixPosByFloorInfoUnderLegs(...) it has been deleted
	* Check room tween butterfly normals

6. Character controller
-----------------------
* Todo:
	* Base AI (tune behavior, mud), path finding (fix boxes filtering logic), ...
	* Fix usage of weapons while crouching (no target targeting forward)
	* Add correct implementation for TR2+ weapons

7. Animation control
--------------------
* Todo:
	* Update documentation about `ss_animation` structure and functions
	* Fix incorrect smoothing if there are _move_ or _rotate_ anim commands
	* Fix dive-rolls:
		* Roll is not commencing immediately while moving forward
		* Rolls distance too great (e.g. falling off 1x1m ledges when on opposite edge)
	* Fix forward and backward consecutive jump rolls (mid-air rolls) not concatenating correctly on keypress (TR2+)
	* Fix swan dive not doable when jumping off of irregular (diamond shaped <>) slopes
	* Fix edge climbing:
		* Just fix configuration in state control

8. Camera control
-----------------
* Todo:
	* Add special `camera_entity`, store it in world module, access by `entity_p World_GetCameraEntity();` - needed for heavy triggers

9. Scripting
------------
* Current situation:
	* SEE TRIGGERS_tasks.md
	* Scripts update EVERY game frame! Use the global engine frame time inside time depended scripts!
* Todo:
	* Add function like `lua_SaveTable(...)` that recursively print to file/buffer/clay tablets lua code with table content (i.e. `table_name = { red = 1; green = 0; blue = 0; name = "name"; is_u = true; in_tbl = { p1 = "inner"; val = 32.45 } }`)
	* In all scripts that may change game state, data must be stored in special global table (that will be saved in save game) - needed for game save/load functions to correctly work

10. Audio
---------
* Current situation:
	* Sound tracks playing was disabled
	* AL build-in library works on Windows, Linux and MacOS, but some people prefer to use only native AL library
* Todo:
	* Implement own audio routine thread (APIs like `Audio_Send(...)` allow that)
	* Use something else instead of Vorbis (it can't read _OGG_ from memory, and uses default functions for files opening, so engine can't precache tracks in memory or use `SDL_rwops`)
