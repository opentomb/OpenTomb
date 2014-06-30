-- OPENTOMB TRIGGER SCRIPT
-- FOR TOMB RAIDER 1, LEVEL2 - 

-- disable / enable entities
-- Bats
setEntityActivity(1, 0);
setEntityActivity(2, 0);
setEntityActivity(3, 0);
setEntityActivity(11, 0);
setEntityActivity(31, 0);
setEntityActivity(32, 0);
setEntityActivity(49, 0);
setEntityWisibility(1, 0);
setEntityWisibility(2, 0);
setEntityWisibility(3, 0);
setEntityWisibility(11, 0);
setEntityWisibility(31, 0);
setEntityWisibility(32, 0);
setEntityWisibility(49, 0);

-- Wolves
setEntityActivity(26, 0);
setEntityActivity(27, 0);
setEntityActivity(45, 0);
setEntityActivity(46, 0);
setEntityActivity(53, 0);
setEntityActivity(57, 0);
setEntityWisibility(26, 0);
setEntityWisibility(27, 0);
setEntityWisibility(45, 0);
setEntityWisibility(46, 0);
setEntityWisibility(53, 0);
setEntityWisibility(57, 0);

-- Bear
setEntityActivity(30, 0);
setEntityWisibility(30, 0);

-- Doors
setEntityActivity(9, 0);
setEntityActivity(12, 0);
setEntityActivity(13, 0);
setEntityActivity(28, 0);
setEntityActivity(35, 0);
setEntityActivity(36, 0);
setEntityActivity(43, 0);
setEntityActivity(44, 0);

-- Collapsible floor
setEntityActivity(50, 0);
setEntityActivity(51, 0);


-- auto open doors
function activate_doors(object_id, activator_id)
    if(object_id == nil or getEntityActivity(object_id) >= 1) then
        return;
    end
    setEntityAnim(object_id, 3, 0);
    setEntityActivity(object_id, 1);
end

--woolves
function activate_wolves(object_id, activator_id)
    if(object_id == nil or getEntityActivity(object_id) >= 1) then
        return;
    end
    setEntityAnim(object_id, 1, 0);
    setEntityActivity(object_id, 1);
	setEntityWisibility(object_id, 1);
end

-- bats
function activate_bats(object_id, activator_id)
    if(object_id == nil or getEntityActivity(object_id) >= 1) then
        return;
    end
    setEntityAnim(object_id, 0, 0);
    setEntityActivity(object_id, 1);
	setEntityWisibility(object_id, 1);
end

-- bear
function activate_bear(object_id, activator_id)
    if(object_id == nil or getEntityActivity(object_id) >= 1) then
        return;
    end
    setEntityAnim(object_id, 8, 0);
    setEntityActivity(object_id, 1);
	setEntityWisibility(object_id, 1);
end

-- floor
function activate_floor(object_id, activator_id)
    if(object_id == nil or getEntityActivity(object_id) >= 1) then
        return;
    end
    setEntityAnim(object_id, 1, 0);
    setEntityActivity(object_id, 1);
end

entity_funcs = {};
entity_funcs[9] = {};  entity_funcs[9].activate  = activate_doors;
entity_funcs[12] = {}; entity_funcs[12].activate = activate_doors;
entity_funcs[13] = {}; entity_funcs[13].activate = activate_doors;
entity_funcs[28] = {}; entity_funcs[28].activate = activate_doors;
entity_funcs[35] = {}; entity_funcs[35].activate = activate_doors;
entity_funcs[36] = {}; entity_funcs[36].activate = activate_doors;
entity_funcs[43] = {}; entity_funcs[43].activate = activate_doors;
entity_funcs[44] = {}; entity_funcs[44].activate = activate_doors;

entity_funcs[50] = {}; entity_funcs[50].activate = activate_floor;
entity_funcs[51] = {}; entity_funcs[51].activate = activate_floor;

entity_funcs[1] = {};  entity_funcs[1].activate  = activate_bats;
entity_funcs[2] = {};  entity_funcs[2].activate  = activate_bats;
entity_funcs[3] = {};  entity_funcs[3].activate  = activate_bats;
entity_funcs[11] = {}; entity_funcs[11].activate = activate_bats;
entity_funcs[31] = {}; entity_funcs[31].activate = activate_bats;
entity_funcs[32] = {}; entity_funcs[32].activate = activate_bats;
entity_funcs[49] = {}; entity_funcs[49].activate = activate_bats;

entity_funcs[26] = {};  entity_funcs[26].activate  = activate_wolves;
entity_funcs[27] = {};  entity_funcs[27].activate  = activate_wolves;
entity_funcs[45] = {};  entity_funcs[45].activate  = activate_wolves;
entity_funcs[46] = {};  entity_funcs[46].activate  = activate_wolves;
entity_funcs[53] = {};  entity_funcs[53].activate  = activate_wolves;
entity_funcs[57] = {};  entity_funcs[57].activate  = activate_wolves;

entity_funcs[30] = {}; entity_funcs[30].activate = activate_bear;

function activateEntity(object_id, activator_id)
    if((activator_id == nil) or (object_id == nil)) then
        return;
    end

    if(entity_funcs[object_id] ~= nil) then
        entity_funcs[object_id].activate(object_id, activator_id);
    end
end


