-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

function gen_trap_init(id)      -- Generic traps (TR1-TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, true);
    
    local state_on = 1;
    local state_off = 0;
    local object_mask = getEntityMask(id);
    if(object_mask == 0x1F) then
        setEntityAnimStateHeavy(id, ANIM_TYPE_BASE, state_on);
        setEntityTriggerLayout(id, 0x00, 0, 0); -- Reset activation mask and event.
        state_on = 0;
        state_off = 1;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_on);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then 
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off) 
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 1) then 
            changeCharacterParam(activator_id, PARAM_HEALTH, -35.0 * 60.0 * frame_time) 
        end;
    end
end


function sethblade_init(id)      -- Seth blades (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, true);

    local state_on = 1;
    local state_off = 2;
    local object_mask = getEntityMask(id);
    if(object_mask == 0x1F) then
        setEntityAnimStateHeavy(id, ANIM_TYPE_BASE, state_on);
        setEntityTriggerLayout(id, 0x00, 0, 0); -- Reset activation mask and event.
        state_on = 2;
        state_off = 1;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
        setEntityAnimState(object_id, ANIM_TYPE_BASE, state_on);
        state_on = state_off;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off)
            setEntityActivity(object_id, false);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityAnimState(object_id, ANIM_TYPE_BASE) == 1) then 
            setCharacterParam(activator_id, PARAM_HEALTH, 0) 
        end;
    end
end


function swingblade_init(id)        -- Swinging blades (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, true);
    
    local state_on = 2;
    local state_off = 0;
    local object_mask = getEntityMask(id);
    if(object_mask == 0x1F) then
        setEntityAnimStateHeavy(id, ANIM_TYPE_BASE, state_on);
        setEntityTriggerLayout(id, 0x00, 0, 0); -- Reset activation mask and event.
        state_on = 0;
        state_off = 2;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_on);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then 
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off) 
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -45.0 * 60 * frame_time);
    end
end


function wallblade_init(id)     -- Wall blade (TR1-TR3)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);

    state_on = 1;
    state_off = 0;

    local object_mask = getEntityMask(id);
    if(object_mask == 0x1F) then
        setEntityActivity(object_id, state_on);
        setEntityTriggerLayout(id, 0x00, 0, 0); -- Reset activation mask and event.
        state_on = 0;
        state_off = 1;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, state_on);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, state_off);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then 
            setEntityActivity(object_id, state_off) 
        end;
        local anim_number = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(anim_number == 2) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 3, 0);
        elseif(anim_number == 1) then
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
        end;
    end;
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -50.0 * 60.0 * frame_time);
    end;
end


function oldspike_init(id)  -- Teeth spikes

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, true);
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
            local ls = getEntitySpeedLinear(activator_id);
                
            if(lz > (pz + 256.0)) then
                local sx,sy,sz = getEntitySpeed(activator_id);
                if(sz < -256.0) then
                    setEntityCollision(object_id, 0);
                    setEntityAnim(activator_id, ANIM_TYPE_BASE, 149, 0);
                    setEntityPos(activator_id, lx, ly, pz);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                end;
            elseif(ls > 512.0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -(ls / 512.0));
            end;
        end;
    end
end


