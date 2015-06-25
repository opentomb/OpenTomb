-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2015

--------------------------------------------------------------------------------
-- This script contains routines for all entity functions. Note that it is not
-- just entity functions, but "function creation" routines, which usually
-- includes some events executed on level start-up. Function itself is placed
-- inside entity function array (entity_funcs).
--------------------------------------------------------------------------------

entity_funcs = {};  -- Initialize entity function array.

-- Load up some extra entity scripts.

dofile("scripts/entity/script_switch.lua");     -- Additional switch scripts.

-- Erase single entity function.

function efuncs_EraseEntity(index)
    if(entity_funcs[index] ~= nil) then
        if(entity_funcs[index].onDelete ~= nil) then    -- Entity-specific clean-up.
            entity_funcs[index].onDelete(index);
            entity_funcs[index].onDelete = nil;
        end;
        entity_funcs[index].onActivate       = nil;
        entity_funcs[index].onDeactivate     = nil;
        entity_funcs[index].onCollide        = nil;
        entity_funcs[index].onStand          = nil;
        entity_funcs[index].onHit            = nil;
        entity_funcs[index].onLoop           = nil;
        entity_funcs[index].onSave           = nil;
        entity_funcs[index].onLoad           = nil;
        entity_funcs[index]                  = nil;
    end;
end;

-- Clear whole entity functions array. Must be called on each level loading.

function entfuncs_Clear()
    for k,v in pairs(entity_funcs) do
        efuncs_EraseEntity(k);
    end;
    print("Entity function table cleaned");
end;

--------------------------------------------------------------------------------
-- ENTITY FUNCTIONS IMPLEMENTATION
--------------------------------------------------------------------------------

function door_init(id)   -- NORMAL doors only!

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(object_id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil) then return end;
        swapEntityState(object_id, 0, 1);
    end;
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;    -- Same function.
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            swapEntityState(object_id, 0, 1);
            setEntityEvent(object_id, 0);
        end;
    end
    
    prepareEntity(id);
end

function keyhole_init(id)    -- Key and puzzle holes

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil or getEntityActivity(object_id) >= 1 or canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0) ~= 1) then
            return;
        end
        
        if((getEntityActivity(object_id) == 0) and (switch_activate(object_id, activator_id) == 1)) then
            setEntityPos(activator_id, getEntityPos(object_id));
            moveEntityLocal(activator_id, 0.0, 360.0, 0.0);
        end
    end;
end

function switch_init(id)     -- Ordinary switches
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(object_id == nil or canTriggerEntity(activator_id, object_id, 256.0, 0.0, 256.0, 0.0) ~= 1) then
            return;
        end
        
        if(switch_activate(object_id, activator_id) == 1) then
            setEntityPos(activator_id, getEntityPos(object_id));    -- Move activator right next to object.
            moveEntityLocal(activator_id, 0.0, 360.0, 0.0);         -- Shift activator back to proper distance.
        end;
    end;
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityState(object_id, 1);
            setEntitySectorStatus(object_id, 1);
        end;
    end;
end

