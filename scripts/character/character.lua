-- OPENTOMB CHARACTER DEFINE FILE
-- by Lwmte, Oct 2014

--------------------------------------------------------------------------------
-- Here we define character options to ease usage of getCharacterOption and
-- setCharacterOption functions.
--------------------------------------------------------------------------------

-------- Lara's model-------
--           .=.
--          | 14|
--           \ / \
--       / |     | \
--  11  / |   7   | \  8
--     /   |     |   \
--     |    =====    |
--  12 |    =====    | 9
--     |   /  0  \   |
--  13 0  /_______\  0 10
--        |  | |  |
--        |1 | |4 |
--        |  | |  |
--        |__| |__|
--        |  | |  |
--        |2 | |5 |
--        |  | |  |
--        |__| |__|
--     3  |__| |__|  6
----------------------------

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
end

