-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016

-- Load up some extra entity scripts.

-- TODO: delete PUSH - Lara are moved up without it
function twobp_init(id)        -- Two-block platform

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityCallbackFlag(id, ENTITY_CALLBACK_STAND, 1);
    setEntityActivity(id, false);
    
    entity_funcs[id].push_height    = 128.0;
    entity_funcs[id].push_speed     = 2.0 * 60;
    entity_funcs[id].push           = false;   -- Push flag, set when Lara is on platform.
    entity_funcs[id].current_height = 0.0;     -- Used for all modes.
    entity_funcs[id].waiting        = true;    -- Initial state is stopped.
    
    local curr_OCB = getEntityOCB(id);
    
    entity_funcs[id].raise_height = bit32.rshift(bit32.band(curr_OCB, 0xFFF0), 4) * 256.0;
    entity_funcs[id].raise_speed  = bit32.band(curr_OCB, 0x000F) * 60 / 2.0; -- Double FPS.
    
    -- Mode legend: 0 - normal ascent, 1 - normal descent, 2 - push.
    -- Only two classic modes are parsed from OCB, extra modes can be implemented through script.
    
    if(curr_OCB == 0) then 
        entity_funcs[id].mode = 2 
    else 
        entity_funcs[id].mode = 0 
    end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) == 1) then
            return ENTITY_TRIGGERING_NOT_READY;
        end;

        if(getEntityActivity(object_id)) then
            if(entity_funcs[object_id].mode < 2) then
                if(entity_funcs[object_id].mode == 0) then 
                    entity_funcs[object_id].mode = 1 
                else 
                    entity_funcs[object_id].mode = 0 
                end;
                entity_funcs[object_id].current_height = entity_funcs[object_id].raise_height - entity_funcs[object_id].current_height;
            else
                setEntityActivity(object_id, false);
            end;
        else
            setEntityActivity(object_id, true);
        end;
        
        entity_funcs[object_id].waiting = false;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;
    
    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        if(getEntityEvent(object_id) ~= 1) then
            return ENTITY_TRIGGERING_DEACTIVATED;
        end;

        return entity_funcs[id].onActivate(object_id, activator_id);
    end;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].waiting == false) then
            if(entity_funcs[object_id].mode < 2) then
                if(entity_funcs[object_id].current_height < entity_funcs[object_id].raise_height) then
                    local dz = entity_funcs[object_id].raise_speed * frame_time;
                    if(entity_funcs[object_id].mode == 0) then
                        moveEntityLocal(object_id, 0.0, 0.0, dz);
                    else
                        moveEntityLocal(object_id, 0.0, 0.0, -dz);
                    end;
                    
                    entity_funcs[object_id].current_height = entity_funcs[object_id].current_height + dz;
                else
                    if(entity_funcs[object_id].mode == 0) then 
                        entity_funcs[object_id].mode = 1 
                    else 
                        entity_funcs[object_id].mode = 0 
                    end;
                    entity_funcs[object_id].waiting = true;
                    entity_funcs[object_id].current_height = 0.0; -- Reset height counter.
                    setEntityActivity(object_id, false);
                end;
            elseif(entity_funcs[object_id].mode == 2) then
                if(entity_funcs[object_id].push == true) then
                    if(entity_funcs[object_id].current_height < entity_funcs[object_id].push_height) then
                        local dz = entity_funcs[object_id].push_speed * frame_time;
                        moveEntityLocal(object_id, 0.0, 0.0, -dz);
                        entity_funcs[object_id].current_height = entity_funcs[object_id].current_height + dz;
                    else
                        entity_funcs[object_id].current_height = entity_funcs[object_id].push_height;
                    end;
                else
                    if(entity_funcs[object_id].current_height > 0.0) then
                        local dz = entity_funcs[object_id].push_speed * frame_time;
                        moveEntityLocal(object_id, 0.0, 0.0, dz);
                        entity_funcs[object_id].current_height = entity_funcs[object_id].current_height - dz;
                    else
                        entity_funcs[object_id].current_height = 0.0;
                    end;
                end;
                
                entity_funcs[object_id].push = false;
            end;
        end;
    end;
    
    entity_funcs[id].onStand = function(object_id, activator_id)
        if(getEntityActivity(object_id) and (getEntityModel(activator_id) == 0)) then  -- Lara
            entity_funcs[object_id].push = true;
        end;
    end;
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].push_height     = nil;
        entity_funcs[object_id].push_speed      = nil;
        entity_funcs[object_id].push            = nil;
        entity_funcs[object_id].raise_height    = nil;
        entity_funcs[object_id].raise_speed     = nil;
        entity_funcs[object_id].waiting         = nil;
        entity_funcs[object_id].mode            = nil;
        entity_funcs[object_id].current_height  = nil;
    end
    
    prepareEntity(id);
