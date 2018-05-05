-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, CUT3

print("Level script loaded (CUT3.lua)");

level_PostLoad = function()
    setGlobalFlipState(1);
    playStream(24);
    entity_funcs[1] = {};
    entity_funcs[1].t = 0;
    entity_funcs[1].onLoop = function(object_id, tick_state)
        if((entity_funcs[1].t < 5) and (entity_funcs[1].t + frame_time >= 5)) then
            setGlobalFlipState(0);
        end;
        entity_funcs[1].t = entity_funcs[1].t + frame_time;
        if(not setCameraFrame(entity_funcs[1].t * 30) or (0 ~= getActionState(ACT_LOOK))) then
            gameflowSend(GF_OP_LEVELCOMPLETE, getLevel() + 1);
            gameflowSend(GF_OP_STARTFMV, 8);
            entity_funcs[1].onLoop = nil;
        end;
    end;
end;

level_PreLoad = function()
    setCinematicTransform(49664, 51712, 18688, -90);
end;