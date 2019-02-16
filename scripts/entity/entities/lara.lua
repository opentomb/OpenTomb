print("entity_functions_enemies->entity->lara.lua loaded !");

function Lara_init(id)
    local ver = getLevelVersion();
    local level = getLevel();
    
    if(ver < TR_II) then
        -- check ifis home level
        if(level == 0) then
            setEntityMeshes(id, 5, 0, 13);
        end;
    elseif(ver < TR_III) then
        print("entity_functions_enemies->unknown ! (L36)");
    elseif(ver < TR_IV) then
        setEntityMeshes(id, 315, 0, 14);
    else
        setEntityMeshes(id, 8, 0, 14);
        setEntitySkinMeshes(id, 9, 1, 14);
    end;
    
    resetRigidBodies(id);
    setEntityTypeFlag(id, ENTITY_TYPE_ACTOR, 1);
    setEntityTypeFlag(id, ENTITY_TYPE_TRIGGER_ACTIVATOR, 1);
    characterCreate(id);
    
    setCharacterParam(id, PARAM_HEALTH, 1000.0, 1000.0);
    setCharacterBones(id, 14, 7, 11, 13, 8, 10);  --head, torso, l_hand_first, l_hand_last, r_hand_first, r_hand_last
    setCharacterMoveSizes(id, 768.0, 128.0, 288.0, 1920.0, 320.0); -- height, min_step_up_height, max_step_up_height, max_climb_height, fall_down_height
    
    setEntityMoveType(id, MOVE_ON_FLOOR);
    setCharacterStateControlFunctions(id, STATE_FUNCTIONS_LARA);
    setCharacterKeyAnim(id, ANIM_TYPE_BASE, ANIMATION_KEY_INIT);
    
    setEntityGhostCollisionShape(id, 0,  COLLISION_SHAPE_SPHERE, -60.0, nil, nil, 60.0, nil, nil);
    setEntityGhostCollisionShape(id, 7,  COLLISION_SHAPE_BOX, -48.0, -54.0, 8.0, 48.0, 32.0, 166.0);
    setEntityGhostCollisionShape(id, 1,  COLLISION_SHAPE_BOX, -32.0, -26.1, -176.0, 32.0, 29.1, -2.7);
    setEntityGhostCollisionShape(id, 4,  COLLISION_SHAPE_BOX, -32.0, -27.1, -175.3, 32.0, 28.1, 1.7);
    setEntityGhostCollisionShape(id, 10, COLLISION_SHAPE_SPHERE, -32.0, nil, -52.0, 16.0, nil, 0);
    setEntityGhostCollisionShape(id, 13, COLLISION_SHAPE_SPHERE, -16.0, nil, -52.0, 32.0, nil, 0);
    setEntityGhostCollisionShape(id, 14, COLLISION_SHAPE_SPHERE, -56.0, 0, 0, 56.0, 16.0, 64.0);

    setEntityGhostCollisionShape(id, 3,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 6,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 2,  COLLISION_SHAPE_BOX, -30.0, -40.0, -200.0, 24.0, 16.0, 0);
    setEntityGhostCollisionShape(id, 5,  COLLISION_SHAPE_BOX, -24.0, -40.0, -200.0, 30.0, 16.0, 0);
    setEntityGhostCollisionShape(id, 12, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 9,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 11, COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);
    setEntityGhostCollisionShape(id, 8,  COLLISION_SHAPE_BOX, nil, nil, nil, nil, nil, nil);

    setHumanoidBodyParts(id);
    local m_id = getEntityModelID(id);
    setModelBodyPartFlag(m_id,  0, BODY_PART_BODY_LOW);
    setModelBodyPartFlag(m_id,  7, BODY_PART_BODY_UPPER);
    setModelBodyPartFlag(m_id, 14, BODY_PART_HEAD);

    setModelBodyPartFlag(m_id, 11, BODY_PART_LEFT_HAND_1);
    setModelBodyPartFlag(m_id, 12, BODY_PART_LEFT_HAND_2);
    setModelBodyPartFlag(m_id, 13, BODY_PART_LEFT_HAND_3);
    setModelBodyPartFlag(m_id,  8, BODY_PART_RIGHT_HAND_1);
    setModelBodyPartFlag(m_id,  9, BODY_PART_RIGHT_HAND_2);
    setModelBodyPartFlag(m_id, 10, BODY_PART_RIGHT_HAND_3);

    setModelBodyPartFlag(m_id,  1, BODY_PART_LEFT_LEG_1);
    setModelBodyPartFlag(m_id,  2, BODY_PART_LEFT_LEG_2);
    setModelBodyPartFlag(m_id,  3, BODY_PART_LEFT_LEG_3);
    setModelBodyPartFlag(m_id,  4, BODY_PART_RIGHT_LEG_1);
    setModelBodyPartFlag(m_id,  5, BODY_PART_RIGHT_LEG_2);
    setModelBodyPartFlag(m_id,  6, BODY_PART_RIGHT_LEG_3);
    
    setCharacterRagdollSetup(id, getRagdollSetup(RD_TYPE_LARA));

    setPlayer(id);
    
    entity_funcs[id].onShoot = function(object_id, activator_id)
        setLaraWeaponDamage(id, object_id);

        if(entity_funcs[activator_id].onHit ~= nil) then
            entity_funcs[activator_id].onHit(activator_id, object_id);
        end;
    end;

    entity_funcs[id].onHit = function(object_id, activator_id)
        changeCharacterParam(object_id, PARAM_HEALTH, -getCharacterParam(activator_id, PARAM_HIT_DAMAGE));
    end;