function anim_init(id)      -- Ordinary animatings

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(object_id, 0);
    disableEntity(id);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        enableEntity(id);
        setEntityActivity(object_id, 1);
        setEntityState(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        disableEntity(id);
        setEntityActivity(object_id, 0);
        setEntityState(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
end

function pushable_init(id)
    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
end

function venicebird_init(id)    -- Venice singing birds (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        if(getEntityDistance(player, object_id) < 8192.0) then
            if(math.random(100000) > 99500) then playSound(316, object_id) end;
        end;
    end
    
    prepareEntity(id);
end

function drips_init(id)    -- Maria Doria drips (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        if(getEntityDistance(player, object_id) < 8192.0) then
            if(math.random(100000) > 99500) then playSound(329, object_id) end;
        end;
    end
    
    prepareEntity(id);
end

function doorbell_init(id)    -- Lara's Home doorbell (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityDistance(player, object_id) < 4096.0) then
            playSound(334, object_id);
            setEntityActivity(object_id, 0);
        end;
    end
end

function alarm_TR2_init(id)    -- Offshore Rig alarm (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 0) then setEntityActivity(object_id, 1) end;
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 1) then
            setEntityActivity(object_id, 0);
            stopSound(332, object_id);
        end;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        playSound(332, object_id);
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityActivity(object_id, 0)
            stopSound(332, object_id);
        end;
    end
end

function alarmbell_init(id)    -- Home Sweet Home alarm (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 0) then setEntityActivity(object_id, 1) end;
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 1) then
            setEntityActivity(object_id, 0);
            stopSound(335, object_id);
        end;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        playSound(335, object_id);
        if(tickEntity(object_id) == TICK_STOPPED) then
            setEntityActivity(object_id, 0)
            stopSound(335, object_id);
        end;
    end
end

function heli_TR2_init(id)    -- Helicopter (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    setEntityVisibility(id, 0);
    
    entity_funcs[id].distance_passed = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 0) then
            setEntityActivity(object_id, 1);
            setEntityVisibility(id, 1);
            playSound(297, object_id);
        end;
    end
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        entity_funcs[object_id].distance_passed = entity_funcs[object_id].distance_passed + 40.0;
        moveEntityLocal(object_id, 0.0, 40.0, 0.0);
        if(entity_funcs[object_id].distance_passed > 30720) then
            stopSound(297, object_id);
            disableEntity(object_id);
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].distance_passed = nil;
    end
    
    prepareEntity(id);
end

function heli_rig_TR2_init(id)    -- Helicopter in Offshore Rig (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    setEntityVisibility(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        setEntityVisibility(object_id, 1);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityLock(object_id) == 1) then
            if(getEntityState(object_id) ~= 2) then
                setEntityState(object_id, 2);
            else
                local anim, frame, count = getEntityAnim(object_id);
                if(frame == count-1) then disableEntity(object_id) end;
            end
        end;
    end
    
    prepareEntity(id);
end

function swingblade_init(id)        -- Swinging blades (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityState(object_id, 2);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityState(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -45);
    end
    
    prepareEntity(id);
end

function tallblock_init(id)    -- Tall moving block (TR1)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 0);
    
    entity_funcs[id].distance_passed = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 0) then
            setEntityActivity(object_id, 1);
            playSound(64, object_id);
        end;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id)
        local move_speed = 32.0;
        if(getEntityEvent(object_id) == 0) then move_speed = 0 - move_speed end;
        
        entity_funcs[object_id].distance_passed = entity_funcs[object_id].distance_passed + move_speed;
        moveEntityLocal(object_id, 0.0, move_speed, 0.0);
        if(math.abs(entity_funcs[object_id].distance_passed) >= 2048.0) then
            stopSound(64, object_id);
            setEntityActivity(object_id, 0);
            entity_funcs[object_id].distance_passed = 0;
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].distance_passed = nil;
    end
    
    prepareEntity(id);
end

function slamdoor_init(id)      -- Slamming doors (TR1-TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityState(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityState(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityState(object_id, 0) end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -50);
    end
    
    prepareEntity(id);
end

function wallblade_init(id)     -- Wall blade (TR1-TR3)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(tickEntity(object_id) == TICK_STOPPED) then setEntityActivity(object_id, 0) end;
        local anim_number = getEntityAnim(object_id);
        if(anim_number == 2) then
            setEntityAnim(object_id, 3);
        elseif(anim_number == 1) then
            setEntityAnim(object_id, 0);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        changeCharacterParam(activator_id, PARAM_HEALTH, -50);
    end
    
    prepareEntity(id);
end

function boulder_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR);
    setEntityAnimFlag(id, ANIM_LOCK);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id == 0)) then
            setEntityBodyMass(object_id, getEntityMeshCount(object_id), 2000.0);
            setEntityActivity(object_id, 1);
            
            if(getLevelVersion() < TR_IV) then
                pushEntityBody(object_id, 0, math.random(150) + 2500.0, 10.0, true);
                lockEntityBodyLinearFactor(object_id, 0);
            end;
        end;
    end

end

function pickup_init(id, item_id)    -- Pick-ups

    setEntityTypeFlag(id, ENTITY_TYPE_PICKABLE);
    setEntityActivationOffset(id, 0.0, 0.0, 0.0, 480.0);
    setEntityActivity(id, 0);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((item_id == nil) or (object_id == nil)) then
            return;
        end
        
        local need_set_pos = true;
        local curr_anim = getEntityAnim(activator_id);

        if(curr_anim == 103) then                 -- Stay idle
            local dx, dy, dz = getEntityVector(object_id, activator_id);
            if(dz < -256.0) then
                need_set_pos = false;
                setEntityAnim(activator_id, 425); -- Standing pickup, test version
            else
                setEntityAnim(activator_id, 135); -- Stay pickup
            end;
        elseif(curr_anim == 222) then             -- Crouch idle
            setEntityAnim(activator_id, 291);     -- Crouch pickup
        elseif(curr_anim == 263) then             -- Crawl idle
            setEntityAnim(activator_id, 292);     -- Crawl pickup
        elseif(curr_anim == 108) then             -- Swim idle
            setEntityAnim(activator_id, 130);     -- Swim pickup
        else
            return;     -- Disable picking up, if Lara isn't idle.
        end;

        addTask(
        function()
        
            -- Position corrector
        
            if(getEntityMoveType(activator_id) == MOVE_UNDERWATER) then
                if(getEntityDistance(object_id, activator_id) > 128.0) then
                    moveEntityToEntity(activator_id, object_id, 25.0);
                end;
            else
                if(getEntityDistance(object_id, activator_id) > 32.0) then
                    moveEntityToEntity(activator_id, object_id, 50.0);
                end;
            end;
            
            local a, f, c = getEntityAnim(activator_id);
            
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
    end;
end

function crystal_TR3_init(id)   -- "Savegame" crystal (TR3 version)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityDistance(player, object_id) < 512.0) then
            playSound(SOUND_MEDIPACK);
            changeCharacterParam(player, PARAM_HEALTH, 200);
            disableEntity(object_id);
        end;
    end