end


function rblock_init(id)        -- Raising block (generic)
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].max_height  = 1024.0;
    entity_funcs[id].move_speed  = 8.0 * 60;
    entity_funcs[id].dummy       = (getEntityOCB(id) == -2);    -- TR5 functionality
    
    if(entity_funcs[id].dummy == true) then
        setEntityScaling(id, 1.0, 1.0, 1.0);
        setEntityVisibility(id, 0);
        entity_funcs[id].curr_height = entity_funcs[id].max_height;
        entity_funcs[id].direction   = 2;
    else
        setEntityScaling(id, 1.0, 1.0, 0.0);
        entity_funcs[id].curr_height = 0.0;
        entity_funcs[id].direction   = 1;
    end;
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].direction == 1) then
            if((entity_funcs[object_id].dummy == false) and (entity_funcs[object_id].curr_height < entity_funcs[object_id].max_height)) then
                local dz = frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_height = entity_funcs[object_id].curr_height + dz;
                camShake(125.0, 0.2, object_id);
            else
                entity_funcs[object_id].curr_height = entity_funcs[object_id].max_height;
                entity_funcs[object_id].direction = 2;
                setEntityActivity(object_id, false);
            end;
        else
            if((entity_funcs[object_id].dummy == false) and (entity_funcs[object_id].curr_height > 0.0)) then
                local dz = frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_height = entity_funcs[object_id].curr_height - dz;
                camShake(125.0, 0.2, object_id);
            else
                entity_funcs[object_id].curr_height = 0.0;
                entity_funcs[object_id].direction = 1;
                setEntityActivity(object_id, false);
            end;
        end;
        
        setEntityScaling(object_id, 1.0, 1.0, (entity_funcs[object_id].curr_height / entity_funcs[object_id].max_height));
    end;
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].curr_height = nil;
        entity_funcs[object_id].max_height  = nil;
        entity_funcs[object_id].move_speed  = nil;
        entity_funcs[object_id].direction   = nil;
        entity_funcs[object_id].dummy       = nil;
    end
    
    prepareEntity(id);
    
end


function rblock2_init(id)   -- Raising block x2 - same as RB1, only max height / speed is changed.
    rblock_init(id);
    entity_funcs[id].max_height = 2048.0;
    entity_funcs[id].move_speed = 16.0;
end


function expplatform_init(id)        -- Expanding platform
    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);
    
    entity_funcs[id].max_width  = 1024.0;
    entity_funcs[id].move_speed = 8.0 * 60;
    entity_funcs[id].curr_width = 0.0;
    entity_funcs[id].direction  = 1;
    
    setEntityScaling(id, 1.0, 0.0, 1.0);
    moveEntityLocal(id, 0.0, entity_funcs[id].max_width / 2, 0.0);  -- Fix position
    
    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end
    
    entity_funcs[id].onDeactivate = entity_funcs[id].onActivate;
    
    entity_funcs[id].onLoop = function(object_id, activator_id)
        if(entity_funcs[object_id].direction == 1) then
            if(entity_funcs[object_id].curr_width < entity_funcs[object_id].max_width) then
                local dz = frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_width = entity_funcs[object_id].curr_width + dz;
            else
                entity_funcs[object_id].curr_width = entity_funcs[object_id].max_width;
                entity_funcs[object_id].direction = 2;
                setEntityActivity(object_id, false);
            end;
        else
            if(entity_funcs[object_id].curr_width > 0.0) then
                local dz = frame_time * entity_funcs[object_id].move_speed;
                entity_funcs[object_id].curr_width = entity_funcs[object_id].curr_width - dz;
            else
                entity_funcs[object_id].curr_width = 0.0;
                entity_funcs[object_id].direction = 1;
                setEntityActivity(object_id, false);
            end;
        end;
        
        setEntityScaling(object_id, 1.0, (entity_funcs[object_id].curr_width / entity_funcs[object_id].max_width), 1.0);
        
    end;
    
    entity_funcs[id].onDelete = function(object_id)
        entity_funcs[object_id].curr_width = nil;
        entity_funcs[object_id].max_width  = nil;
        entity_funcs[object_id].move_speed  = nil;
        entity_funcs[object_id].direction   = nil;
    end
    
    prepareEntity(id);
end