end;

function setLaraWeaponDamage(id, object_id)
    -- need to be changed after the ammo select system is created !
    local weapon = getCharacterCurrentWeapon(object_id);
    local ver = getLevelVersion();
    
    -- Generic Weapon (only one ID in all TR)
    if(weapon == TR_MODEL_PISTOL) then  -- no ammo consume
        setCharacterParam(object_id, PARAM_HIT_DAMAGE, 1, 1);
    elseif((weapon == TR_MODEL_SHOTGUN) and (ver > TR_I_UB)) then
        setCharacterParam(object_id, PARAM_HIT_DAMAGE, 3, 3);
    end;
    
    -- Tomb Raider 1
    if(ver < TR_II) then
        if(weapon == TR1_MODEL_UZI) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 1, 1);
        elseif(weapon == TR1_MODEL_MAGNUM) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 12, 12); -- divised by 2 (revolver and desert eagle deal 21 dmg);
        elseif(weapon == TR1_MODEL_SHOTGUN) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 3, 3);   -- 3 (*6 (5 for wide bullet)) bullet dmg
        end;
    -- Tomb Raider 2
    elseif(ver < TR_III) then
        if(weapon == TR2_MODEL_UZI) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 1, 1);
        elseif(weapon == TR2_MODEL_HARPOONGUN) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 40, 40);
        elseif(weapon == TR2_MODEL_AUTOMAGS) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 12, 12);  -- more firerate than magnum
        elseif(weapon == TR2_MODEL_M16) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 21, 21);  -- deal more dmg but medium firerate
        elseif(weapon == TR2_MODEL_GRENADEGUN) then
			setCharacterParam(object_id, PARAM_HIT_DAMAGE, 20, 20);
		end;
    -- Tomb Raider 3
    elseif(ver < TR_IV) then
        if(weapon == TR3_MODEL_DESERTEAGLE) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 21, 21);  -- low firerate
        elseif(weapon == TR3_MODEL_UZI) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 1, 1);
        elseif(weapon == TR3_MODEL_MP5) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 12, 12);  -- same as M16 but more firerate
        elseif(weapon == TR3_MODEL_ROCKETGUN) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 100, 100);  -- low ammo can be found, low firerate, hard to control.
        elseif(weapon == TR3_MODEL_HARPOONGUN) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 40, 40);
        elseif(weapon == TR3_MODEL_GRENADEGUN) then
			setCharacterParam(object_id, PARAM_HIT_DAMAGE, 20, 20);
		end;
    -- Tomb Raider 4 or Tomb Raider 5
    elseif(ver < TR_V or ver == TR_V) then
        if(weapon == TR4C_MODEL_UZI) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 1, 1);
        elseif(weapon == TR4C_MODEL_CROSSBOW) then    -- can be crossbow or grapplin gun
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 5, 5);  -- 3 dmg for explosives
        elseif(weapon == TR4C_MODEL_GRENADEGUN) then
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 20, 20);
		elseif(weapon == TR4C_MODEL_REVOLVER) then    -- can be revolver or desert eagle
            setCharacterParam(object_id, PARAM_HIT_DAMAGE, 21, 21); -- same for the 2 guns in the game
        end;
    end;
end;