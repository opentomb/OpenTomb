-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016


function save_crystal_init(id)
    disableEntity(id);
end


function door_init(id)   -- NORMAL doors only!

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, true);
    
    entity_funcs[id].state_on = 1;
    entity_funcs[id].state_off = 0;
    local object_mask = getEntityMask(id);
    if(object_mask == 0x1F) then
        setEntityAnimStateHeavy(id, ANIM_TYPE_BASE, entity_funcs[id].state_on);
        setEntityTriggerLayout(id, 0x00, 0, 0); -- Reset activation mask and event.
        entity_funcs[id].state_on = 0;
        entity_funcs[id].state_off = 1;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].state_on);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].state_off);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;
    
    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((tick_state == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].state_off);
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
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            -- floor door 317 anim
            -- vertical door 412 anim
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 412, 0);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
end


function anim_single_init(id)      -- Ordinary one way animatings

    local version = getLevelVersion();
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
   
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, false);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(tick_state == TICK_STOPPED) then
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 0);
            setEntityActivity(object_id, false);
        end;
    end;

    --TODO: move that hack to level script (after loading scripts system backporting?)
    if(version == TR_I) then
        if(id == 12) then
            entity_funcs[id].onLoop = function(object_id, tick_state)
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

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(tick_state == TICK_STOPPED) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, state_off);
            setEntityActivity(object_id, false);
            -- disable entity?
        end;
    end
end


function pushable_init(id)
    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR, 1);
    setEntityTypeFlag(id, ENTITY_TYPE_TRAVERSE, 1);
    if(getLevelVersion() < TR_IV) then
        setEntityTypeFlag(id, ENTITY_TYPE_TRAVERSE_FLOOR, 1);
    end;

    setEntityCollisionFlags(id, COLLISION_GROUP_STATIC_ROOM, nil, nil);
    setEntityCollisionGroupAndMask(id, COLLISION_GROUP_STATIC_ROOM, COLLISION_MASK_ALL);
end


function pickable_init(id)    -- Pick-ups

    local model_id = getEntityModelID(id);
    local pickup_count, item_type, item_id = getBaseItemInfoByWorldID(model_id);

    if(item_id == nil) then
        return;
    end;

    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end;

    setEntityTypeFlag(id, ENTITY_TYPE_PICKABLE);
    setEntityActivationOffset(id, 0.0, 0.0, 0.0, 480.0);
    setEntityActivity(id, false);
    setEntityAnimFlag(id, ANIM_TYPE_BASE, ANIM_FRAME_LOCK);

    removeAllItems(id);
    addItem(id, item_id, pickup_count);

    entity_funcs[id].activator_id = nil;
    entity_funcs[id].need_set_pos = true;

    entity_funcs[id].onSave = function()
        local ret = "";
        if(getEntityTypeFlag(id, ENTITY_TYPE_SPAWNED) ~= 0) then
            ret = "\npickable_init(" .. id .. ");";
        end;

        if(entity_funcs[id].activator_id ~= nil) then
            local addr = "\nentity_funcs[" .. id .. "].";
            ret = ret .. addr .. "activator_id = " .. entity_funcs[id].activator_id .. ";";
            if(entity_funcs[id].need_set_pos) then
                ret = ret .. addr .. "need_set_pos = true;";
            else
                ret = ret .. addr .. "need_set_pos = false;";
            end;
            return ret;
        end;

        return ret;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (entity_funcs[object_id].activator_id ~= nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end

        enableEntity(object_id);
        setEntityActivity(id, false);
        local curr_anim = getEntityAnim(activator_id, ANIM_TYPE_BASE);

        if(curr_anim == 103) then                 -- Stay idle
            local dx, dy, dz = getEntityVector(object_id, activator_id);
            if(dz < -256.0) then
                entity_funcs[object_id].need_set_pos = false;
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 425, 0); -- Standing pickup, test version
                --noEntityMove(activator_id, true);
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

        entity_funcs[object_id].activator_id = activator_id;
        setEntityActivity(object_id, true);

        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(entity_funcs[object_id].activator_id ~= nil) then
            local activator_id = entity_funcs[object_id].activator_id;

            -- Position corrector
            if(getEntityMoveType(activator_id) == MOVE_UNDERWATER) then
                if(getEntityDistance(object_id, activator_id) > 128.0) then
                    moveEntityToEntity(activator_id, object_id, 25.0 * 60.0 * frame_time);
                end;
            elseif(entity_funcs[object_id].need_set_pos) then
                if(getEntityDistance(object_id, activator_id) > 32.0) then
                    moveEntityToEntity(activator_id, object_id, 50.0 * 60.0 * frame_time);
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

            local items = getItems(object_id);
            if(items ~= nil) then
                removeAllItems(object_id);
                for k, v in pairs(items) do
                    addItem(activator_id, k, v);
                end;
            end;
            items = nil;

            disableEntity(object_id);
            entity_funcs[object_id].activator_id = nil;
        end;
    end;