end

function fallblock_init(id)  -- Falling block (TR1-3)

    setEntityCallbackFlag(id, ENTITY_CALLBACK_STAND, 1);
    setEntitySpeed(id, 0.0, 0.0, 0.0);

    entity_funcs[id].onStand = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end

        local anim = getEntityAnim(object_id);
        if(anim == 0) then
            setEntityAnim(object_id, 1);
            -- print("you trapped to id = "..object_id);
            local once = true;
            addTask(
            function()
                local anim = getEntityAnim(object_id);
                if(anim == 1) then
                    return true;
                end;
                if(once) then
                    setEntityCollision(object_id, 0);
                    once = false;
                end;
                if(dropEntity(object_id, frame_time)) then
                    setEntityAnim(object_id, 3);
                    return false;
                end;
                return true;
            end);
        end;
    end;
end

function fallceiling_init(id)  -- Falling ceiling (TR1-3)

    setEntitySpeed(id, 0.0, 0.0, 0.0);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    
    local level_version = getLevelVersion();
    if((level_version < TR_II) or (level_version >= TR_III)) then setEntityVisibility(id, 0) end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end
        
        local anim = getEntityAnim(object_id);
        if(anim == 0) then
            setEntityAnim(object_id, 1);
            setEntityVisibility(object_id, 1);
            addTask(
            function()
                if(dropEntity(object_id, frame_time)) then
                    setEntityAnim(object_id, 2);
                    setEntityCollision(object_id, 0);
                    return false;
                end;
                return true;
            end);
        end;
    end;
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityAnim(object_id) == 1) and (getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
        end;
    end
end

function pushdoor_init(id)   -- Pushdoors (TR4)

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, 0);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return;
        end;

        if((getEntityActivity(object_id) == 0) and (getEntityDirDot(object_id, activator_id) < -0.9)) then
            setEntityActivity(object_id, 1);
            local x, y, z, az, ax, ay = getEntityPos(object_id);
            setEntityPos(activator_id, x, y, z, az + 180.0, ax, ay);
            moveEntityLocal(activator_id, 0.0, 256.0, 0.0);
            -- floor door 317 anim
            -- vertical door 412 anim
            setEntityAnim(activator_id, 412);
        end;
    end;
end

function midastouch_init(id)    -- Midas gold touch

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    
    entity_funcs[id].onLoop = function(object_id)
        if(getEntityDistance(player, object_id) < 1024.0) then
            local lara_anim, frame, count = getEntityAnim(player);
            local lara_sector = getEntitySectorIndex(player);
            local hand_sector = getEntitySectorIndex(object_id);
            
            if((lara_sector == hand_sector) and (getEntityMoveType(player) == MOVE_ON_FLOOR) and (getEntityAnim(player) ~= 50)) then
                setCharacterParam(player, PARAM_HEALTH, 0);
                setEntityAnim(player, 1, 0, 5);
                disableEntity(object_id);
            end;
        end;
    end
    
    prepareEntity(id);
end

function oldspike_init(id)  -- Teeth spikes

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 1);
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
            local ls = getEntitySpeedLinear(activator_id);
                
            if(lz > (pz + 256.0)) then
                setEntityCollision(object_id, 0);   -- Temporary, must use ghost collision instead.
                setEntityPos(activator_id, lx, ly, pz);
                setEntityAnim(activator_id, 149, 0);
                setCharacterParam(activator_id, PARAM_HEALTH, 0);
            elseif(ls > 512.0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -(ls / 256.0));
            end;
        end;
    end
    
end

function spikewall_init(id)      -- Spike wall

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local ver = getLevelVersion();
        local scan_distance = 32.0;
        if(ver < TR_II) then scan_distance = 1536.0 end; -- TR1's lava mass has different floor scan distance.
        
        if(similarSector(object_id, 0.0, scan_distance, 0.0, false)) then
            moveEntityLocal(object_id, 0.0, 8.0, 0.0);
            playSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        else
            setEntityActivity(object_id, 0);    -- Stop
            stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityModelID(activator_id) == 0) then
            local curr_st = getEntityMoveType(activator_id);
            if((curr_st == MOVE_CLIMBING) or (curr_st == MOVE_MONKEYSWING)) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, 28, 0);
            elseif(curr_st == MOVE_WALLS_CLIMB) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, 30, 0);
            end;
            
            if(getCharacterParam(activator_id, PARAM_HEALTH) > 0) then
                changeCharacterParam(activator_id, PARAM_HEALTH, -20);
                playSound(getGlobalSound(getLevelVersion(), GLOBALID_SPIKEHIT), activator_id);
                if(getCharacterParam(activator_id, PARAM_HEALTH) <= 0) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    playSound(SOUND_GEN_DEATH, activator_id);
                    setEntityActivity(object_id, 0);
                    stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
                end;
            end;
        end;
    end
