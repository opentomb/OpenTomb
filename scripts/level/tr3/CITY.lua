-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 3, CITY.TR2

print("Level script loaded (CITY.lua)");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR3));
    playStream(26);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Floor lamp
    static_tbl[11] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Lamp upper part
    static_tbl[12] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Floodlight
    static_tbl[13] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Rays
    static_tbl[14] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Tinnos wall-mounted urn
    static_tbl[15] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Statue
    static_tbl[16] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Stone structure
    static_tbl[18] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Tiki
    static_tbl[19] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};        -- Wooden slant
    static_tbl[20] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Crystal formation
    static_tbl[30] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Metallic structure
    static_tbl[31] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Puzzle tile set 1
    static_tbl[32] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Puzzle tile set 2
    static_tbl[33] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Puzzle tile set 3
    static_tbl[34] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Puzzle tile set 4
    static_tbl[35] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};            -- Puzzle tile set 5
    static_tbl[36] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Tinnos statue
    static_tbl[37] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Tinnos idol
    static_tbl[38] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Fire pedestal
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_BOX};            -- Thin pillar
end;
