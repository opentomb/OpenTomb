-- OPENTOMB FLIPEFFECT TRIGGER FUNCTION SCRIPT
-- by Lwmte, April 2015

--------------------------------------------------------------------------------
-- In this file you can put extra flipeffect functions which act in the same way
-- as Flipeffect Editor in TREP - all flipeffects above 47 are hardcoded, and
-- remaining ones are refering to this function table. Only difference is, if
-- there IS flipeffect above 47 in the table, then table function is used
-- instead of hardcoded one; it is needed for some version-specific flipeffects.
--------------------------------------------------------------------------------

flipeffects = {};   -- Initialize flipeffect function array.

-- Flipeffects implementation

-- Shake camera
flipeffects[1] = {};
flipeffects[1].doEffect = function(parameter) 
    camShake(15, 1);
end;

-- Play desired sound ID
flipeffects[10] = {};
flipeffects[10].doEffect = function(parameter) 
    playSound(parameter);
end;

-- Play explosion sound and effect
flipeffects[11] = {};
flipeffects[11].doEffect = function(parameter)
    flashSetup(255, 255,220,80, 10,600);
    flashStart();
    playSound(105);
end;

-- room flickering effect
flipeffects[16] = {};
flipeffects[16].doEffect = function(parameter)
    if((flipeffects[16].state == nil) and ((parameter == true) or (getFlipState(0) == 1))) then
        flipeffects[16].timer = 4.0;
        flipeffects[16].state = 0;
        addTask(function()
            if(flipeffects[16].state == 0) then
                flipeffects[16].timer = flipeffects[16].timer - frame_time;
                if(flipeffects[16].timer <= 0.0) then
                    flipeffects[16].timer = 0.2;
                    flipeffects[16].state = 1;
                    setFlipState(0x00, 0);
                end;
            elseif(flipeffects[16].state == 1) then
                flipeffects[16].timer = flipeffects[16].timer - frame_time;
                if(flipeffects[16].timer <= 0.0) then
                    flipeffects[16].timer = 1.0;
                    flipeffects[16].state = 2;
                    setFlipState(0x00, 1);
                end;
            elseif(flipeffects[16].state == 2) then
                flipeffects[16].timer = flipeffects[16].timer - frame_time;
                if(flipeffects[16].timer <= 0.0) then
                    flipeffects[16].timer = 0.0;
                    setFlipState(0x00, 0);
                    flipeffects[16].state = nil;
                    return false;
                end;
            else
                setFlipState(0x00, 0);
                flipeffects[16].state = nil;
                return false;
            end;
            
            return true;
        end);
    else
        flipeffects[16].state = nil;
        setFlipState(0x00, 0);
    end;
end;

flipeffects[16].saveEffect = function()
    print("save effect 16");
    local ret = "";
    if(flipeffects[16].state ~= nil) then
        ret = "\nflipeffects[16].doEffect(true);";
        ret = ret .. "\nflipeffects[16].state = " .. flipeffects[16].state .. ";";
        ret = ret .. "\nflipeffects[16].timer = " .. flipeffects[16].timer .. ";";
        ret = ret .. "\nsetFlipState(0x00, " .. getFlipState(0) .. ");";
    end;
    return ret;
end;