-----------------------------------
--           Misc Code           --
-----------------------------------
local DEBUG_MODE = 1;
local ver = getLevelVersion();

setModelCollisionMap(0, 0, 0);              -- butt
setModelCollisionMap(0, 1, 7);              -- body
setModelCollisionMap(0, 2, 14);             -- head

setModelCollisionMap(0, 3, 1);              -- legs 1
setModelCollisionMap(0, 4, 4);
setModelCollisionMap(0, 5, 11);             -- hands 1
setModelCollisionMap(0, 6, 8);

setModelCollisionMap(0, 7, 2);              -- legs 2
setModelCollisionMap(0, 8, 5);
setModelCollisionMap(0, 9, 12);             -- hands 2
setModelCollisionMap(0, 10, 9);

setModelCollisionMap(0, 11, 3);             -- boots 3
setModelCollisionMap(0, 12, 6);
setModelCollisionMap(0, 13, 10);            -- braces 3
setModelCollisionMap(0, 14, 13);

-- Generate UV rotation texture animations for waterfalls in TR4+ versions
if (ver == TR_IV) then
    for i=423, 426, 1 do
        genUVRotateAnimation(i);
    end;
elseif (ver == TR_V) then
    for i=410, 415, 1 do
        genUVRotateAnimation(i);
    end;
end;

-- Add global level tasks
if (DEBUG_MODE == 1) then
    addTask(checkDebugKeys);
end;