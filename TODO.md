* - high priority bug / task;

-	The main plan:
	  first we need to implement TR1 gameplay, so TR1 functions tasks are more priority than TR4 / TR5 functions; let us made simple, but stable and working version and next step by step extends functionality;

-	Build configuration:
		1. *add lua and SDLImage to extern folder (TeslaRus task);
		2. *add build extern libs script ( -> *.a), update c::b project (TeslaRus task);
		3. make good autobuild system;
	  
-	Git working:
		commits to master may be:
		1. merging with feature branch (branch that contains new feature, or big bug fixing); feature branch must be avaliable in github; after merging it must be deleted;
		2. little and clean bug fixes;
		3. documentation updating;
		4. build scripting updates (may be used feature branch, by autor's opinion);
		5. independed resources updating;
		
		creating feature branches and commits to them are free;
		extern folder not for editing - it is place for external libraries sources / headers;
		How to do:
		0. install GIT bash; next use git bash terminal;
		1. in project folder with GIT repo make feature branch: "git checkout -b feature_branch_name"; (or make repo by github web ui);
		2. make feature; commit change "git add -u", "git commit"; if you use vim, press i, enter message, escape button, :wq enter;
		3. merge change "git merge master"; resolve conflicts (try to rebuild project); commit changes;
		4. push feature branch to feature branch on github;
		5. do merge request;
		
		src/core folder: only by TeslaRus, make request if you want to change something;
		other folders: by merge requests to TeslaRus or after code review (by merge request) by command (more details will be discussed); I will make some commits after merge request too (number of errors will be decreased significantly in review case);
		
-	Code in main
		todo:
		1. game.cpp - many different logic in one place, need to be refactored;
		
-	Collision system:
		current situation: 
			1. fixed back / front faced polygons orientation for physics geometry, so there is working Filtered Ray Test (skips back faced polygons) in engine;
			2. collision margin == 0, because in other case near edges normales become smooth and Lara slides down or stops in places she should not;
		 
		todo:
			1. *fix shimmy left / right jamming;
			2. *fix mowing while landing on sloping surface (1. find body parts that stops Lara; 2. tune collision form, or disable collision checking for them;); bind with 3;
			3. *make ghost body parts meshes tunable by config (no more hardcoded boxes);
			4. for the future optimazation add switchable single ghost object for character;
			5. add long ray test (parces rooms portals and builds room list for collisional checking) needs for long range shooting and AI;
			6. make refactoring of Physics_GetCurrentCollisions(...) (mem managment);
			7. fix mowing in some floor slant cases in Character_FixPosByFloorInfoUnderLegs(...);
			8. *fix jammed (or slows and stopped) rolling stones, where it is critical (optional roll by path(not implemented), or roll by physics (implemented));
			9. check room tween butterfly normales;
			
-	Character controller:
		todo:
			1. *wepon control system needed to be refactored / fixed; (2 handed weapon model switches in wrong frame);
			2. *add auto weapon hiding in water environment a.t.c. (simple task)
			3. fix case of croaching weapons usage;
			4. *implements base state control for TR1 enemies (bat, wolf, bear, first, others - later); - it is needed for simple AI testing / developing;
			
-	Animation control:
		todo:
			1. fix state change missing in low fps case;
			2. skeletal model ss_anim control: make functional interface to control it, instead direct complex access to flags and structures;
			3. *update documentation about ss_animation structure and functions;
			4. *fix incorrect smoothing if there are move or rotate anim commands;
			
-	Camera control:
		todo:
			1. use targeting to correct body part or OBB center;
			2. implement camera flags usage (i.e. flyby);
			3. add special "camera_entity", store it in world module, access by "engine_camera_p World_GetCameraEntity();"; needs for heavy triggers;
			
-	Scripting:
		current situation:
			1. scripts updates EVERY game frame! so use engine frame time global inside time depended scripts!
		todo:
			1. *add function like lua_SaveTable(...) - that recursively print to file / buffer / clay tablets lua code with table content (i.e. table_name = { red = 1; green = 0; blue = 0; name = "name"; is_u = true; in_tbl = { p1 = "inner"; val = 32.45 }};
			2. *in all scripts that may change game state, data must be stored in special global table (that will be saved in save game); it is necessary for correct save / load game working;
			3. *activate_Entity script function must returns state (no duplication of activation e.t.c. + better state control);