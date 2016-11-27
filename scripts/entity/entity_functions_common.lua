-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

-- Load up some extra entity scripts.

dofile(base_path .. "scripts/entity/script_switch.lua");     -- Additional switch scripts.


function door_init(id)   -- NORMAL doors only!

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
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
        if(getEntityEvent(object_id) == 0) then
            setEntityTimer(object_id, 0.0);
        end;

        if((tickEntity(object_id) == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
            setEntityEvent(object_id, 0);
        end;
    end
end


function pushdoor_init(id)   -- Pushdoors (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, false);

    setEntityActivationDirection(id, 0.0, -1.0, 0.0, 0.70);
    setEntityActivationOffset(id, 0.0, -400.0, 0.0, 128.0);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end;

        if((not getEntityActivity(object_id))) then
            setEntityActivity(object_id, true);
            local x, y, z, az, ax, ay = getEntityPos(object_id);
            setEntityPos(activator_id, x, y, z, az + 180.0, ax, ay);
            moveEntityLocal(activator_id, 0.0, 256.0, 0.0);
            -- floor door 317 anim
            -- vertical door 412 anim
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 412, 0);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
end


function set_activation_offset(id)
    local model_id = getEntityModelID(id);
    local dx = 0.0;
    local dy = 360.0;
    local dz = 0.0;
    local  r = 128.0;

    if(getLevelVersion() < TR_II) then
        if(model_id == 56) then
            dy = 0.0;
            r = 160.0;
        end
    elseif(getLevelVersion() < TR_III) then
        if(model_id == 105) then
            dy = 0.0;
            r = 160.0;
        end
    elseif(getLevelVersion() < TR_IV) then
        if(model_id == 130) then
            dy = 0.0;
            r = 160.0;
        end
    end

    setEntityActivationOffset(id, dx, dy, dz, r);
end


function keyhole_init(id)    -- Key and puzzle holes

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, false);
    set_activation_offset(id);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (getEntityActivity(object_id))) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        if((not getEntityActivity(object_id)) and (switch_activate(object_id, activator_id) == 1)) then
            setEntityPos(activator_id, getEntityPos(object_id));
            moveEntityLocal(activator_id, getEntityActivationOffset(id));
            entityRotateToTriggerZ(activator_id, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end
        return ENTITY_TRIGGERING_NOT_READY;
    end;
end

function switch_init(id)     -- Ordinary switches
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    set_activation_offset(id);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (getEntityTimer(object_id) > 0.0)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        if(switch_activate(object_id, activator_id) > 0) then
            setEntityPos(activator_id, getEntityPos(object_id));                -- Move activator right next to object.
            moveEntityLocal(activator_id, getEntityActivationOffset(id));       -- Shift activator back to proper distance.
            entityRotateToTriggerZ(activator_id, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if((tickEntity(object_id) == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, 1);
            setEntitySectorStatus(object_id, 1);
            setEntityEvent(object_id, 0);
        end;
    end;
end


function lever_switch_init(id)     -- Big switches (TR4) - lever
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivationOffset(id, 0.0, -520.0, 0.0, 128);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        local state = switch_activate(object_id, activator_id);                 -- 0 - nothing / in process, 1 - on, 2 - off
        if(state > 0) then
            setEntityPos(activator_id, getEntityPos(object_id));                -- Move activator right next to object.
            moveEntityLocal(activator_id, getEntityActivationOffset(id));       -- Shift activator back to proper distance.
            entityRotateToTriggerZ(activator_id, object_id);
            if(state == 1) then
                setEntityActivationDirection(id, 0.0, -1.0, 0.0);
                setEntityActivationOffset(id, 0.0, 520.0, 0.0, 128);
            else
                setEntityActivationDirection(id, 0.0, 1.0, 0.0);
                setEntityActivationOffset(id, 0.0, -520.0, 0.0, 128);
            end;
            setEntityCollision(id, 1);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if((tickEntity(object_id) == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, 0);              -- 0 - diactivate anim
            setEntitySectorStatus(object_id, 1);
            setEntityEvent(object_id, 0);
        end;
    end;
end


function anim_single_init(id)      -- Ordinary one way animatings

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
   
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    --TODO: move that hack to level script (after loading scripts system backporting?)
    if(id == 12) then
        entity_funcs[id].onLoop = function(object_id)
            local x, y, z = getEntityPos(object_id);
            if(x > 48896) then
                x = x - 1024.0 * frame_time;
                if(x < 48896) then
                    x = 48896;
                end;
                setEntityPos(object_id, x, y, z);
            else
                entity_funcs[id].onLoop = nil;
            end;
        end;
    end;
end


function anim_init(id)      -- Ordinary animatings

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    disableEntity(id);
    
    local state_on = 1;
    local state_off = 0;
   
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_on);
        setEntityActivity(object_id, true);
        enableEntity(object_id);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
        setEntityActivity(object_id, false);
        disableEntity(object_id);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
            setEntityActivity(object_id, false);
            -- disable entity?
        end;
    end
end


function pushable_init(id)
    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
end


function pickup_init(id, item_id)    -- Pick-ups

    setEntityTypeFlag(id, ENTITY_TYPE_PICKABLE);
    setEntityActivationOffset(id, 0.0, 0.0, 0.0, 480.0);
    setEntityActivity(id, false);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((item_id == nil) or (object_id == nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        local need_set_pos = true;
        local curr_anim = getEntityAnim(activator_id, ANIM_TYPE_BASE);

        if(curr_anim == 103) then                 -- Stay idle
            local dx, dy, dz = getEntityVector(object_id, activator_id);
            if(dz < -256.0) then
                need_set_pos = false;
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 425, 0); -- Standing pickup, test version
                --noFixEntityCollision(activator_id);
            else
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 135, 0); -- Stay pickup
            end;
        elseif(curr_anim == 222) then             -- Crouch idle
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 291, 0);     -- Crouch pickup
        elseif(curr_anim == 263) then             -- Crawl idle
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 292, 0);     -- Crawl pickup
        elseif(curr_anim == 108) then             -- Swim idle
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 130, 0);     -- Swim pickup
        else
            return ENTITY_TRIGGERING_NOT_READY;                      -- Disable picking up, if Lara isn't idle.
        end;

        addTask(
        function()
            -- Position corrector
            if(getEntityMoveType(activator_id) == MOVE_UNDERWATER) then
                if(getEntityDistance(object_id, activator_id) > 128.0) then
                    moveEntityToEntity(activator_id, object_id, 25.0);
                end;
            elseif(need_set_pos) then
                if(getEntityDistance(object_id, activator_id) > 32.0) then
                    moveEntityToEntity(activator_id, object_id, 50.0);
                end;
            end;
            
            local a, f, c = getEntityAnim(activator_id, ANIM_TYPE_BASE);
            -- Standing pickup anim makes action on frame 40 in TR1-3, in TR4-5
            -- it was generalized with all rest animations by frame 16.
            if((a == 135) and (getLevelVersion() < TR_IV)) then
                if(f < 40) then
                    return true;
                end;
            else
                if(f < 16) then
                    return true;
                end;
            end;

            addItem(activator_id, item_id);
            disableEntity(object_id);
            return false;   -- Item successfully picked up, kill the task.
        end);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
