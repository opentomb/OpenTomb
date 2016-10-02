function autoexec_PreLoad()
    static_tbl[00] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};          -- Tree 1
    static_tbl[01] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Tree 2
    static_tbl[02] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Hanging plant
    static_tbl[03] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Hanging plant (tilt)
    static_tbl[05] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};          -- Tree 3
    static_tbl[06] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Tree 4
    static_tbl[07] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Grass
    static_tbl[08] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};          -- Tree 5
    static_tbl[11] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Cockpit
    static_tbl[12] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Turret
    static_tbl[16] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Ray
    static_tbl[17] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Rays
    static_tbl[18] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Control panel 1
    static_tbl[19] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Control panel 2
    static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX};          -- Fence
    static_tbl[34] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Wall light
    static_tbl[35] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Wheel and ropes
    static_tbl[36] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Ropes
    static_tbl[37] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Ropes (horizontal)
    static_tbl[38] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_BOX, hide = true};-- Dummy cube
    static_tbl[39] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Ring
    static_tbl[40] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Cockpit 2
    static_tbl[41] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Control panel 3
    static_tbl[42] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Control panel 4
    static_tbl[48] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};      -- Bed
    static_tbl[49] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};          -- Rope
end;

function autoexec_PostLoad()
    addCharacterHair(player, HAIR_TR3);
    playStream(33);
end;