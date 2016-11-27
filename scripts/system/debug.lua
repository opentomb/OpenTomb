-- OPENTOMB DEBUG SCRIPT
-- by Lwmte, June 2015

--------------------------------------------------------------------------------
-- Here we'll place any debug routines probably needed for testing in game.
--------------------------------------------------------------------------------

debug_game = GAME_1;
debug_level = 1;

function goToEntity(id)
    if(player ~= nil) then
        setEntityPos(player, getEntityPos(id));
    end;
end;

function checkDebugKeys()
    if(checkKey(KEY_RETURN, true)) then
        setGame(debug_game, debug_level);
        debug_level = debug_level + 1;
        if(debug_game == GAME_1) then 
            if(debug_level == 16) then
                debug_game = GAME_1_5;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_1_5) then
            if(debug_level == 5) then
                debug_game = GAME_2;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_2) then
            if(debug_level == 19) then
                debug_game = GAME_2_5;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_2_5) then
            if(debug_level == 6) then
                debug_game = GAME_3;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_3) then
            if(debug_level == 21) then
                debug_game = GAME_3_5;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_3_5) then
            if(debug_level == 7) then
                debug_game = GAME_4;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_4) then
            if(debug_level == 39) then
                debug_game = GAME_5;
                debug_level = 1;
            end;
        elseif(debug_game == GAME_5) then
            if(debug_level == 15) then
                debug_game = GAME_1;
                debug_level = 1;
            end;
        end;
    end;

    if(checkKey(KEY_R, true)) then
        print("Ragdoll activated!");
        addEntityRagdoll(player, RD_TYPE_LARA);
    end;
    
    if(checkKey(KEY_T, true)) then
        print("Ragdoll deactivated!");
        removeEntityRagdoll(player);
    end;
       
    if(checkKey(KEY_Y, true)) then
        debuginfo();
    end;
    
    if(checkKey(KEY_H, true)) then
        setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        playSound(SOUND_MEDIPACK, player);
    end;
    
    if(checkKey(KEY_Z, true)) then
        if(getEntityMoveType(player) == MOVE_UNDERWATER) then
            setEntityAnim(player, ANIM_TYPE_BASE, 103, 0);
        else
            setEntityAnim(player, ANIM_TYPE_BASE, 108, 0);
        end;
        
        setEntityCollision(player, 1);
        removeEntityRagdoll(player);
        setEntityMoveType(player, MOVE_FREE_FALLING);
        setCharacterResponse(player, RESP_KILL, 0);
        setCharacterParam(player, PARAM_HEALTH, PARAM_ABSOLUTE_MAX);
        setEntityAnimFlag(player, ANIM_TYPE_BASE, ANIM_NORMAL_CONTROL);
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
    local anim, frame, count = getEntityAnim(player, ANIM_TYPE_BASE);
    local version = getLevelVersion();
    
    if(getEntityTypeFlag(player, ENTITY_TYPE_DYNAMIC) == 0) then
        if( ((anim ==  25) and (frame >= 6 )) or
            ((anim == 155) and (frame >= 5 )) or
            ((anim == 139) and (frame >= 17)) or
            ((anim == 133) and (frame >= 18)) or
            ((anim == 145) and (frame >= 67)) or
            ((anim == 301) and (frame >= 57)) or
            ((anim == 138) and (((frame >= 60) and (version >= TR_II)) or ((frame >= 8) and (version < TR_II)))) ) then
                addEntityRagdoll(player, RD_TYPE_LARA);
        end;
    end;
    
    return true;
end;