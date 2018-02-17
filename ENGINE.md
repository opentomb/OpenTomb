# Source Folders

- `extern`
    Contains all built-in libraries:
    - `al` - OpenAL 1.16 (or 1.17, needs confirmation) Changes: Uses custom SDL2 backend; AL built-in library works on Windows and MacOS. However, Linux is not supported thus it is important when building under Linux, native AL library are used instead.
    - `bullet` 2.83 - No changes.
    - `freetype2` - No changes.
	- `lua` 5.3 - No changes.
 
- `src`
    Contains engine code:
    - `core` - Low-level code:
         - `avl` - AVL tree container for internal usage (entity and items storage).
         - `base_types` - engine container and transform description, base container defines.
         - `vmath` - Base vector, Quaternion, Matrix 4x4 and spline mathematics.
         - `polygon` - Base polygon structure.
         - `obb` - Oriented bounding box module (OBB).
         - `utf8_32` - UT8 - 32 string manipulation functions.
         - `console` - Console implementation (allows reading UTF-8 strings inputted into the console window).
         - `gl_utils` - Contains OpenGL function pointers and base shader loading functions; module only makes use of `SDL_opengl` and `SDL_GL_GetProcAdress(...)`. It is important that ONLY `gl_ulils.h` as `gl` header is used. Also, only use qgl\* functions for interaction with the OpenGL API.
         - `gl_font` - Contains implementation of rendering for True Type Font lib within an OpenGL context. (works with UTF-8 strings).
         - `gl_text` - Module for fonts and styles storage.
         - `gl_console` - Console rendering and commands launching.
         - `system` - Contains basic debug print, error functions, check if file exists function and take screenshot function.
    - `notes`:
         - \* - For file I/O operations. It is important to use `SDL_rwops`. This is part of SDL, works everywhere and is suggested to maintain continuity.

    - `vt` - External trosettastone Tomb Raider resource loader project, rewritten and updated :-)

    - `render` - Contains the source for scene rendering.
         - `bordered_texture_atlas`, `bsp_tree_2d` - [Cochrane](https://github.com/Cochrane)'s module for storing many original textures in a single one.
         - `bsp_tree` - Module for transparent polygon sorting (BSP tree creation module, uses internal memory management).
         - `camera` - Structure with camera related fields, matrices + camera manipulation functions.
         - `frustum` - Special module for rooms and object visibility calculation. This is done via portal/frustum intersections (uses internal memory management).
         - `shader_description`, shader_manager - Module for shader object creation/application.
         - `render` - The main scene rendering module, works in two stages: 1: Generates room rendering list based on the camera. 2: render previously generated list, only used during debug rendering.

    - `script` - Contains LUA script functions.
         - `script` - Engine constants loading to LUA, parsers functions, system functions.
         - `script_audio` - Audio config file parsing, audio starting,  stopping and state retrieval.
         - `script_character` - Character parameter manipulation and inventory related functions.
         - `script_entity` - Physics, positioning, flags state manipulation.
         - `script_skeletal_model` - Animation control system.
         - `script_world` - Spawning objects, level transitions, level configuration functions, objects and effects generation.

    - `state_control` - Contains skeletal models state control handlers.
         - `tate_control` - Main interface to state control module (binding special functions to the models).
         - `tate_control_Lara` - Only Lara's state control control module.
         - `tate_control_*` - All other objects control functions.
    - `audio` - AL audio sources, soundtrack manipulation and a storage module.
         - `stb_vorbis.c` - Ogg vorbis format loader.
         - `audio` - Main audio routine handler and public interface.
         - `audio_fx` - Sound effects helper.
         - `audio_stream` - Audio stream handler and public interface.
		 
    - `fmv` - transformed ffmpeg library part for FMV video format support.

    - `physics` - Contains abstract engine interface for working with physics. It is important to use it only within the engine code! Here ray/sphere test functions, multi-mesh models for skeletal models.
         - `physics.h` - Public module interface.
         - `hair.h` - Public hair type interface.
         - `ragdoll.h` - Public ragdoll type interface.
         - `physics_bullet` - Private implementation, based on bullet library. Stores all engine physics geometry, creates own physics geometry from level resources.

    - `gui` - Contains GUI management code (exclude console)
         - `gui` - Renders all debug strings, bars, load screen.
         - `gui_inventory` - Renders the inventory menu.

    - `character_controller` - Manages controls for the character under different state conditions i.e whilst (on floor, free fall, underwater, on water or climbing etc.). Also, helper functions and weapon state control functions are included.
    - `controls` - Parses input from SDL and updates engine input control state structure.
    - `engine` - Contains main loop function, SDL event handlers, and debug output functions.
    - `entity` - Main in-game object type structure and manipulation functions. Contains frame update functions, callback callers, physics state updater/checker functions.
    - `game` - Contains main game frame function. Also contains save/load functions and low-level game effects (flyby camera, look at control, load screen updater).
    - `gameflow` - Contains the base module for interfacing with external LUA "gameflow" scripts. This module is responsible for tracking secrets and transitioning between different game states (i.e play FMV, play level etc) or setting special game/level specific parameters.
    - `image` - Layer module for reading pcx, png and saving png images. bpp = 24 or 32 only (RGB or RGBA only).
    - `inventory` - Contains the item structure and simple add/remove item from inventory functions.
    - `main_SDL` - Only main function and engine start.
    - `mesh` - Base item for rendering, contains vertices and VBO.
    - `resource` - Simple layer for converting level data from VT format to a format this engine supports. There is also a floor data to collision geometry converter included.
    - `room` - Contains room structure and object ownership manipulation (entity is within in room c and moved to room d).
    - `skeletal_model` - Contains base model, animation representation structures for in-game usage. A unique skeletal model structure is implemented with a smooth skeletal model update algorithm. Multi-animation system algorithm, multi-targeting bone mutators algorithm (head tracking, weapons targeting).
    - `trigger` - Here is the main (in-game) sector trigger handler/parser and object functions caller.
    - `world` - Main level database storage (excluding sound): models, entities, rooms, meshes etc. here are level loader/destructor and interface for accessing to rooms/entities by coordinates/ids;

