OpenTomb ? TRIGGERS and SCRIPTING STORY
---------------------------------------

### Table of contents ###

1. The main requirements
2. Main tasks

1. The main requirements
------------------------
	* LUA game state may be saved in every time and can be fully restored after loading; LUA game state - all LUA information about objects, tasks and game world;
	* LUA must have all necessary functions to set all saveable game parameters to objects, tasks, environment (game world);
	* The LUA functions must to require "single responsibility principle" (without fanaticism); no hacky tricks using;
	* Main trigger handler works in `trigger.cpp`;
	* Activate / DeActivate entity functions must return state like: activated, deactivated, already_done, object_busy;

2. Main tasks
-------------
	* main trigger handler (correct floor data trigger list parsing);
	* adding functiions handlers for using in main trigger handler;
	* adding special scripting objects handlers;