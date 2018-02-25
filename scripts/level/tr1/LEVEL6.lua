-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL6

print("Level script loaded (LEVEL6.lua)");

level_PostLoad = function()
    moveEntityLocal(player, 0, 0, 256);
    setEntityMoveType(player, MOVE_UNDERWATER);
    setEntityAnim(player, ANIM_TYPE_BASE, 108, 0);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[06] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[08] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
    static_tbl[22] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Grass
    static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
    static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
    static_tbl[38] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
end;


function midastouch_init(id)    -- Midas gold touch
    --enable Midas death anim
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);

    setEntityActivationOffset(id, -640.0, 0.0, -512.0, 128.0);
    setEntityActivationDirection(id, 1.0, 0.0, 0.0, 0.87);
    entity_funcs[id].activator_id = nil;

    entity_funcs[id].onSave = function()
        if(entity_funcs[id].activator_id ~= nil) then
            local addr = "\nentity_funcs[" .. id .. "].";
            return addr .. "activator_id = " .. entity_funcs[id].activator_id .. ";";
        end;
        return "";
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if(activator_id == nil) then
            return ENTITY_TRIGGERING_NOT_READY;
        end

        if((0 == getEntityModelID(activator_id, ANIM_TYPE_BASE)) and (getItemsCount(activator_id, 100) > 0)) then
            entity_funcs[object_id].activator_id = activator_id;
            entityRotateToTriggerZ(activator_id, object_id);
            setEntityBaseAnimModel(activator_id, 5);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, 0, 0);
            noEntityMove(activator_id, true);
        end;
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local activator_id = entity_funcs[object_id].activator_id;
        if(getEntityDistance(activator_id, object_id) < 1024.0) then
            local lara_anim, frame, count = getEntityAnim(activator_id, ANIM_TYPE_BASE);
            local lara_sector = getEntitySectorIndex(activator_id);             -- do not use getEntitySectorIndex: DEPRECATED!
            local hand_sector = getEntitySectorIndex(object_id);

            if(5 == getEntityModelID(activator_id, ANIM_TYPE_BASE)) then
                local a, f, c = getEntityAnim(activator_id, ANIM_TYPE_BASE);
                if((a == 0) and (f + 1 >= c)) then
                    setEntityBaseAnimModel(activator_id, 0);
                    setEntityAnim(activator_id, ANIM_TYPE_BASE, 103, 0);
                    removeItem(activator_id, 100, 1);
                    addItem(activator_id, ITEM_PUZZLE_1, 1);
                    noEntityMove(activator_id, false);
                end;
            end;

            if((lara_sector == hand_sector) and (getEntityMoveType(activator_id) == MOVE_ON_FLOOR) and (lara_anim ~= 50)) then
                setCharacterParam(activator_id, PARAM_HEALTH, 0);
                setEntityBaseAnimModel(activator_id, 5);
                setEntityAnim(activator_id, ANIM_TYPE_BASE, 1, 0);
                disableEntity(object_id);
            end;
        end;
    end
end
