-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, CUT1

print("Level script loaded (CUT1.lua)");

level_PostLoad = function()
    playStream(23);
    entity_funcs[0] = {};
    entity_funcs[0].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        local x, y, z = getEntityPos(2);
        if(f == 1) then
            setCameraPos(36668, 63180, z + 1024);
            setCameraAngles(90.0, -0.4, 0);
            freelook(1);
        end;
        if(f + 1 >= c) then
            freelook(0);
            setGame(GAME_1, 5);
        end;
    end;
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;