end


function boulder_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
    setEntityAnimFlag(id, ANIM_TYPE_BASE, ANIM_FRAME_LOCK);
    setEntityActivity(id, false);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityBodyMass(object_id, getEntityMeshCount(object_id), 2000.0);
            setEntityActivity(object_id, true);
            
            if(getLevelVersion() < TR_IV) then
                pushEntityBody(object_id, 0, math.random(150) + 2500.0, 10.0, true);
                lockEntityBodyLinearFactor(object_id, 0);
            end;
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end

end


function boulder_heavy_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
    setEntityActivity(id, false);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    createGhosts(id);
    local group = bit32.bor(COLLISION_GROUP_TRIGGERS, COLLISION_GROUP_CHARACTERS);
    local mask = bit32.bor(COLLISION_GROUP_STATIC_ROOM, COLLISION_GROUP_STATIC_OBLECT);
    setEntityCollisionFlags(id, group, nil, mask);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id)) then
            setEntityActivity(object_id, true);
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end

    entity_funcs[id].onLoop = function(object_id)
        if(getEntityActivity(object_id)) then
            moveEntityLocal(object_id, 0.0, 2048.0 * frame_time, 0.0);
            local is_stopped, dx, dy, dz = getEntityCollisionFix(object_id, COLLISION_GROUP_STATIC_ROOM);
            if(is_stopped) then
                is_stopped = dz * dz < dx * dx + dy * dy;
                moveEntityGlobal(object_id, dx, dy, dz);
            end;

            local is_dropped = dropEntity(object_id, frame_time, true);
            if(is_dropped and is_stopped) then
                setEntityActivity(object_id, false);
                dropEntity(object_id, 64.0, true);
            end;
        end;
    end;

    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityActivity(object_id)) then 
            changeCharacterParam(activator_id, PARAM_HEALTH, -35.0 * 60 * frame_time) 
        end;
    end;

end;
