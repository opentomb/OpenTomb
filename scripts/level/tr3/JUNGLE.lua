
-- Penetration configuration specifies collision type for floor and ceiling
-- sectors (squares).

--TR_PENETRATION_CONFIG_SOLID             0   // Ordinary sector.
--TR_PENETRATION_CONFIG_DOOR_VERTICAL_A   1   // TR3-5 triangulated door.
--TR_PENETRATION_CONFIG_DOOR_VERTICAL_B   2   // TR3-5 triangulated door.
--TR_PENETRATION_CONFIG_WALL              3   // Wall (0x81 == 32512)
--TR_PENETRATION_CONFIG_GHOST             4   // No collision.

--There are two types of diagonal splits - we call them north-east (NE) and
--north-west (NW). In case there is no diagonal in sector (TR1-2 classic sector),
--then NONE type is used.

--TR_SECTOR_DIAGONAL_TYPE_NONE            0
--TR_SECTOR_DIAGONAL_TYPE_NE              1
--TR_SECTOR_DIAGONAL_TYPE_NW              2

-- test sector geometry tuning
function doTuneSector()
    -- (room_id, index_x, index_y, penetration_config, diagonal_type, floor, z0, z1, z2, z3)
    --setSectorFloorConfig(4, 7, 2, nil, nil, nil, 512, 512, 512, 512)
    --setSectorCeilingConfig(4, 7, 2, nil, nil, nil, 2048, 2048, 2048, 2048)
    print("test sectors config");
end
