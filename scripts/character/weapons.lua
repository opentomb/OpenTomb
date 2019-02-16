print("autoexec.lua->weapons loaded !");
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

-- 0x00: no overriding / adding;
-- 0x01: overriding mesh in armed state;
-- 0x02: add mesh to slot in armed state;
-- 0x03: overriding mesh in disarmed state;
-- 0x04: add mesh to slot in disarmed state;

-- creates map for left and right hands
function setDefaultModelAnimReplaceFlag(m_id)
    setModelAnimReplaceFlag(m_id, 8, 0x01);
    setModelAnimReplaceFlag(m_id, 9, 0x01);
    setModelAnimReplaceFlag(m_id, 10, 0x01);
    setModelAnimReplaceFlag(m_id, 11, 0x01);
    setModelAnimReplaceFlag(m_id, 12, 0x01);
    setModelAnimReplaceFlag(m_id, 13, 0x01);
end;

function enableModelAnimReplaceFlags(m_id, copy_meshes_back)
    local meshes_count = getModelMeshCount(m_id);
    local m = 0;

    while(m < meshes_count) do
        setModelAnimReplaceFlag(m_id, m, 0x01);
        if(copy_meshes_back) then
            copyMeshFromModelToModel(m_id, 0, m, m);
        end;
        m = m + 1;
    end;
end;

local ver = getLevelVersion();

if (ver < TR_II) then
    enableModelAnimReplaceFlags(5, false);
	
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x01);
    setModelMeshReplaceFlag(1, 4, 0x01);
	
    -- shotgun
    setDefaultModelAnimReplaceFlag(2);
    setModelMeshReplaceFlag(2, 10, 0x01);
    setModelMeshReplaceFlag(2, 13, 0x01);
    setModelMeshReplaceFlag(2, 7, 0x01);

    -- magnum
    setDefaultModelAnimReplaceFlag(3);
    copyModelAnimations(3, 1);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 13, 0x01);
    setModelMeshReplaceFlag(3, 1, 0x01);
    setModelMeshReplaceFlag(3, 4, 0x01);
	
    -- uzi
    setDefaultModelAnimReplaceFlag(4);
    copyModelAnimations(4, 1);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 13, 0x01);
    setModelMeshReplaceFlag(4, 14, 0x01);
    setModelMeshReplaceFlag(4, 1, 0x01);
    setModelMeshReplaceFlag(4, 4, 0x01);

elseif (ver < TR_III) then
    enableModelAnimReplaceFlags(12, true);
    
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x01);
    setModelMeshReplaceFlag(1, 4, 0x01);
    
    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x02);
    
    -- auto pistols
    setDefaultModelAnimReplaceFlag(4);
    copyModelAnimations(4, 1);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 13, 0x01);
    setModelMeshReplaceFlag(4, 1, 0x01);
    setModelMeshReplaceFlag(4, 4, 0x01);

    -- uzi
    setDefaultModelAnimReplaceFlag(5);
    copyModelAnimations(5, 1);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 13, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x01);
    setModelMeshReplaceFlag(5, 1, 0x01);
    setModelMeshReplaceFlag(5, 4, 0x01);

    -- m16
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    setModelMeshReplaceFlag(6, 14, 0x02);
    -- m16 override model anim (movement)
    setModelAnimReplaceFlag(6, 7,  0x01);  -- needed for animation (weapon)
    setModelAnimReplaceFlag(6, 14, 0x01);  -- needed for animation (weapon)
    
    -- grenade launcher
    setDefaultModelAnimReplaceFlag(7);
    setModelMeshReplaceFlag(7, 10, 0x01);
    setModelMeshReplaceFlag(7, 14, 0x02);

    -- harpoon gun
    setDefaultModelAnimReplaceFlag(8);
    setModelMeshReplaceFlag(8, 10, 0x01);
    setModelMeshReplaceFlag(8, 14, 0x02);      -- 0x02 slot mesh model: draws with original mesh
	
    -- flare
    setModelAnimReplaceFlag(9, 11, 0x01);
    setModelAnimReplaceFlag(9, 12, 0x01);
    setModelAnimReplaceFlag(9, 13, 0x01);
    setModelMeshReplaceFlag(9, 13, 0x01);