end;


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
    local group = COLLISION_GROUP_KINEMATIC;
    local mask = COLLISION_GROUP_STATIC_ROOM;
    setEntityCollisionFlags(id, group, nil, mask);

    local x, y, z = getEntityPos(id);
    entity_funcs[id].x = x;
    entity_funcs[id].y = y;
    entity_funcs[id].z = z;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(not getEntityActivity(object_id) and (getEntityEvent(object_id) == 0)) then
            setEntityActivity(object_id, true);
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end

    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityPos(object_id, entity_funcs[object_id].x, entity_funcs[object_id].y, entity_funcs[object_id].z);
        setEntityActivity(object_id, false);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getEntityActivity(object_id)) then
            local dy = 2048.0 * frame_time;
            local R = 512;
            local r = 256;
            local test_y = R - r + dy;
            local lim = dy / test_y;
            local is_stopped, t = getEntitySphereTest(object_id, mask, r, 0.0, test_y, 0.0);
            if(is_stopped) then
                dy = t * test_y - (R - r);
                is_stopped = dy < 16.0;
            end;
            
            local need_fix, fx, fy, fz = getEntityCollisionFix(object_id, COLLISION_GROUP_STATIC_ROOM);
            if(need_fix and (fz < 64.0)) then
                moveEntityGlobal(object_id, 0.0, 0.0, fz * 60.0 * frame_time);
            end;

            moveEntityLocal(object_id, 0.0, dy, 0.0);

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


function projectile_init(id, speed, damage)
    if(entity_funcs[id] == nil) then
        entity_funcs[id] = {};
    end;

    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    createGhosts(id);
    local group = COLLISION_GROUP_CHARACTER;
    local mask = COLLISION_GROUP_CHARACTER;
    local filter = bit32.bor(COLLISION_GROUP_STATIC_ROOM, COLLISION_GROUP_STATIC_OBLECT, COLLISION_GROUP_KINEMATIC);
    setEntityCollisionFlags(id, group, nil, mask);
    setEntityGhostCollisionShape(id, 0, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityActivity(id, true);

    entity_funcs[id].onSave = function()
        return "projectile_init(" .. id .. ", " .. speed .. ", " .. damage .. ");\n";
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        moveEntityLocal(object_id, 0.0, speed * frame_time, 0.0);
        if(getEntityCollisionFix(object_id, filter)) then
            setEntityStateFlag(object_id, ENTITY_STATE_DELETED, 1);
            print("projectile wall hit: " .. object_id);
        end;
    end;

    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -damage);
        disableEntity(object_id);
        setEntityStateFlag(object_id, ENTITY_STATE_DELETED, 1);
        print("projectile character hit: " .. object_id);
    end;
end


function zipline_init(id)
    setEntityActivity(id, false);
    setEntityAnim(id, ANIM_TYPE_BASE, 0, 0);
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);

    setEntityActivationOffset(id, 0.0, 350.0, 0.0, 256.0);
    setEntityActivationDirection(id, 0.0, 1.0, 0.0, 0.77);
    
    entity_funcs[id].activator_id = nil;
    entity_funcs[id].x0, entity_funcs[id].y0, entity_funcs[id].z0 = getEntityPos(id); 
    local anim_grab = 215;
    local anim_drop = 217;
    if(getLevelVersion() >= TR_III) then
        anim_grab = 214;
        anim_drop = 216;
    end;

    entity_funcs[id].onSave = function()
        if(entity_funcs[id].activator_id ~= nil) then
            local addr = "\nentity_funcs[" .. id .. "].";
            return addr .. "activator_id = " .. entity_funcs[id].activator_id .. ";";
        end;
        return "";
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((getEntityEvent(object_id) == 0) and canTriggerEntity(activator_id, object_id)) then
            setEntityEvent(object_id, 1);
            setEntityActivity(object_id, true);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, anim_grab, 0);
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            entity_funcs[id].activator_id = activator_id;
            return ENTITY_TRIGGERING_ACTIVATED;
        elseif((getEntityEvent(object_id) == 1) and (not getEntityActivity(object_id))) then
            setEntityEvent(object_id, 0);
            setEntityActivity(object_id, false);
            setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
            setEntityPos(object_id, entity_funcs[object_id].x0, entity_funcs[object_id].y0, entity_funcs[object_id].z0);
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;

    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityEvent(object_id, 0);
        setEntityActivity(object_id, false);
        setEntityAnim(object_id, ANIM_TYPE_BASE, 0, 0);
        setEntityPos(object_id, entity_funcs[object_id].x0, entity_funcs[object_id].y0, entity_funcs[object_id].z0);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a == 1) then
            local dx = 0.0;
            local dy = 3072.0 * frame_time;
            local dz = 0.0 - dy / 4.0;
            local hit = getEntityRayTest(object_id, COLLISION_GROUP_STATIC_ROOM, dx, dy + 128.0, dz, 0, 0, 0);
            if(hit) then
                if(entity_funcs[object_id].activator_id ~= nil) then
                    setEntityAnim(entity_funcs[object_id].activator_id, ANIM_TYPE_BASE, anim_drop, 0);
                end;
                entity_funcs[object_id].activator_id = nil;
                setEntityActivity(object_id, false);
            else
                moveEntityLocal(object_id, dx, dy, dz);
            end;
        end;

        if(entity_funcs[object_id].activator_id ~= nil) then
            if(70 == getEntityAnimState(entity_funcs[object_id].activator_id, ANIM_TYPE_BASE)) then
                setEntityPos(entity_funcs[object_id].activator_id, getEntityPos(object_id));
            elseif(a == 1) then
                entity_funcs[object_id].activator_id = nil;
            end;
        end;
    end;
end;


function breakable_window_init(id)
    setEntityActivity(id, false);

    entity_funcs[id].onHit = function(object_id, activator_id)
        setEntityActivity(object_id, true);
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(f >= c - 1) then
            disableEntity(object_id);
        end;
    end;
end;

--WORKAROUND
function breakable_window_jmp_init(id)
    breakable_window_init(id);
end;
