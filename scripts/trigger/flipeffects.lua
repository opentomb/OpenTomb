-- OPENTOMB FLIPEFFECT TRIGGER FUNCTION SCRIPT
-- by Lwmte, July 2015

--------------------------------------------------------------------------------
-- In this file you can put extra flipeffect functions which act in the same way
-- as Flipeffect Editor in TREP - you write the code for each flipeffect
-- yourself. Code can be as complex as for entity script programming; you can
-- define local variables and even subfunctions for each flipeffect, but be sure
-- to create appropriate constructor (onInit) and destructor (onDelete) for them
--------------------------------------------------------------------------------

-- Initialize flipeffect function arrays.

flipeffects     = {};  -- Global array storing ALL flipeffects.

tr1_flipeffects = {};  -- Local version array with links to global array.
tr2_flipeffects = {};
tr3_flipeffects = {};
tr4_flipeffects = {};
tr5_flipeffects = {};

-- Helper function to quickly assign flipeffect to multiple versions.
-- Flipeffect assignation is needed, as different TR versions have different flipeffects stored with
-- the same IDs (for ex., TR2/TR3 uses lots of flipeffects for assault course clock control, while
-- TR4 re-uses them for completely different purpose).

function fe_Assign(ver, fe, id)
    if((fe) and (id) and (ver > 0) and (ver < 5)) then
        if(ver == 1) then tr1_flipeffects[id] = flipeffects[fe] end;
        if(ver == 2) then tr2_flipeffects[id] = flipeffects[fe] end;
        if(ver == 3) then tr3_flipeffects[id] = flipeffects[fe] end;
        if(ver == 4) then tr4_flipeffects[id] = flipeffects[fe] end;
        if(ver == 5) then tr5_flipeffects[id] = flipeffects[fe] end;
    end;
end;


-- FLIPEFFECTS IMPLEMENTATION

flipeffects[1] = {  -- Shake camera

    onExec =
    function(caller, operand)
        if(caller >= 0) then
            camShake(100, 1, caller);
        else
            camShake(100, 1);
        end;
    end;
};
fe_Assign(1, 1,1); fe_Assign(2, 1,1); fe_Assign(3, 1,1); fe_Assign(4, 1,1); fe_Assign(5, 1,1);

flipeffects[3] = {  -- Emit bubble particle

    onExec =
    function(caller, operand)
        if((caller >= 0) and (math.random(100) > 60)) then
            playSound(SOUND_BUBBLE, caller)
        end;
    end;
};
fe_Assign(1, 3,3); fe_Assign(2, 3,3); fe_Assign(3, 3,3); fe_Assign(4, 3,3); fe_Assign(5, 3,3);

flipeffects[10] = { -- Play desired sound ID

    onExec =
    function(caller, operand)
        if(operand < 0) then return end;
        if(caller >= 0) then
            playSound(operand, caller);
        else
            playSound(operand);
        end;
    end;
};
--fe_Assign(1, 10,10); fe_Assign(2, 10,10); -- Not sure if this is used in TR1-2.
fe_Assign(3, 10,10); fe_Assign(4, 10,10); fe_Assign(5, 10,10);

flipeffects[11] = { -- Play explosion sound and effect

    onExec =
    function(caller, operand)
        flashSetup(255, 255,220,80, 10,600);
        flashStart();
        playSound(105);
    end;
};
fe_Assign(1, 11,11); fe_Assign(2, 11,11); fe_Assign(3, 11,11); fe_Assign(4, 11,11); fe_Assign(5, 11,11);

flipeffects[23] = { -- Hide caller

    onExec =
    function(caller, operand)
        if(caller < 0) then return end;
        setEntityVisibility(caller, false);
    end;
};
fe_Assign(1, 23,23); fe_Assign(2, 23,23); fe_Assign(3, 23,23); fe_Assign(4, 23,23); fe_Assign(5, 23,23);

