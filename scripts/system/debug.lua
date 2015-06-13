-- OPENTOMB DEBUG SCRIPT
-- by Lwmte, June 2015

--------------------------------------------------------------------------------
-- Here we'll place any debug routines probably needed for testing in game.
--------------------------------------------------------------------------------

function checkDebugKeys()
    if(checkKey(KEY_R, true)) then
        print("Ragdoll activated!");
        addEntityRagdoll(player, RD_TYPE_LARA);
    end;
    
    if(checkKey(KEY_T, true)) then
        print("Ragdoll deactivated!");
        removeEntityRagdoll(player);
    end;
    
    if(checkKey(KEY_H, true)) then
        setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        playSound(SOUND_MEDIPACK, player);
    end;
    
    if(checkKey(KEY_Z, true)) then
        if(getEntityMoveType(player) == MOVE_UNDERWATER) then
            setEntityAnim(player, 103);
            setEntityMoveType(player, MOVE_ON_FLOOR);
            setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        else
            setEntityAnim(player, 108);
            setEntityMoveType(player, MOVE_UNDERWATER);
            setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        end;
        
        setEntityAnimFlag(player, ANIM_NORMAL_CONTROL);
    end;
    
    return true;
end;

function checkPlayerRagdollConditions()
    local anim, frame, count = getEntityAnim(player);
    
    if( ((anim ==  25) and (frame >= 6))   or
        ((anim == 155) and (frame >= 5)) ) then
        addEntityRagdoll(player, RD_TYPE_LARA);
    end;
    
    return true;
end;