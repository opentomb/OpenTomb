-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, CUT2

print("Level script loaded (CUT2.lua)");

level_PostLoad = function()
    playStream(25);
    local id = 0;
    while(true) do
        local x, y, z, az = getEntityPos(id);
        if(x == nil) then
            break;
        end;
        setEntityPos(id, 51712, 53760, -3840, -90, 0, 0);
        id = id + 1;
    end;

    entity_funcs[0] = {};
    entity_funcs[0].t = 0;
    entity_funcs[0].onLoop = function(object_id, tick_state)
        entity_funcs[0].t = entity_funcs[0].t + frame_time;
        if(not setCameraFrame(entity_funcs[0].t * 30)) then
            setGame(GAME_1, 10);
        end;
    end;
end;

level_PreLoad = function()
    setCinematicTransform(51712, 53760, -3840, -90);
end;