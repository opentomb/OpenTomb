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

setGravity(0, 0, -5700.0);
mlook(1);
freelook(0);
cam_distance(1024.0);
noclip(0);
--loadMap(base_path .. "data/tr1/data/CUT1.PHD");
loadMap(base_path .. "tests/altroom3/LEVEL1.PHD");
--loadMap(base_path .. "tests/heavy1/LEVEL1.PHD");
--loadMap(base_path .. "tests/TRIGGERS.PHD");
--setgamef(1, 14);
--dofile(base_path .. "save/qsave.lua");
