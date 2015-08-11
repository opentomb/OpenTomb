-- OPENTOMB ENTITY MODEL OVERRIDE SCRIPT
-- by TeslaRus

--------------------------------------------------------------------------------
-- The purpose of this script is to replace certain in-game models with another
-- ones. This is needed for early TR versions, which had sprites instead of
-- in-level items.
--------------------------------------------------------------------------------

print("model ID override script loaded");

--------------------------------------------------------------------------------
----------------------------- TR_I, TR_I_DEMO, TR_I_UB -------------------------
--------------------------------------------------------------------------------
tr1_id_override = {};

-- ITEMS
tr1_id_override[93] = 108;              -- BIG MEDKIT
tr1_id_override[94] = 109;              -- SMALL MEDKIT

-- WEAPONS
tr1_id_override[84] = 99;               -- PISTOLS
tr1_id_override[85] = 100;              -- SHOTGUN
tr1_id_override[86] = 101;              -- DESERT EAGLES
tr1_id_override[87] = 102;              -- UZI

-- AMMO
tr1_id_override[88] = 103;              -- Infinite pistol ammo, never on the map (Who knows?..)
tr1_id_override[89] = 104;              -- SHOTGUN
tr1_id_override[90] = 105;              -- DESERT EAGLES
tr1_id_override[91] = 106;              -- UZI

-- QUAES ITEMS / KEYS
tr1_id_override[141] = 145;             -- MAIN ARTEFACT
tr1_id_override[142] = 146;             -- MAIN ARTEFACT
tr1_id_override[143] = 150;             -- MAIN ARTEFACT
tr1_id_override[144] = 150;             -- MAIN ARTEFACT
tr1_id_override[110] = 114;             -- keys and others
tr1_id_override[111] = 115;
tr1_id_override[112] = 116;
tr1_id_override[113] = 117;
tr1_id_override[126] = 127;             --
tr1_id_override[129] = 133;             --
tr1_id_override[130] = 134;             --
tr1_id_override[131] = 135;             --
tr1_id_override[132] = 136;             --

--------------------------------------------------------------------------------
--------------------------------- TR_II, TR_II_DEMO ----------------------------
--------------------------------------------------------------------------------
tr2_id_override = {};

-- ITEMS
tr2_id_override[149] = 171;             -- SMALL MEDKIT
tr2_id_override[150] = 172;             -- BIG MEDKIT
tr2_id_override[151] = 173;             -- FLARES

-- WEAPONS
tr2_id_override[135] = 157;             -- PISTOLS
tr2_id_override[136] = 158;             -- SHOTGUN
tr2_id_override[137] = 159;             -- AUTOMAGS
tr2_id_override[138] = 160;             -- UZI
tr2_id_override[139] = 161;             -- GARPOON
tr2_id_override[140] = 162;             -- M16
tr2_id_override[141] = 163;             -- GRENADE GUN

-- AMMO
tr2_id_override[143] = 165;             -- SHOTGUN
tr2_id_override[144] = 166;             -- DESERT EAGLES
tr2_id_override[145] = 167;             -- UZI
tr2_id_override[146] = 168;             -- GARPOON
tr2_id_override[147] = 169;             -- M16
tr2_id_override[148] = 170;             -- GRENADE GUN

-- QUEST
tr2_id_override[174] = 178;
tr2_id_override[175] = 179;
tr2_id_override[176] = 180;
tr2_id_override[177] = 181;
tr2_id_override[193] = 197;
tr2_id_override[194] = 198;
tr2_id_override[195] = 199;
tr2_id_override[196] = 200;


-- In TR2, some entities have animations borrowed from another entities, so for
-- this purpose, we define animation override table.

tr2_anim_override = {};

tr2_anim_override[17] = 16;             -- Masked goon (white mask, vest)
tr2_anim_override[18] = 16;             -- Masked goon (black mask)


--------------------------------------------------------------------------------
-------------------------------------- TR_III ----------------------------------
--------------------------------------------------------------------------------
--tr3_id_override = {};

--------------------------------------------------------------------------------
--------------------------------- TR_IV, TR_IV_DEMO ----------------------------
--------------------------------------------------------------------------------
--tr4_id_override = {};

--------------------------------------------------------------------------------
------------------------------------- TR_V -------------------------------------
--------------------------------------------------------------------------------
--tr5_id_override = {};

function getOverridedAnim(ver, id)
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        if((tr1_anim_override == nil) or (tr1_anim_override[id] == nil)) then
            return -1;
        else
            return tr1_anim_override[id];
        end;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        if((tr2_anim_override == nil) or (tr2_anim_override[id] == nil)) then
            return -1;
        else
            return tr2_anim_override[id];
        end
    elseif(ver < 6) then                -- TR_III
        if((tr3_anim_override == nil) or (tr3_anim_override[id] == nil)) then
            return -1;
        else
            return tr3_anim_override[id];
        end
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        if((tr4_anim_override == nil) or (tr4_anim_override[id] == nil)) then
            return -1;
        else
            return tr4_anim_override[id];
        end
    elseif(ver < 9) then                -- TR_V
        if((tr5_anim_override == nil) or (tr5_anim_override[id] == nil)) then
            return -1;
        else
            return tr5_anim_override[id];
        end
    else
        return -1;
    end;
end;

function getOverridedID(ver, id)
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        if(tr1_id_override[id] == nil) then
            return -1;
        else
            return tr1_id_override[id];
        end;
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        if(tr2_id_override[id] == nil) then
            return -1;
        else
            return tr2_id_override[id];
        end
    elseif(ver < 6) then                -- TR_III
        if(tr3_id_override[id] == nil) then
            return -1;
        else
            return tr3_id_override[id];
        end
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        if(tr4_id_override[id] == nil) then
            return -1;
        else
            return tr4_id_override[id];
        end
    elseif(ver < 9) then                -- TR_V
        if(tr5_id_override[id] == nil) then
            return -1;
        else
            return tr5_id_override[id];
        end
    else
        return -1;
    end;
end;
