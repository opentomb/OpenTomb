-- LUA autoexec config file
-- CVAR's section. here you can create and delete CVAR's

if(cvars == nil) then
    cvars = {};
end;

-- CVAR's section. here you can create and delete CVAR's
cvars.show_fps = true;
cvars.free_look_speed = 2500;

-- AUTOEXEC LINES
setLanguage("english");

setGravity(0, 0, -5856.0);
mlook(true);
freelook(false);
cam_distance(1024.0);
noclip(false);
--loadMap("data/newlevel.tr4");
setgamef(1, 1);
