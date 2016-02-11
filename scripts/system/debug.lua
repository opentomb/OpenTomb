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
    
    if(checkKey(KEY_N, true)) then
        noclip();
    end;
    
    if(checkKey(KEY_G, true)) then
        timescale();
    end;
    
    if(checkKey(KEY_Y, true)) then
        debuginfo();
    end;
    
    if(checkKey(KEY_H, true)) then
        setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        playSound(SoundId.Medipack, player);
    end;
    
    if(checkKey(KEY_Z, true)) then
        if(getEntityMoveType(player) == MOVE_UNDERWATER) then
            setEntityAnim(player, 103);
        else
            setEntityAnim(player, 108);
        end;
        
        setEntityCollision(player, true);
        removeEntityRagdoll(player);
        setEntityMoveType(player, MOVE_FREE_FALLING);
        setEntityResponse(player, RESP_KILL, 0);
        setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        setEntityAnimFlag(player, AnimMode.NormalControl);
    end;
    
    if(checkKey(KEY_1, true)) then setCharacterCurrentWeapon(player, 1) end;
    if(checkKey(KEY_2, true)) then setCharacterCurrentWeapon(player, 2) end;
    if(checkKey(KEY_3, true)) then setCharacterCurrentWeapon(player, 3) end;
    if(checkKey(KEY_4, true)) then setCharacterCurrentWeapon(player, 4) end;
    if(checkKey(KEY_5, true)) then setCharacterCurrentWeapon(player, 5) end;
    if(checkKey(KEY_6, true)) then setCharacterCurrentWeapon(player, 6) end;
    if(checkKey(KEY_7, true)) then setCharacterCurrentWeapon(player, 7) end;
    if(checkKey(KEY_8, true)) then setCharacterCurrentWeapon(player, 8) end;
    if(checkKey(KEY_9, true)) then setCharacterCurrentWeapon(player, 9) end;
    
    return true;
end;

function checkPlayerRagdollConditions()
    local anim, frame, count = getEntityAnim(player);
    local version = getEngineVersion();
    
    if(getEntityTypeFlag(player, ENTITY_TYPE_DYNAMIC) == 0) then
        if( ((anim ==  25) and (frame >= 6 )) or
            ((anim == 155) and (frame >= 5 )) or
            ((anim == 139) and (frame >= 17)) or
            ((anim == 133) and (frame >= 18)) or
            ((anim == 145) and (frame >= 67)) or
            ((anim == 301) and (frame >= 57)) or
            ((anim == 138) and (((frame >= 60) and (version >= Engine.II)) or ((frame >= 8) and (version < Engine.II)))) ) then
                addEntityRagdoll(player, RD_TYPE_LARA);
        end;
    end;
    
    return true;
end;