elseif (ver < TR_IV) then
    -- pistols
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x01);
    setModelMeshReplaceFlag(1, 4, 0x01);
    
    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x02);
	
    -- desert eagle
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 4, 0x01);

    -- uzi
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 13, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x01);
    setModelMeshReplaceFlag(5, 1, 0x01);
    setModelMeshReplaceFlag(5, 4, 0x01);

    -- mp5
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    setModelMeshReplaceFlag(6, 14, 0x02);
    -- mp5 override model anim (movement)
    setModelAnimReplaceFlag(6, 7,  0x01);  -- needed for animation
    setModelAnimReplaceFlag(6, 14, 0x01);  -- needed for animation (weapon)
    
    -- rocket launcher
    setDefaultModelAnimReplaceFlag(7);
    setModelMeshReplaceFlag(7, 10, 0x01);
    setModelMeshReplaceFlag(7, 14, 0x02);

    -- grenade launcher
    setDefaultModelAnimReplaceFlag(8);
    setModelMeshReplaceFlag(8, 10, 0x01);
    setModelMeshReplaceFlag(8, 14, 0x02);

    -- harpoon gun
    setDefaultModelAnimReplaceFlag(9);
    setModelMeshReplaceFlag(9, 10, 0x01);
    setModelMeshReplaceFlag(9, 14, 0x02);

    -- flare
    setModelAnimReplaceFlag(10, 11, 0x01);
    setModelAnimReplaceFlag(10, 12, 0x01);
    setModelAnimReplaceFlag(10, 13, 0x01);
    setModelMeshReplaceFlag(10, 13, 0x01);

elseif (ver < TR_V) then
    -- pistols
    copyMeshFromModelToModel(1, 14, 1, 4);
    copyMeshFromModelToModel(1, 14, 4, 8);
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x02);
    setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- uzi
    copyMeshFromModelToModel(2, 15, 1, 4);
    copyMeshFromModelToModel(2, 15, 4, 8);
    setDefaultModelAnimReplaceFlag(2);
    setModelMeshReplaceFlag(2, 10, 0x01);
    setModelMeshReplaceFlag(2, 13, 0x01);
    setModelMeshReplaceFlag(2, 1, 0x02);
    setModelMeshReplaceFlag(2, 4, 0x02);

    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x02);

    -- crossbow
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 14, 0x02);
    -- grenade launcher
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x02);

    -- revolver
    copyMeshFromModelToModel(6, 16, 4, 8);
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    setModelMeshReplaceFlag(6, 4, 0x02);

    -- flare
    setModelAnimReplaceFlag(7, 11, 0x01);
    setModelAnimReplaceFlag(7, 12, 0x01);
    setModelAnimReplaceFlag(7, 13, 0x01);
    setModelMeshReplaceFlag(7, 13, 0x01);
elseif(ver == TR_V) then
	local lvl = getLevel();
    -- pistols
    copyMeshFromModelToModel(1, 14, 1, 4);
    copyMeshFromModelToModel(1, 14, 4, 8);
    setDefaultModelAnimReplaceFlag(1);
    setModelMeshReplaceFlag(1, 10, 0x01);
    setModelMeshReplaceFlag(1, 13, 0x01);
    setModelMeshReplaceFlag(1, 1, 0x02);
    setModelMeshReplaceFlag(1, 4, 0x02);
    
    -- uzi
    copyMeshFromModelToModel(2, 15, 1, 4);
    copyMeshFromModelToModel(2, 15, 4, 8);
    setDefaultModelAnimReplaceFlag(2);
    setModelMeshReplaceFlag(2, 10, 0x01);
    setModelMeshReplaceFlag(2, 13, 0x01);
    setModelMeshReplaceFlag(2, 1, 0x02);
    setModelMeshReplaceFlag(2, 4, 0x02);

    -- shotgun
    setDefaultModelAnimReplaceFlag(3);
    setModelMeshReplaceFlag(3, 10, 0x01);
    setModelMeshReplaceFlag(3, 14, 0x02);

    -- grappling gun
    setDefaultModelAnimReplaceFlag(4);
    setModelMeshReplaceFlag(4, 10, 0x01);
    setModelMeshReplaceFlag(4, 14, 0x02);
    -- the firing animation are empty !!!

	-- mp5
    setDefaultModelAnimReplaceFlag(5);
    setModelMeshReplaceFlag(5, 10, 0x01);
    setModelMeshReplaceFlag(5, 14, 0x02);
	-- mp5 override model anim (movement)
    setModelAnimReplaceFlag(5, 7,  0x01);  -- needed for animation
    setModelAnimReplaceFlag(5, 14, 0x01);  -- needed for animation (weapon)

    -- revolver / desert eagle
    copyMeshFromModelToModel(6, 16, 4, 8);
    setDefaultModelAnimReplaceFlag(6);
    setModelMeshReplaceFlag(6, 10, 0x01);
    setModelMeshReplaceFlag(6, 4, 0x02);

    -- flare
    setModelAnimReplaceFlag(7, 11, 0x01);
    setModelAnimReplaceFlag(7, 12, 0x01);
    setModelAnimReplaceFlag(7, 13, 0x01);
    setModelMeshReplaceFlag(7, 13, 0x01);
end;