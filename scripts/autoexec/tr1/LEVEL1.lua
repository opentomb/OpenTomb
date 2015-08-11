function autoexec_PreLoad()
    static_tbl[06] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[08] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[10] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
    static_tbl[33] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
    static_tbl[34] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
    static_tbl[38] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[39] = {coll = COLLISION_TYPE_STATIC, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_TYPE_NONE,   shape = COLLISION_SHAPE_BOX};           -- Icicle
    print("Preload autoexec done (LEVEL1.lua)");
end;

function autoexec_PostLoad()
    playStream(5);
    print("Postload autoexec done");
end;
