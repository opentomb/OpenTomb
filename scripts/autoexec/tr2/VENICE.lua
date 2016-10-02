function autoexec_PreLoad()
    static_tbl[00] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Pillar 1
    static_tbl[01] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Pillar 2
    static_tbl[02] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Pillar 3
    static_tbl[03] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Pillar 4
    static_tbl[04] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Pillar 5
    static_tbl[05] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Vault
    static_tbl[06] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Tree
    static_tbl[10] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Chain
    static_tbl[13] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Square hole
    static_tbl[14] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Window tent
    static_tbl[15] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Smoke pipe
    static_tbl[16] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Fence
    static_tbl[20] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Water pole top
    static_tbl[21] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Water pole bottom
    static_tbl[22] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Water pole middle
    static_tbl[30] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Chandelier
    static_tbl[31] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Square pillar
    static_tbl[32] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Square pillar 2
    static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Horizontal block
    static_tbl[34] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Wall light
    static_tbl[35] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Stone fence
    static_tbl[36] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Wooden fence
    static_tbl[37] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Broken window
    static_tbl[38] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Normal window
    static_tbl[39] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Grated fence
    static_tbl[40] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Pipes
    static_tbl[41] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Furnace
    static_tbl[43] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Wooden barrier
    static_tbl[44] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Barbed wire
    static_tbl[45] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Books on the floor
    static_tbl[46] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Curtain
    static_tbl[47] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};   -- Chair
    static_tbl[48] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};       -- Fireplace
    static_tbl[49] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};       -- Flags 
end;

function autoexec_PostLoad()
    addCharacterHair(player, HAIR_TR2);
end;