end

function spikeceiling_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        local px, py, pz = getEntityPos(object_id);

        if(pz > (getSectorHeight(object_id) + 512.0)) then
            moveEntityLocal(object_id, 0.0, 0.0, -4.0);
            playSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        else
            setEntityActivity(object_id, 0);    -- Stop
            stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if(getEntityModelID(activator_id) == 0) then
            local curr_st = getEntityMoveType(activator_id);
            if((curr_st == MOVE_CLIMBING) or (curr_st == MOVE_MONKEYSWING)) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, 28, 0);
            elseif(curr_st == MOVE_WALLS_CLIMB) then
                setEntityMoveType(activator_id, MOVE_FREE_FALLING);
                setEntityAnim(activator_id, 30, 0);
            end;
        
            local px,py,pz = getEntityPos(object_id);
            local lx,ly,lz = getEntityPos(activator_id);
                
            if(getCharacterParam(activator_id, PARAM_HEALTH) > 0) then
                if(lz > (pz - 256.0)) then
                    addEntityRagdoll(activator_id, RD_TYPE_LARA);
                    setCharacterParam(activator_id, PARAM_HEALTH, 0);
                    playSound(SOUND_IMPALE, activator_id);
                else
                    changeCharacterParam(activator_id, PARAM_HEALTH, -20);
                    playSound(getGlobalSound(getLevelVersion(), GLOBALID_SPIKEHIT), activator_id);
                    if(getCharacterParam(activator_id, PARAM_HEALTH) <= 0) then
                        addEntityRagdoll(activator_id, RD_TYPE_LARA);
                        playSound(SOUND_GEN_DEATH, activator_id);
                        setEntityActivity(object_id, 0);
                        stopSound(getGlobalSound(getLevelVersion(), GLOBALID_MOVINGWALL), object_id);
                    end;
                end;
            end;
        end;
    end
end

function cleaner_init(id)      -- Thames Wharf machine

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].rotating = 0;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
    end
    
    entity_funcs[id].onLoop = function(object_id)
        if(similarSector(object_id, 1024.0, 0.0, 0.0, false)) then
        
        else
        
        end;
    end
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
    
    end
end

function damocles_init(id)      -- Sword of Damocles

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_COLLISION, 1);
    setEntityActivity(id, 0);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, 1);
        entity_funcs[id].rot_speed = (math.random(20) + 1) / 10;
        entity_funcs[id].falling = false;
    end    
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, 0);
        entity_funcs[id].rot_speed = 0.0;
    end
    
    entity_funcs[id].onLoop = function(object_id)
        rotateEntity(object_id, entity_funcs[object_id].rot_speed);
        
        if(sameRoom(player, object_id)) then
            local dx,dy,dz = getEntityVector(player, object_id);
            if((math.abs(dx) < 1024.0) and (math.abs(dy) < 1024.0) and (entity_funcs[object_id].falling == false)) then
                entity_funcs[object_id].falling = true;
                addTask(
                function()
                    moveEntityToEntity(object_id, player, 24.0, true);
                    if(dropEntity(object_id, frame_time, true)) then
                        playSound(103, object_id);
                        setEntityActivity(object_id, 0);
                        entity_funcs[object_id].falling = false;
                        return false;
                    end;
                    return true;
                end);
            end;
        end
    end    
    
    entity_funcs[id].onCollide = function(object_id, activator_id)
        if((entity_funcs[object_id].falling == true) and (getEntityModelID(activator_id) == 0) and (getCharacterParam(activator_id, PARAM_HEALTH) > 0)) then
            setCharacterParam(activator_id, PARAM_HEALTH, 0);
            playSound(SOUND_GEN_DEATH, activator_id);
            playSound(103, object_id);
            addEntityRagdoll(activator_id, RD_TYPE_LARA);
            setEntityActivity(object_id, 0);
            setEntityBodyMass(object_id, 1, 15.0);
        end;
    end
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].rot_speed = nil;
        entity_funcs[object_id].falling = nil;
    end
end

function baddie_init(id)    -- INVALID!

    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR);
    disableEntity(id);
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityActivity(object_id) == 0) then enableEntity(object_id) end;
    end;
    
end