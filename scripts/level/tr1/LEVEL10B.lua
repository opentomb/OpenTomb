-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL10B
print("level/tr1/level10b.atlantis->level_loaded !");

level_PostLoad = function()
    playStream(60);
    addRoomToOverlappedList(47, 84);
    addRoomToOverlappedList(84, 47);
    setEntityTypeFlag(0x6E, ENTITY_TYPE_PICKABLE, 0);
    setEntityTypeFlag(0x6E, ENTITY_TYPE_GENERIC, 1);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[06] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[08] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
    static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
    static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
    static_tbl[38] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
end;


function ScionHolder_init(id)

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivationOffset(id, -660.0, 2048.0, 128.0, 256.0);
    setEntityActivationDirection(id, 1.0, 0.0, 0.0, 0.77);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((activator_id == nil) or (not canTriggerEntity(activator_id, object_id))) then
            return ENTITY_TRIGGERING_NOT_READY;
        end;

        if(0 == getEntityModelID(activator_id, ANIM_TYPE_BASE)) then
            entity_funcs[id].activator_id = activator_id;
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            setEntityBaseAnimModel(activator_id, 5);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 0, 0);
            noEntityMove(activator_id, true);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((entity_funcs[id].activator_id ~= nil) and (5 == getEntityModelID(entity_funcs[id].activator_id, ANIM_TYPE_BASE))) then
            local a, f, c = getEntityAnim(entity_funcs[id].activator_id, ANIM_TYPE_BASE);
            if((a == 0) and (f + 1 >= c)) then
                gameflowSend(GF_OP_LEVELCOMPLETE, getLevel() + 1);
            end;
        end;
    end;
end;