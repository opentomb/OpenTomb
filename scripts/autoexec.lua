dofile("scripts/inventory/item_list.lua")
dofile("scripts/inventory/items.lua");

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

setModelCollisionMapSize(0, 11);            
setModelCollisionMap(0, 0, 0);              -- butt
setModelCollisionMap(0, 1, 7);              -- body
setModelCollisionMap(0, 2, 14);             -- head

setModelCollisionMap(0, 3, 1);              -- greawes
setModelCollisionMap(0, 4, 4);
setModelCollisionMap(0, 5, 11);             -- pauldrons
setModelCollisionMap(0, 6, 8);

setModelCollisionMap(0, 7, 2);              -- boots
setModelCollisionMap(0, 8, 5);
setModelCollisionMap(0, 9, 12);             -- braces
setModelCollisionMap(0, 10, 9);

-- dir[anim = 147, frame = 0, frames = 16]
setAnimCommandTransform(0, 147, 0, 0x00);   -- roll animation smooth fix 
setAnimCommandTransform(0, 146, -1, 0x02);

setAnimCommandTransform(0, 147, 0, 0x00);   -- roll animation smooth fix 
setAnimCommandTransform(0, 146, -1, 0x02);

setAnimCommandTransform(0, 205, 1, 0x00);   -- underwater roll animation smooth fix 
setAnimCommandTransform(0, 205, 0, 0x00);
setAnimCommandTransform(0, 203, -1, 0x02);

--setAnimCommandTransform(0, 207, 13, 0x00);   -- jump roll animation smooth fix 
--setAnimCommandTransform(0, 207, 12, 0x02);

--setAnimCommandTransform(0, 210, 8, 0x00);   -- standing jump roll animation smooth fix 
--setAnimCommandTransform(0, 210, 7, 0x02);

setAnimCommandTransform(0, 212, 10, 0x00);   -- back jump roll animation smooth fix 
setAnimCommandTransform(0, 212, 9, 0x02);
