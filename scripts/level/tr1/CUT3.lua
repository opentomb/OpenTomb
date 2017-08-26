-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, CUT3

print("Level script loaded (CUT3.lua)");

level_PostLoad = function()
    playStream(24);
    entity_funcs[1] = {};
    entity_funcs[1].t = 0;
    print(getEntityPos(1));
    entity_funcs[1].onLoop = function(object_id, tick_state)
        entity_funcs[1].t = entity_funcs[1].t + frame_time;
        if(not setCameraFrame(entity_funcs[1].t * 30)) then
            setGame(GAME_1, 14);
        end;
    end;
end;

level_PreLoad = function()
    setCinematicTransform(49664, 51712, 18688, -90);
end;