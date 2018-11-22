-- OPENTOMB FLIPEFFECT TRIGGER FUNCTION SCRIPT
-- by Lwmte, April 2015
print("flipeffects->loaded !");

--------------------------------------------------------------------------------
-- In this file you can put extra flipeffect functions which act in the same way
-- as Flipeffect Editor in TREP - all flipeffects above 47 are hardcoded, and
-- remaining ones are refering to this function table. Only difference is, if
-- there IS flipeffect above 47 in the table, then table function is used
-- instead of hardcoded one; it is needed for some version-specific flipeffects.
--------------------------------------------------------------------------------

tr1_flipeffects = {};
tr2_flipeffects = {};
tr3_flipeffects = {};
tr4_flipeffects = {};
tr5_flipeffects = {};

-- Flipeffects implementation

-- Shake camera
tr1_flipeffects[1] = {};
tr1_flipeffects[1].doEffect = function(caller_id, parameter) 
    camShake(15, 1);
end;
tr2_flipeffects[11] = tr1_flipeffects[11];
tr3_flipeffects[11] = tr1_flipeffects[11];
tr4_flipeffects[11] = tr1_flipeffects[11];
tr5_flipeffects[11] = tr1_flipeffects[11];

-- Play desired sound ID
tr4_flipeffects[10] = {};
tr4_flipeffects[10].doEffect = function(caller_id, parameter) 
    if(caller_id >= 0) then
        playSound(parameter, caller_id);
    else
        playSound(parameter);
    end;
end;
tr5_flipeffects[10] = tr4_flipeffects[10];

-- Play explosion sound and effect
tr1_flipeffects[11] = {};
tr1_flipeffects[11].doEffect = function(caller_id, parameter)
    flashSetup(255, 255, 220, 80, 10, 600);
    flashStart();
    playSound(105);
end;
tr2_flipeffects[11] = tr1_flipeffects[11];
tr3_flipeffects[11] = tr1_flipeffects[11];
tr4_flipeffects[11] = tr1_flipeffects[11];
tr5_flipeffects[11] = tr1_flipeffects[11];

-- room flickering effect
tr1_flipeffects[16] = {};
tr1_flipeffects[16].doEffect = function(caller_id, parameter)
    if((tr1_flipeffects[16].state == nil) and ((parameter == true) or (getFlipState(0) == 1))) then
        tr1_flipeffects[16].timer = 4.0;
        tr1_flipeffects[16].state = 0;
        addTask(function()
            if(tr1_flipeffects[16].state == 0) then
                tr1_flipeffects[16].timer = tr1_flipeffects[16].timer - frame_time;
                if(tr1_flipeffects[16].timer <= 0.0) then
                    tr1_flipeffects[16].timer = 0.2;
                    tr1_flipeffects[16].state = 1;
                    setFlipState(0x00, 0);
                end;
            elseif(tr1_flipeffects[16].state == 1) then
                tr1_flipeffects[16].timer = tr1_flipeffects[16].timer - frame_time;
                if(tr1_flipeffects[16].timer <= 0.0) then
                    tr1_flipeffects[16].timer = 1.0;
                    tr1_flipeffects[16].state = 2;
                    setFlipState(0x00, 1);
                end;
            elseif(tr1_flipeffects[16].state == 2) then
                tr1_flipeffects[16].timer = tr1_flipeffects[16].timer - frame_time;
                if(tr1_flipeffects[16].timer <= 0.0) then
                    tr1_flipeffects[16].timer = 0.0;
                    setFlipState(0x00, 0);
                    tr1_flipeffects[16].state = nil;
                    return false;
                end;
            else
                setFlipState(0x00, 0);
                tr1_flipeffects[16].state = nil;
                return false;
            end;
            
            return true;
        end);
    else
        tr1_flipeffects[16].state = nil;
        setFlipState(0x00, 0);
    end;
end;

tr1_flipeffects[16].saveEffect = function()
    print("save effect 16");
    local ret = "";
    if(tr1_flipeffects[16].state ~= nil) then
        ret = "\ntr1_flipeffects[16].doEffect(nil, true);";
        ret = ret .. "\ntr1_flipeffects[16].state = " .. tr1_flipeffects[16].state .. ";";
        ret = ret .. "\ntr1_flipeffects[16].timer = " .. tr1_flipeffects[16].timer .. ";";
        ret = ret .. "\nsetFlipState(0x00, " .. getFlipState(0) .. ");";
    end;
    return ret;
end;

-- Does specified flipeffect.
function doFlipEffect(effect_index, caller_id, parameter) -- extra parameter is usually the timer field
    print("flip effect[" .. effect_index .. "]doEffect(" .. parameter .. ")");
    
    local ver = getLevelVersion();
    if(ver < TR_II) then
        if((tr1_flipeffects[effect_index] ~= nil) and (tr1_flipeffects[effect_index].doEffect ~= nil)) then
            return tr1_flipeffects[effect_index].doEffect(caller_id, parameter);
        end;
    elseif(ver < TR_III) then
        if((tr2_flipeffects[effect_index] ~= nil) and (tr2_flipeffects[effect_index].doEffect ~= nil)) then
            return tr2_flipeffects[effect_index].doEffect(caller_id, parameter);
        end;
    elseif(ver < TR_IV) then
        if((tr3_flipeffects[effect_index] ~= nil) and (tr3_flipeffects[effect_index].doEffect ~= nil)) then
            return tr3_flipeffects[effect_index].doEffect(caller_id, parameter);
        end;
    elseif(ver < TR_V) then
        if((tr4_flipeffects[effect_index] ~= nil) and (tr4_flipeffects[effect_index].doEffect ~= nil)) then
            return tr4_flipeffects[effect_index].doEffect(caller_id, parameter);
        end;
    else
        if((tr5_flipeffects[effect_index] ~= nil) and (tr5_flipeffects[effect_index].doEffect ~= nil)) then
            return tr5_flipeffects[effect_index].doEffect(caller_id, parameter);
        end;
    end;
end


function onSaveFlipEffects()
    local ret = "-- save flip effects";
    local ver = getLevelVersion();
    if(ver < TR_II) then
        for k, v in pairs(tr1_flipeffects) do
            if(v.saveEffect ~= nil) then
                ret = ret .. v.saveEffect();
            end;
        end;
    elseif(ver < TR_III) then
        for k, v in pairs(tr2_flipeffects) do
            if(v.saveEffect ~= nil) then
                ret = ret .. v.saveEffect();
            end;
        end;
    elseif(ver < TR_IV) then
        for k, v in pairs(tr3_flipeffects) do
            if(v.saveEffect ~= nil) then
                ret = ret .. v.saveEffect();
            end;
        end;
    elseif(ver < TR_V) then
        for k, v in pairs(tr4_flipeffects) do
            if(v.saveEffect ~= nil) then
                ret = ret .. v.saveEffect();
            end;
        end;
    else
        for k, v in pairs(tr5_flipeffects) do
            if(v.saveEffect ~= nil) then
                ret = ret .. v.saveEffect();
            end;
        end;
    end;

    return ret;
end;