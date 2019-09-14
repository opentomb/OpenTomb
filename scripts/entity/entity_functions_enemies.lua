-- OPENTOMB ENTITY FUNCTIONS SCRIPT
-- By TeslaRus, Lwmte, 2014-2016
-- Reworked by TokyoSU, 2019
print("entity_functions_enemies->loading...");

function setHumanoidBodyParts(id)
    setEntityBodyPartFlag(id,  0, BODY_PART_BODY_LOW);
    setEntityBodyPartFlag(id,  7, BODY_PART_BODY_UPPER);
    setEntityBodyPartFlag(id, 14, BODY_PART_HEAD);

    setEntityBodyPartFlag(id, 11, BODY_PART_LEFT_HAND_1);
    setEntityBodyPartFlag(id, 12, BODY_PART_LEFT_HAND_2);
    setEntityBodyPartFlag(id, 13, BODY_PART_LEFT_HAND_3);
    setEntityBodyPartFlag(id,  8, BODY_PART_RIGHT_HAND_1);
    setEntityBodyPartFlag(id,  9, BODY_PART_RIGHT_HAND_2);
    setEntityBodyPartFlag(id, 10, BODY_PART_RIGHT_HAND_3);

    setEntityBodyPartFlag(id,  1, BODY_PART_LEFT_LEG_1);
    setEntityBodyPartFlag(id,  2, BODY_PART_LEFT_LEG_2);
    setEntityBodyPartFlag(id,  3, BODY_PART_LEFT_LEG_3);
    setEntityBodyPartFlag(id,  4, BODY_PART_RIGHT_LEG_1);
    setEntityBodyPartFlag(id,  5, BODY_PART_RIGHT_LEG_2);
    setEntityBodyPartFlag(id,  6, BODY_PART_RIGHT_LEG_3);
end;

--------------------------------------------
--      Tomb Raider Default Function      --
--------------------------------------------

dofile(base_path .. "scripts/entity/entities/constants.lua");
dofile(base_path .. "scripts/entity/entities/basic.lua");
dofile(base_path .. "scripts/entity/entities/lara.lua");

--------------------------------------
--      Tomb Raider 1 Entities      --
--------------------------------------
-- animals & monster
dofile(base_path .. "scripts/entity/entities/tr1/tr1_lara_mutant.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_dog.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_bat.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_rat.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_crocodile.lua");     -- IA not finished for water!
dofile(base_path .. "scripts/entity/entities/tr1/tr1_bear.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_raptor.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_trex.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_lion.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_gorilla.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_puma.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_winged_mutant.lua");
dofile(base_path .. "scripts/entity/entities/tr1/tr1_centaur.lua");

-- characters
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_larson.lua");
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_pierre.lua");
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_cowboy.lua");
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_mrt.lua");
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_skateboardist.lua");
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_natla.lua");
dofile(base_path .. "scripts/entity/entities/tr1/characters/tr1_torsoboss.lua");

-- other
dofile(base_path .. "scripts/entity/entities/tr1/tr1_spawner.lua");

--------------------------------------
--      Tomb Raider 2 Entities      --
--------------------------------------
-- animals & monster
dofile(base_path .. "scripts/entity/entities/tr2/tr2_tiger.lua");

print("entity_functions_enemies->loaded success !");