function newspike_init(id)  -- Teeth spikes (TR4-5)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, false);
    
    setEntityVisibility(id, 0);
    setEntityCollision(id, 0);
    
    entity_funcs[id].interval        = 150;     -- 150 frames = 2.5 seconds
    entity_funcs[id].curr_timer      = entity_funcs[id].interval;   -- This activates spikes on first call.
    
    entity_funcs[id].curr_scaling    = 0.0;     -- Scaling is done via linear function.
    entity_funcs[id].curr_subscaling = 0.0;     -- Subscaling is done via trigonometric function.
    
    entity_funcs[id].mode            = 0;       -- Movement mode.
    entity_funcs[id].waiting         = false;   -- Non-active state flag.
    
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 1) then
            return ENTITY_TRIGGERING_ACTIVATED;
        end;

        -- This is case for spikes which were already activated before, but were set to idle state
        -- after activation - that is, it means if we're activating it again, they should be
        -- disabled. On the next activation event, however, they will be activated again.
    
        if(entity_funcs[object_id].waiting == true) then
            entity_funcs[object_id].waiting = false;
            return entity_funcs[object_id].onDeactivate(object_id, activator_id);
        end;
    
        -- For teeth spikes, OCB value stores vertical rotation in first 3 bits (0x07), horizontal
        -- rotation in fourth bit (0x08) and working mode in next two bits (0x30).

        local curr_OCB  = getEntityOCB(object_id);
        local rot_value = bit32.band(curr_OCB, 0x07);
        
        -- Spikes are vertically rotated in 45 degree step, and zero rotational value actually means
        -- that spikes are pointed downwards (e. g. 180 degrees rotation).
        
        rotateEntity(object_id, 0.0, 0.0, (180.0 + rot_value * 45.0));
        if(bit32.band(curr_OCB, 0x08) ~= 0) then 
            rotateEntity(object_id, 270.0) 
        end;   -- Rotate horizontally.
        
        -- Fix teeth spikes position for right angles (except upwards case).
        
        if(rot_value == 0) then
            moveEntityLocal(object_id, 0.0, 0.0, -1024.0);
        elseif(rot_value == 2) then
            moveEntityLocal(object_id, 512.0, 0.0, -512.0);
        elseif(rot_value == 6) then
            moveEntityLocal(object_id, -512.0, 0.0, -512.0);
        end;
        
        -- Mode 0 is normal teeth spike movement (continous pop-retract), mode 1 is sticked out
        -- constantly mode, and mode 2 is single pop-retract (as seen in Coastal Ruins shooting range).
        
        entity_funcs[object_id].mode = bit32.rshift(bit32.band(curr_OCB, 0x30), 4);
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 0) then
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;
        -- This non-obvious code is needed to make teeth spike retract with animation.
        -- What it basically does is overrides spike mode with single pop-retract, and also
        -- overrides current activity state, which allows spikes to successfully retract and
        -- disable themselves. Teeth spike mode will be back to normal on the next re-activation,
        -- cause OCB parsing happens right in onActivate event.
        
        setEntityActivity(object_id, true);
        entity_funcs[object_id].mode = 2;
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
    
        -- If spike timer is less than 60 (meaning the first second of cycle), we should initiate
        -- pop-retract phase. Pop phase is done in first 10 frames, and retract phase is done after
        -- frame 50 (e.g. 60-10 = 10 frames as well). For mode 1, we stop the phase right after
        -- spikes were popped out, and send them to idle state - so they can be deactivated later.
        local time_coef = frame_time * 60.0;
        if(entity_funcs[object_id].curr_timer < 60) then
            entity_funcs[object_id].curr_subscaling = entity_funcs[object_id].curr_subscaling + 6 * time_coef;  -- Subscaling makes full turn in 1 sec.
            if(entity_funcs[object_id].curr_timer <= 10) then
                entity_funcs[object_id].curr_scaling = entity_funcs[object_id].curr_scaling + 0.1 * time_coef;
            elseif(entity_funcs[object_id].curr_timer > 50) then
                if(entity_funcs[object_id].mode == 1) then
                    entity_funcs[object_id].waiting = true;
                    setEntityActivity(object_id, false);
                    return;
                else
                    entity_funcs[object_id].curr_scaling = entity_funcs[object_id].curr_scaling - 0.1 * time_coef;
                end;
            end;
            
        -- If spike timer is large than 60 (rest of the cycle), we force spikes to hide
        -- completely and send them to idle state. For mode 2, we also stop whole cycle, so
        -- spikes will remain in the dormant state until next activation.
            
        elseif(entity_funcs[object_id].curr_timer < entity_funcs[object_id].interval) then
            if(entity_funcs[object_id].waiting == false) then
                entity_funcs[object_id].curr_subscaling = 0;
                entity_funcs[object_id].curr_scaling = 0.0;
                setEntityVisibility(object_id, 0);
                setEntityScaling(object_id, 1.0, 1.0, 0.0);
                setEntityCollision(object_id, 0);
                entity_funcs[object_id].waiting = true;
                if(entity_funcs[object_id].mode == 2) then
                    setEntityActivity(object_id, false);
                    return;
                end;
            end;
            
        -- When cycle has ended, we reset idle state and timer, and force spikes to be visible
        -- and material, and also play spike sound.
            
        else
            setEntityVisibility(object_id, 1);
            setEntityCollision(object_id, 1);
            entity_funcs[object_id].curr_timer = 0;
            playSound(343, object_id);
            entity_funcs[object_id].waiting = false;
        end;
        
        -- We update entity scaling (material and visible) only if entity isn't in idle state.
        -- This way, we conserve system resources in idle cycle phase.
        
        if(entity_funcs[object_id].waiting == false) then
            setEntityScaling(object_id, 1.0, 1.0, entity_funcs[object_id].curr_scaling - math.abs(math.cos(math.rad(entity_funcs[object_id].curr_subscaling))) / 4);
        end;
        
        entity_funcs[object_id].curr_timer = entity_funcs[object_id].curr_timer + 1.0 * time_coef;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
        
            -- If Lara has collided with spikes in early phase of spike cycle (i.e. popping out),
            -- we immediately kill her and play impale animation. Also, we force spike mode to 1,
            -- so that particular spikes won't retract back, or else it'll look unrealistic with
            -- Lara being "impaled" into thin air.
            
            if(entity_funcs[object_id].curr_timer <= 10) then
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 149, 0);
                setCharacterParam(activator_id, PARAM_HEALTH, 0);
                entity_funcs[object_id].mode = 1;
                return;
            end;
            
            -- Normal spikes behaviour - only difference is we're not playing impale animation if
            -- Lara is approaching from the top, but create ragdoll instead - it is needed to
            -- prevent aforementioned problem, but in this case Lara could be impaled in the
            -- retraction phase, which will look unrealistic as well.
            
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
            local ls = getEntitySpeedLinear(activator_id);
                
            if(lz > (pz + 256.0)) then
                local sx,sy,sz = getEntitySpeed(activator_id);
                if(sz < -256.0) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                    playSound(SOUND_IMPALE, activator_id);
                end;
            elseif(ls > 512.0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -(ls * frame_time * 60 / 512.0));
            else
                changeCharacterParam(activator_id, PARAM_HEALTH, -4 * 60 * frame_time);
            end;
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].interval        = nil;
        entity_funcs[object_id].curr_timer      = nil;
        entity_funcs[object_id].curr_scaling    = nil;
        entity_funcs[object_id].curr_subscaling = nil;
        entity_funcs[object_id].waiting         = nil;
        entity_funcs[object_id].mode            = nil;
    end
    
    prepareEntity(id);