flipeffects[24] = { -- Show caller

    onExec =
    function(caller, operand)
        if(caller < 0) then return end;
        setEntityVisibility(caller, true);
    end;
};
fe_Assign(1, 24,24); fe_Assign(2, 24,24); fe_Assign(3, 24,24); fe_Assign(4, 24,24); fe_Assign(5, 24,24);

flipeffects[32] = { -- Play footprint and overlay it on floor.

    onExec =
    function(caller, operand)
        if(caller < 0) then return end;
        if(getEntitySubstanceState(caller) == 0) then
            local material = getEntitySectorMaterial(caller);
            local sound_id = -1;
            
                if(material == SECTOR_MATERIAL_MUD       ) then sound_id = 288;
            elseif(material == SECTOR_MATERIAL_SAND      ) then sound_id = 291;
            elseif(material == SECTOR_MATERIAL_GRAVEL    ) then sound_id = 290;
            elseif(material == SECTOR_MATERIAL_WOOD      ) then sound_id = 292;
            elseif(material == SECTOR_MATERIAL_METAL     ) then sound_id = 294;
            elseif(material == SECTOR_MATERIAL_GRASS     ) then sound_id = 291;
            elseif(material == SECTOR_MATERIAL_OLDWOOD   ) then sound_id = 292;
            elseif(material == SECTOR_MATERIAL_OLDMETAL  ) then sound_id = 294;
            
            elseif( (material == SECTOR_MATERIAL_WATER   ) or
                    (material == SECTOR_MATERIAL_STONE   ) or
                    (material == SECTOR_MATERIAL_CONCRETE) ) then return;

            elseif(getLevelVersion() ~= TR_IV) then
            
                    if(material == SECTOR_MATERIAL_SNOW) then sound_id = 293;
                elseif(material == SECTOR_MATERIAL_ICE ) then sound_id = 289; end;
                
            elseif(getLevelVersion() == TR_IV) then
                    
                    if(material == SECTOR_MATERIAL_MARBLE) then sound_id = 293; end;
                
            end;
            
            playSound(sound_id, caller);
        end;
    end;
};
fe_Assign(3, 32,32); fe_Assign(4, 32,32); fe_Assign(5, 32,32);



-- Get certain flipeffect array, regarding current version.

function getFlipeffectArray(ver)
    if(ver == TR_I) then
        return tr1_flipeffects;
    elseif(ver == TR_II) then
        return tr2_flipeffects;
    elseif(ver == TR_III) then
        return tr3_flipeffects;
    elseif(ver == TR_IV) then
        return tr4_flipeffects;
    elseif(ver == TR_V) then
        return tr5_flipeffects;
    else
        return nil;
    end;
end;

-- Launch given flipeffect (id) for a given entity (caller) with given argument (operand).

function execFlipeffect(id, caller, operand)
    local flipeffects = getFlipeffectArray(getLevelVersion());
    if((not flipeffects) or (not flipeffects[id]) or (not flipeffects[id].onExec)) then return end;
    
    if(not caller ) then caller  = -1 end;
    if(not operand) then operand = -1 end;
    
    flipeffects[id].onExec(caller, operand);
end;

-- Clean and prepare single effect.

function fe_ClearEffect(flipeffect)
    if(not flipeffect) then return end;
    if(flipeffect.onDelete) then flipeffect.onDelete() end;
end;

function fe_PrepareEffect(flipeffect)
    if(not flipeffect) then return end;
    if(flipeffect.onInit) then flipeffect.onInit() end;
end;

-- Clean up flipeffect temporary variables. Must be called on level end.

function fe_Clear()
    local flipeffects = getFlipeffectArray(getLevelVersion());

    for k,v in pairs(flipeffects) do
        fe_ClearEffect(flipeffects[k]);
    end;
    print("Flipeffect functions cleaned");
end;

-- Initialize flipeffect temporary variables. Must be called on level loading.

function fe_Prepare()
    local flipeffects = getFlipeffectArray(getLevelVersion());

    for k,v in pairs(flipeffects) do
        fe_PrepareEffect(flipeffects[k]);
    end;
    print("Flipeffect functions prepared");
end;
