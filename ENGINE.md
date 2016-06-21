1. SOURCE DIRECTORIES
--------------
-	extern
	Contains build in libraries:
	-	al - OpenAL 1.16 or 1.17... do not remember exactly... modified: used custom SDL2 backend;
	-	bullet 2.83 - no changes;
	-	freetype2 - no changes
	-	ogg - no changes

-	src
	Contains engine code:
	-	core - low level code:
		-	vmath - base vector and quaternion mathematics, matrices 4x4, splines; 
		-	polygon - base polygon structure; 
		-	obb - oriented bounding box module; 
		-	utf8_32 - ut8 - 32 string manipulation functions; 
		-	console - console implementation (allows utf-8 string inputing); 
		-	gl_utils - module uses only SDL_opengl and SDL_GL_GetProcAdress(...), so use ONLY gl_ulils.h as gl header and only qgl* functions;
		-	gl_font - here implements true type font rendering in OpenGL context;
		-	redblack - red black tree for build-in data storage;
		-	system - basic debug print and error functions, file found function and screenshot making function;
		
		*for files manipulation use SDL_rwops module: it works everywhere SDL works...
	
	-	vt - external trosettastone Tomb Raider resource loader project, rewritten and updated :-)
	
	-	render - here is sources for scene rendering;
		-	bordered_texture_atlas, bsp_tree_2d - Cochrane's module for storing many original textures in single one;
		-	bsp_tree - module for transparent polygons sorting (bsp tree creation module, uses internal mem managment);
		-	camera - structure with camera parameters, matrices + camera manipulation functions;
		-	frustum - special module for rooms and object visibility calculation by portal / frustum intersections (uses internal mem managment);
		-	shader_description, shader_manager - module for shaders manipulations;
		-	render - main scene rendering module, working in two steps: 1: generates rendering list by camera, 2: render previously generated list; here implemented debug rendering;
		
	-	anim_state_control - only Lara's state control controller module;
	-	audio - AL audio sources and soundtrack manipulation and storage module;
	-	character_controller - controls moving in different conditions (on floor, free fall, under water, on water, climbing a.t.c...); + contains helpers functions and weapon state control functions;
	-	controls - parses input and updates engine control state structure;
	-	engine - contains main loop function, SDL event handlers, and debug output functions;
	-	entity - main in game object type structure and manipulation functions; contains frame updates functions, callback callers, physics state updaters / checkers functions;
	-	game - contains main game frame function; also contains save / load functions, low-level game effects (flyby camera, look at control, load screen updater...);
	-	gameflow - contains levels loading order control functions and allows to get load screen info;
	-	gui - renders all debug strings, bars, load screen, inventory menu;
	-	inventory - only item structure and simplest add / remove item functions;
	-	main_SDL - only main function and engine start (+ todo list in comment);
	-	mesh - base item for rendering, contains vertices and VBO;
	-	physics - contains abstract engine interface for working with physics - use only it in engine code! here ray / sphere test functions, multimesh models for skeletal models;
	-	physics_bullet - stores all engine physics geometry; contains all physics code implementation with bullet library; creates own physics geometry from level resources;
	-	resource - simple layer for conversion level data from VT format to engine format;
	-	room - contains room structure and objects ownership manipulation (entity a contains in room c and moved to room d);
	-	script - contains LUA script functions, engine constants loading to LUA;
	-	skeletal_model - contains base model animation representation structures, and in game usage unique skeletel model structure; implemented smoothed skeletal model update algorithm, multi animation system algoritm and multi targeting bone mutators algorithm (head tracking, weapons targeting);
	-	trigger - here is main (in game) sector trigger handler / parser;
	-	world - main level data base storage (exclude sound): models, entities, rooms, meshes a.t.c. here are level loader / destructor and interface for accessing to rooms / entities by coordinates / id's;
	