end


function spikewall_init(id)      -- Spike wall

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityCollisionFlags(id, bit32.bor(COLLISION_GROUP_TRIGGERS, COLLISION_GROUP_CHARACTERS), nil, bit32.bor(COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_GHOST));
    setEntityActivity(id, false);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, false);
        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local ver = getLevelVersion();
        local scan_distance = 32.0;
        if(ver < TR_II) then scan_distance = 1536.0 end; -- TR1's lava mass has different floor scan distance.
        
        if(similarSector(object_id, 0.0, scan_distance, 0.0, false)) then
            moveEntityLocal(object_id, 0.0, 8.0 * 60.0 * frame_time, 0.0);
            playSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        else
            setEntityActivity(object_id, false);    -- Stop
            stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityModelID(activator_id) == 0) then
            local curr_st = getEntityMoveType(activator_id);
            if((curr_st == MOVE_CLIMBING) or (curr_st == MOVE_MONKEYSWING)) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 28, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            elseif(curr_st == MOVE_WALLS_CLIMB) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 30, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            end;
            
            if(getCharacterParam(activator_id, PARAM_HEALTH) > 0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -20 * 60 * frame_time);
                playSound(getGlobalSound(getLevelVersion(), GLOBALID_SPIKEHIT), activator_id);
                if(getCharacterParam(activator_id, PARAM_HEALTH) <= 0) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    playSound(SOUND_GEN_DEATH, activator_id);
                    setEntityActivity(object_id, false);
                    stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
                end;
            end;
        end;
    end
end


