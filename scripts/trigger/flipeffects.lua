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
        if(ver == Engine.I) then tr1_flipeffects[id] = flipeffects[fe] end;
        if(ver == Engine.II) then tr2_flipeffects[id] = flipeffects[fe] end;
        if(ver == Engine.III) then tr3_flipeffects[id] = flipeffects[fe] end;
        if(ver == Engine.IV) then tr4_flipeffects[id] = flipeffects[fe] end;
        if(ver == Engine.V) then tr5_flipeffects[id] = flipeffects[fe] end;
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
fe_Assign(Engine.I, 1,1); fe_Assign(Engine.II, 1,1); fe_Assign(Engine.III, 1,1); fe_Assign(Engine.IV, 1,1); fe_Assign(Engine.V, 1,1);

flipeffects[3] = {  -- Emit bubble particle

    onExec =
    function(caller, operand)
        if((caller >= 0) and (math.random(100) > 60)) then
            playSound(SoundId.Bubble, caller)
        end;
    end;
};
fe_Assign(Engine.I, 3,3); fe_Assign(Engine.II, 3,3); fe_Assign(Engine.III, 3,3); fe_Assign(Engine.IV, 3,3); fe_Assign(Engine.V, 3,3);

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
--fe_Assign(Engine.I, 10,10); fe_Assign(Engine.II, 10,10); -- Not sure if this is used in TR1-2.
fe_Assign(Engine.III, 10,10); fe_Assign(Engine.IV, 10,10); fe_Assign(Engine.V, 10,10);

flipeffects[11] = { -- Play explosion sound and effect

    onExec =
    function(caller, operand)
        flashSetup(255, 255,220,80, 10,600);
        flashStart();
        playSound(SoundId.Eplosion);
    end;
};
fe_Assign(Engine.I, 11,11); fe_Assign(Engine.II, 11,11); fe_Assign(Engine.III, 11,11); fe_Assign(Engine.IV, 11,11); fe_Assign(Engine.V, 11,11);

flipeffects[23] = { -- Hide caller

    onExec =
    function(caller, operand)
        if(caller < 0) then return end;
        setEntityVisibility(caller, false);
    end;
};
fe_Assign(Engine.I, 23,23); fe_Assign(Engine.II, 23,23); fe_Assign(Engine.III, 23,23); fe_Assign(Engine.IV, 23,23); fe_Assign(Engine.V, 23,23);

flipeffects[24] = { -- Show caller

    onExec =
    function(caller, operand)
        if(caller < 0) then return end;
        setEntityVisibility(caller, true);
    end;
};
fe_Assign(Engine.I, 24,24); fe_Assign(Engine.II, 24,24); fe_Assign(Engine.III, 24,24); fe_Assign(Engine.IV, 24,24); fe_Assign(Engine.V, 24,24);

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

            elseif(getEngineVersion() ~= Engine.IV) then
            
                    if(material == SECTOR_MATERIAL_SNOW) then sound_id = 293;
                elseif(material == SECTOR_MATERIAL_ICE ) then sound_id = 289; end;
                
            elseif(getEngineVersion() == Engine.V) then
                    
                    if(material == SECTOR_MATERIAL_MARBLE) then sound_id = 293; end;
                
            end;
            
            playSound(sound_id, caller);
        end;
    end;
};
fe_Assign(Engine.III, 32,32); fe_Assign(Engine.IV, 32,32); fe_Assign(Engine.V, 32,32);



-- Get certain flipeffect array, regarding current version.

function getFlipeffectArray(ver)
    if(ver == Engine.I) then
        return tr1_flipeffects;
    elseif(ver == Engine.II) then
        return tr2_flipeffects;
    elseif(ver == Engine.III) then
        return tr3_flipeffects;
    elseif(ver == Engine.IV) then
        return tr4_flipeffects;
    elseif(ver == Engine.V) then
        return tr5_flipeffects;
    else
        return nil;
    end;
end;

-- Launch given flipeffect (id) for a given entity (caller) with given argument (operand).

function execFlipeffect(id, caller, operand)
    local flipeffects = getFlipeffectArray(getEngineVersion());
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
    local flipeffects = getFlipeffectArray(getEngineVersion());

    for k,v in pairs(flipeffects) do
        fe_ClearEffect(flipeffects[k]);
    end;
    print("Flipeffect functions cleaned");
end;

-- Initialize flipeffect temporary variables. Must be called on level loading.

function fe_Prepare()
    local flipeffects = getFlipeffectArray(getEngineVersion());

    for k,v in pairs(flipeffects) do
        fe_PrepareEffect(flipeffects[k]);
    end;
    print("Flipeffect functions prepared");
end;
