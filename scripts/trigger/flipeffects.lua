-- OPENTOMB FLIPEFFECT TRIGGER FUNCTION SCRIPT
-- by Lwmte, April 2015

--------------------------------------------------------------------------------
-- In this file you can put extra flipeffect functions which act in the same way
-- as Flipeffect Editor in TREP - all flipeffects above 47 are hardcoded, and
-- remaining ones are refering to this function table. Only difference is, if
-- there IS flipeffect above 47 in the table, then table function is used
-- instead of hardcoded one; it is needed for some version-specific flipeffects.
--------------------------------------------------------------------------------

-- Initialize flipeffect function arrays.

tr1_flipeffects = {};
tr2_flipeffects = {};
tr3_flipeffects = {};
tr4_flipeffects = {};
tr5_flipeffects = {};


-- TR4 Flipeffects implementation

-- Shake camera
tr4_flipeffects[1] = {

    onExec =
    function(caller, operand)
        if(caller >= 0) then
            local x,y,z = getEntityPos(caller);
            camShake(15, 1, x, y, z);
        else
            camShake(15, 1);
        end;
    end;
};

-- Play desired sound ID
tr4_flipeffects[10] = {

    onExec =
    function(caller, operand)
        if(caller >= 0) then
            playSound(operand, caller);
        else
            playSound(operand);
        end;
    end;
};

-- Play explosion sound and effect
tr4_flipeffects[11] = {

    onExec =
    function(caller, operand)
        flashSetup(255, 255,220,80, 10,600);
        flashStart();
        playSound(105);
    end;
};



function getFlipeffectArray(ver)
    if(ver < TR_II) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        return tr1_flipeffects;
    elseif(ver < TR_III) then                -- TR_II, TR_II_DEMO
        return tr2_flipeffects;
    elseif(ver < TR_IV) then
        return tr3_flipeffects;
    elseif(ver < TR_V) then                -- TR_IV, TR_IV_DEMO
        return tr4_flipeffects;
    elseif(ver < TR_UNKNOWN) then                -- TR_V
        return tr5_flipeffects;
    else
        return nil;
    end;
end;

function execFlipeffect(id, caller, operand)
    local flipeffects = getFlipeffectArray(getLevelVersion());
    if((not flipeffects) or (not flipeffects[id]) or (not flipeffects[id].onExec)) then return end;
    
    if(not caller)  then caller  = -1 end;
    if(not operand) then operand = -1 end;
    
    flipeffects[id].onExec(caller, operand);
end;

function fe_Clear(flipeffect)
    if(not flipeffect) then return end;
    if(flipeffect.onDelete) then flipeffect.onDelete() end;
end;

function fe_Init(flipeffect)
    if(not flipeffect) then return end;
    if(flipeffect.onInit) then flipeffect.onInit() end;
end;

-- Clean up flipeffect temporary variables. Must be called on level end.

function fe_ClearAll(ver)
    local flipeffects = getFlipeffectArray(ver);

    for k,v in pairs(flipeffects) do
        fe_Clear(flipeffects[k]);
    end;
    print("Flipeffect function table cleaned");
end;

-- Initialize flipeffect temporary variables. Must be called on level loading.

function fe_InitAll(ver)
    local flipeffects = getFlipeffectArray(ver);

    for k,v in pairs(flipeffects) do
        fe_Init(flipeffects[k]);
    end;
    print("Flipeffect function table initialized");
end;