function spikeceiling_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityCollisionFlags(id, COLLISION_GROUP_TRIGGERS, nil, bit32.bor(COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_GHOST));
    setEntityActivity(id, false);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, false);
        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local px, py, pz = getEntityPos(object_id);

        if(pz > (getSectorHeight(object_id) + 512.0)) then
            moveEntityLocal(object_id, 0.0, 0.0, -4.0 * 60.0 * frame_time);
            playSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        else
            setEntityActivity(object_id, false);    -- Stop
            stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityModelID(activator_id) == 0) then
            local curr_st = getEntityMoveType(activator_id);
            if((curr_st == MOVE_CLIMBING) or (curr_st == MOVE_MONKEYSWING)) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 28, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            elseif(curr_st == MOVE_WALLS_CLIMB) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 30, 0);
                playSound(SOUND_LARAINJURY, activator_id);
            end;
        
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
                
            if(getCharacterParam(activator_id, PARAM_HEALTH) > 0) then
                if(lz > (pz - 256.0)) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                    playSound(SOUND_IMPALE, activator_id);
                else
                    changeCharacterParam(activator_id, PARAM_HEALTH, -20 * 60 * frame_time);
                    playSound(getGlobalSound(getLevelVersion(), GLOBALID_SPIKEHIT), activator_id);
                    if(getCharacterParam(activator_id, PARAM_HEALTH) <= 0) then
                        addEntityRagdoll(activator_id, RD_TYPE_LARA);
                        playSound(SOUND_GEN_DEATH, activator_id);
                        setEntityActivity(object_id, false);
                        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
                    end;
                end;
            end;
        end;
    end
end


function lasersweep_init(id)      -- Laser sweeper (TR3)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, false);
    
    entity_funcs[id].speed         = 1.0 * 60.0;
    entity_funcs[id].phase_length  = math.abs(2048.0 * entity_funcs[id].speed / 60.0);
    entity_funcs[id].current_phase = 0.0;
    
    entity_funcs[id].stopping      = false;  -- Sweeper will be stopped on the next phase end.
    entity_funcs[id].current_wait  = 0.0;    -- Wait timer.
    entity_funcs[id].max_wait      = 1.5;    -- Sweeper will wait for 1.5 seconds at both ends.
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityActivity(id, true);
            return ENTITY_TRIGGERING_ACTIVATED;
        else
            entity_funcs[object_id].stopping = true;
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityActivity(object_id, false);
            return;
        end;
        
        if(entity_funcs[object_id].current_wait == 0.0) then
            local next_phase = entity_funcs[object_id].current_phase + entity_funcs[object_id].speed * frame_time;
            
            if(((next_phase >= 90.0) and (entity_funcs[object_id].current_phase < 90.0)) or
               ((next_phase >= 270.0) and (entity_funcs[object_id].current_phase < 270.0))) then
                entity_funcs[object_id].current_wait = frame_time;
            end;
            
            entity_funcs[object_id].current_phase = next_phase;
            
            if(entity_funcs[object_id].current_phase > 360.0) then
                entity_funcs[object_id].current_phase = entity_funcs[object_id].current_phase - 360.0;
            end;
            
            moveEntityLocal(object_id, 0.0, math.cos(math.rad(entity_funcs[object_id].current_phase)) * entity_funcs[object_id].phase_length, 0.0);
        else
            if(entity_funcs[object_id].stopping == true) then
                setEntityActivity(object_id, false);
                entity_funcs[object_id].stopping = false;
            else
                entity_funcs[object_id].current_wait = entity_funcs[object_id].current_wait + frame_time;
                
                if(entity_funcs[object_id].current_wait > entity_funcs[object_id].max_wait) then
                    entity_funcs[object_id].current_wait = 0.0;
                end;
            end;
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then changeCharacterParam(activator_id, PARAM_HEALTH, -15.0) end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].current_phase = nil;
        entity_funcs[object_id].phase_length  = nil;
        entity_funcs[object_id].speed         = nil;
        entity_funcs[object_id].stopping      = nil;
        entity_funcs[object_id].current_wait  = nil;
        entity_funcs[object_id].max_wait      = nil;
    end
    
    prepareEntity(id);
end






