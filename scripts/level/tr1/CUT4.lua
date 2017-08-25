-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, CUT4

print("Level script loaded (CUT4.lua)");

level_PostLoad = function()
    playStream(22);
    entity_funcs[0] = {};
    entity_funcs[0].t = 0;
    entity_funcs[0].onLoop = function(object_id, tick_state)
        entity_funcs[0].t = entity_funcs[0].t + frame_time;
        if(not setCameraFrame(entity_funcs[0].t * 30)) then
            setGame(GAME_1, 15);
        end;
    end;
end;

level_PreLoad = function()
    setCinematicTransform(0, 0, 0, 128.056);
end;