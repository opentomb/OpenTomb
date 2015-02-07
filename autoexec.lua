-- LUA autoexec config file
-- CVAR's section. here you can create and delete CVAR's

if(cvars == nil) then
    cvars = {};
end;

-- CVAR's section. here you can create and delete CVAR's
cvars.show_fps = 1;
cvars.free_look_speed = 2500;

-- AUTOEXEC LINES
setLanguage("english");
setgamef(1, 1);

setGravity(0, 0, -5700.0);
mlook(1);
freelook(0);
cam_distance(1024.0);
noclip(0);
