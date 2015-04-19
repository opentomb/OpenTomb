dofile("scripts/entity/character.lua")
dofile("scripts/inventory/item_list.lua")
dofile("scripts/inventory/item_combine.lua")
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

-- creates map for left and right hands
function setDefaultModelAnimReplaceFlag(m_id)
    setModelAnimReplaceFlag(m_id, 8, 0x01);
    setModelAnimReplaceFlag(m_id, 9, 0x01);
    setModelAnimReplaceFlag(m_id, 10, 0x01);
    setModelAnimReplaceFlag(m_id, 11, 0x01);
    setModelAnimReplaceFlag(m_id, 12, 0x01);
    setModelAnimReplaceFlag(m_id, 13, 0x01);
end

if (getLevelVersion() < TR_II) then
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x02);
    setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- shotgun
    setDefaultModelAnimReplaceFlag(2);
    setModelMeshReplaceFlag(2, 10, 0x01);
    setModelMeshReplaceFlag(2, 13, 0x01);
    setModelMeshReplaceFlag(2, 7, 0x02);

    -- auto pistols
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 13, 0x01);
    setModelMeshReplaceFlag(3, 1, 0x02);
    setModelMeshReplaceFlag(3, 4, 0x02);

    -- UZI
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 13, 0x01);
    setModelMeshReplaceFlag(4, 14, 0x01);
    setModelMeshReplaceFlag(4, 1, 0x02);
    setModelMeshReplaceFlag(4, 4, 0x02);

elseif (getLevelVersion() < TR_III) then
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x02);
    setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 13, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x03);

    -- auto pistols
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 13, 0x01);
    setModelMeshReplaceFlag(4, 1, 0x02);
    setModelMeshReplaceFlag(4, 4, 0x02);

    -- UZI
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 13, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x01);
    setModelMeshReplaceFlag(5, 1, 0x02);
    setModelMeshReplaceFlag(5, 4, 0x02);

    -- M16
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    setModelMeshReplaceFlag(6, 14, 0x03);

    -- grenade launcher
    setDefaultModelAnimReplaceFlag(7);
    setModelMeshReplaceFlag(7, 10, 0x01);
    setModelMeshReplaceFlag(7, 14, 0x03);

    -- harpoon gun
    setDefaultModelAnimReplaceFlag(8);
    setModelMeshReplaceFlag(8, 10, 0x01);
    setModelMeshReplaceFlag(8, 14, 0x03);      -- 0x03 slot mesh model: draws with original mesh

    -- flare
    setModelAnimReplaceFlag(9, 11, 0x01);
    setModelAnimReplaceFlag(9, 12, 0x01);
    setModelAnimReplaceFlag(9, 13, 0x01);
    setModelMeshReplaceFlag(9, 13, 0x01);
elseif (getLevelVersion() < TR_IV) then
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x02);
    setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x03);

    -- magnum
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 4, 0x02);

    -- UZI
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 13, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x01);
    setModelMeshReplaceFlag(5, 1, 0x02);
    setModelMeshReplaceFlag(5, 4, 0x02);

    -- M16
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    setModelMeshReplaceFlag(6, 14, 0x03);

    -- rocket launcher
    setDefaultModelAnimReplaceFlag(7);
    setModelMeshReplaceFlag(7, 10, 0x01);
    setModelMeshReplaceFlag(7, 14, 0x03);

    -- grenade launcher
    setDefaultModelAnimReplaceFlag(8);
    setModelMeshReplaceFlag(8, 10, 0x01);
    setModelMeshReplaceFlag(8, 14, 0x03);

    -- harpoon gun
    setDefaultModelAnimReplaceFlag(9);
    setModelMeshReplaceFlag(9, 10, 0x01);
    setModelMeshReplaceFlag(9, 14, 0x03);

    -- flare
    setModelAnimReplaceFlag(10, 11, 0x01);
    setModelAnimReplaceFlag(10, 12, 0x01);
    setModelAnimReplaceFlag(10, 13, 0x01);
    setModelMeshReplaceFlag(10, 13, 0x01);

elseif (getLevelVersion() < TR_V) then
    -- pistols
    --copyMeshFromModelToModel(1, 13, 1);
    --copyMeshFromModelToModel(1, 13, 4);
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    --setModelMeshReplaceFlag(1, 1, 0x02);
    --setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- UZI
    setDefaultModelAnimReplaceFlag(2);
    setModelMeshReplaceFlag(2, 10, 0x01);
    setModelMeshReplaceFlag(2, 13, 0x01);
    --setModelMeshReplaceFlag(2, 1, 0x02);
    --setModelMeshReplaceFlag(2, 4, 0x02);

    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x03);

    -- crossbow
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 14, 0x03);

    -- grenade launcher
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x03);

    -- magnum
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    --setModelMeshReplaceFlag(6, 4, 0x02);

    -- flare
    setModelAnimReplaceFlag(7, 11, 0x01);
    setModelAnimReplaceFlag(7, 12, 0x01);
    setModelAnimReplaceFlag(7, 13, 0x01);
    setModelMeshReplaceFlag(7, 13, 0x01);
elseif (getLevelVersion() == TR_V) then
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    --setModelMeshReplaceFlag(1, 1, 0x02);
    --setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- UZI
    setDefaultModelAnimReplaceFlag(2);
    setModelMeshReplaceFlag(2, 10, 0x01);
    setModelMeshReplaceFlag(2, 13, 0x01);
    --setModelMeshReplaceFlag(2, 1, 0x02);
    --setModelMeshReplaceFlag(2, 4, 0x02);

    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x03);

    -- crossbow
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 14, 0x03);

    -- grenade launcher
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x03);

    -- magnum
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    --setModelMeshReplaceFlag(6, 4, 0x02);

    -- flare
    setModelAnimReplaceFlag(7, 11, 0x01);
    setModelAnimReplaceFlag(7, 12, 0x01);
    setModelAnimReplaceFlag(7, 13, 0x01);
    setModelMeshReplaceFlag(7, 13, 0x01);
end

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

-- generate UV rotation texture animations for waterfalls in TR4+ versions
if (getLevelVersion() == TR_IV) then
    for i=423, 426, 1 do
        genUVRotateAnimation(i);
    end;
elseif (getLevelVersion() == TR_V) then
    for i=410, 415, 1 do
        genUVRotateAnimation(i);
    end;
end

