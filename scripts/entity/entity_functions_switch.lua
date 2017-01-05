-- OPENTOMB ADDITIONAL SWITCH SCRIPT
-- by TeslaRus, Lwmte - May 2014

--------------------------------------------------------------------------------
-- Switch script is used to define switch entity begaviour, which assigns
-- certain Lara's animations to them. Note that switch entities includes not
-- only switches, but also keyholes, puzzleholes and pickups. Also this script
-- is used to assign puzzlehole meshswaps.
--------------------------------------------------------------------------------

tr1_switches = {};
tr2_switches = {};
tr3_switches = {};
tr4_switches = {};
tr5_switches = {};

tr1_puzzlehole_meshswap = {};
tr2_puzzlehole_meshswap = {};
tr3_puzzlehole_meshswap = {};
tr4_puzzlehole_meshswap = {};
tr5_puzzlehole_meshswap = {};

-- trX_switches[<model_id>] = {on = {ready_anim = a, trig_anim = b, actor_anim = c}, off = {ready_anim = a, trig_anim = b, actor_anim = c}};

-- LARA actor animations:
-- 63 - switch down (on)
-- 64 - switch up (off)
-- 129 - underwater switch toggle
-- 130 - underwater pick up
-- 131 - keyhole key
-- 134 - keyhole puzzle
-- 135 - stand pick up
-- 195 - little switch down (on)
-- 196 - little switch op (off)
-- 197 - press button
-- 214 - kick
-- 215 - slide down on rope


-- GLOBAL SWITCH PATTERNS (valid for all TR versions)

tr_anim_puzzlehole =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 134, switch_frame = 79 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_keyhole =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 131, switch_frame = 109},
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_bigwallswitch =
{
    on  = {ready_anim =  0, trig_anim = 2, actor_anim = 63,  switch_frame = 36 },
    off = {ready_anim =  1, trig_anim = 3, actor_anim = 64,  switch_frame = 33 }
};

tr_anim_smallwallswitch =
{
    on  = {ready_anim =  0, trig_anim = 2, actor_anim = 195, switch_frame = 50 },
    off = {ready_anim =  1, trig_anim = 3, actor_anim = 196, switch_frame = 50 }
};

tr_anim_smallbutton =
{
    on  = {ready_anim =  0, trig_anim = 2, actor_anim = 197, switch_frame = 24 },
    off = {ready_anim =  1, trig_anim = 3, actor_anim = 197, switch_frame = 24 }
};

tr_anim_uwswitch =
{
    on  = {ready_anim =  0, trig_anim = 2, actor_anim = 129, switch_frame = 85},
    off = {ready_anim =  1, trig_anim = 3, actor_anim = 129, switch_frame = 85}
};

tr_anim_hole =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 325, switch_frame = 40 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_bigbutton =
{
    on  = {ready_anim =  0, trig_anim = 1, actor_anim = 413, switch_frame = 35 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_bigswitch =
{
    on  = {ready_anim =  0, trig_anim = 1, actor_anim = 324, switch_frame = 80 },
    off = {ready_anim =  2, trig_anim = 3, actor_anim = 324, switch_frame = 80  }
};

tr_anim_valve =
{
    on  = {ready_anim =  0, trig_anim = 1, actor_anim = 470, switch_frame = 30 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_pickup_stand =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 135, switch_frame = 14 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_pickup_underwater =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 130, switch_frame = 15 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_pickup_crouch =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 291, switch_frame = 15 },
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};

tr_anim_pickup_crawl =
{
    on  = {ready_anim = -1, trig_anim = 0, actor_anim = 292, switch_frame = 15 },    -- CHECK: TR3 possibly have less keyframes.
    off = {ready_anim =  0, trig_anim = 0, actor_anim = 0  }
};



tr1_puzzlehole_meshswap[118] = 122;
tr1_puzzlehole_meshswap[119] = 123;
tr1_puzzlehole_meshswap[120] = 124;
tr1_puzzlehole_meshswap[121] = 125;

tr2_puzzlehole_meshswap[182] = 186;
tr2_puzzlehole_meshswap[183] = 187;
tr2_puzzlehole_meshswap[184] = 188;
tr2_puzzlehole_meshswap[185] = 189;

tr3_puzzlehole_meshswap[213] = 217;
tr3_puzzlehole_meshswap[214] = 218;
tr3_puzzlehole_meshswap[215] = 219;
tr3_puzzlehole_meshswap[216] = 220;

tr4_puzzlehole_meshswap[260] = 272;
tr4_puzzlehole_meshswap[261] = 273;
tr4_puzzlehole_meshswap[262] = 274;
tr4_puzzlehole_meshswap[263] = 275;
tr4_puzzlehole_meshswap[264] = 276;
tr4_puzzlehole_meshswap[265] = 277;
tr4_puzzlehole_meshswap[266] = 278;
tr4_puzzlehole_meshswap[267] = 279;
tr4_puzzlehole_meshswap[268] = 280;
tr4_puzzlehole_meshswap[269] = 281;
tr4_puzzlehole_meshswap[270] = 282;
tr4_puzzlehole_meshswap[271] = 283;

tr5_puzzlehole_meshswap[242] = 250;
tr5_puzzlehole_meshswap[243] = 251;
tr5_puzzlehole_meshswap[244] = 252;
tr5_puzzlehole_meshswap[245] = 253;
tr5_puzzlehole_meshswap[246] = 254;
tr5_puzzlehole_meshswap[247] = 255;
tr5_puzzlehole_meshswap[248] = 256;
tr5_puzzlehole_meshswap[249] = 257;


tr1_switches[55]  = tr_anim_bigwallswitch;
tr2_switches[104] = tr_anim_bigwallswitch;
tr3_switches[129] = tr_anim_bigwallswitch;
tr4_switches[306] = tr_anim_bigwallswitch;
tr4_switches[318] = tr_anim_bigswitch;

tr1_switches[56]  = tr_anim_uwswitch;
tr2_switches[105] = tr_anim_uwswitch;
tr3_switches[130] = tr_anim_uwswitch;

tr2_switches[93]  = tr_anim_smallwallswitch;
tr3_switches[118] = tr_anim_smallwallswitch;

tr2_switches[103] = tr_anim_smallbutton;
tr3_switches[128] = tr_anim_smallbutton;

tr1_switches[118] = tr_anim_puzzlehole;
tr1_switches[119] = tr_anim_puzzlehole;
tr1_switches[120] = tr_anim_puzzlehole;
tr1_switches[121] = tr_anim_puzzlehole;

tr2_switches[182] = tr_anim_puzzlehole;
tr2_switches[183] = tr_anim_puzzlehole;
tr2_switches[184] = tr_anim_puzzlehole;
tr2_switches[185] = tr_anim_puzzlehole;

tr3_switches[213] = tr_anim_puzzlehole;
tr3_switches[214] = tr_anim_puzzlehole;
tr3_switches[215] = tr_anim_puzzlehole;
tr3_switches[216] = tr_anim_puzzlehole;

tr4_switches[260] = tr_anim_puzzlehole;
tr4_switches[261] = tr_anim_puzzlehole;
tr4_switches[262] = tr_anim_puzzlehole;
tr4_switches[263] = tr_anim_puzzlehole;
tr4_switches[264] = tr_anim_puzzlehole;
tr4_switches[265] = tr_anim_puzzlehole;
tr4_switches[266] = tr_anim_puzzlehole;
tr4_switches[267] = tr_anim_puzzlehole;
tr4_switches[268] = tr_anim_puzzlehole;
tr4_switches[269] = tr_anim_puzzlehole;
tr4_switches[270] = tr_anim_puzzlehole;
tr4_switches[271] = tr_anim_puzzlehole;

tr5_switches[242] = tr_anim_puzzlehole;
tr5_switches[243] = tr_anim_puzzlehole;
tr5_switches[244] = tr_anim_puzzlehole;
tr5_switches[245] = tr_anim_puzzlehole;
tr5_switches[246] = tr_anim_puzzlehole;
tr5_switches[247] = tr_anim_puzzlehole;
tr5_switches[248] = tr_anim_puzzlehole;
tr5_switches[249] = tr_anim_puzzlehole;

tr1_switches[137] = tr_anim_keyhole;
tr1_switches[138] = tr_anim_keyhole;
tr1_switches[139] = tr_anim_keyhole;
tr1_switches[140] = tr_anim_keyhole;

tr2_switches[201] = tr_anim_keyhole;
tr2_switches[202] = tr_anim_keyhole;
tr2_switches[203] = tr_anim_keyhole;
tr2_switches[204] = tr_anim_keyhole;

tr3_switches[232] = tr_anim_keyhole;
tr3_switches[233] = tr_anim_keyhole;
tr3_switches[234] = tr_anim_keyhole;
tr3_switches[235] = tr_anim_keyhole;

tr4_switches[284] = tr_anim_keyhole;
tr4_switches[285] = tr_anim_keyhole;
tr4_switches[286] = tr_anim_keyhole;
tr4_switches[287] = tr_anim_keyhole;
tr4_switches[288] = tr_anim_keyhole;
tr4_switches[289] = tr_anim_keyhole;
tr4_switches[290] = tr_anim_keyhole;
tr4_switches[291] = tr_anim_keyhole;
tr4_switches[292] = tr_anim_keyhole;
tr4_switches[293] = tr_anim_keyhole;
tr4_switches[294] = tr_anim_keyhole;
tr4_switches[295] = tr_anim_keyhole;

tr5_switches[258] = tr_anim_keyhole;
tr5_switches[259] = tr_anim_keyhole;
tr5_switches[260] = tr_anim_keyhole;
tr5_switches[261] = tr_anim_keyhole;
tr5_switches[262] = tr_anim_keyhole;
tr5_switches[263] = tr_anim_keyhole;
tr5_switches[264] = tr_anim_keyhole;
tr5_switches[265] = tr_anim_keyhole;


function set_activation_data(id)
    local m_id = getEntityModelID(id);
    if(m_id == nil or m_id < 0) then
        return 0;
    end

    entity_funcs[id].on  = {};
    entity_funcs[id].off = {};
    entity_funcs[id].key = nil;
    entity_funcs[id].meshswap = nil;

    if(getLevelVersion() < TR_II) then
        if(tr1_switches[m_id] ~= nil) then
            entity_funcs[id].on       = tr1_switches[m_id].on;
            entity_funcs[id].off      = tr1_switches[m_id].off;
            entity_funcs[id].key      = tr1_key[m_id];
            entity_funcs[id].meshswap = tr1_puzzlehole_meshswap[m_id];
        else
            return 0;
        end
    elseif(getLevelVersion() < TR_III) then
        if(tr2_switches[m_id] ~= nil) then
            entity_funcs[id].on       = tr2_switches[m_id].on;
            entity_funcs[id].off      = tr2_switches[m_id].off;
            entity_funcs[id].key      = tr2_key[m_id];
            entity_funcs[id].meshswap = tr2_puzzlehole_meshswap[m_id];
        else
            return 0;
        end
    elseif(getLevelVersion() < TR_IV) then
        if(tr3_switches[m_id] ~= nil) then
            entity_funcs[id].on       = tr3_switches[m_id].on;
            entity_funcs[id].off      = tr3_switches[m_id].off;
            entity_funcs[id].key      = tr3_key[m_id];
            entity_funcs[id].meshswap = tr3_puzzlehole_meshswap[m_id];
        else
            return 0;
        end
    elseif(getLevelVersion() < TR_V) then
        if(tr4_switches[m_id] ~= nil) then
            entity_funcs[id].on       = tr4_switches[m_id].on;
            entity_funcs[id].off      = tr4_switches[m_id].off;
            entity_funcs[id].key      = tr4_key[m_id];
            entity_funcs[id].meshswap = tr4_puzzlehole_meshswap[m_id];
        else
            return 0;
        end
    else
        if(tr5_switches[m_id] == nil) then
            if(m_id >= 266 and m_id <= 274) then
                local ocb = getEntityOCB(object_id);
                if(ocb == 0) then
                    entity_funcs[id].on  = tr_anim_bigwallswitch.on;
                    entity_funcs[id].off = tr_anim_bigwallswitch.off;
                elseif(ocb == 1 or ocb == 2) then
                    entity_funcs[id].on  = tr_anim_hole.on;
                    entity_funcs[id].off = tr_anim_hole.off;
                elseif(ocb == 3) then
                    entity_funcs[id].on  = tr_anim_bigbutton.on;
                    entity_funcs[id].off = tr_anim_bigbutton.off;
                elseif(ocb == 4) then
                    entity_funcs[id].on  = tr_anim_smallwallswitch.on;
                    entity_funcs[id].off = tr_anim_smallwallswitch.off;
                elseif(ocb == 5) then
                    entity_funcs[id].on  = tr_anim_smallbutton.on;
                    entity_funcs[id].off = tr_anim_smallbutton.off;
                elseif(ocb >= 6 and ocb <= 10) then
                    entity_funcs[id].on  = tr_anim_valve.on;
                    entity_funcs[id].off = tr_anim_valve.off;
                elseif(ocb > 10) then
                    entity_funcs[id].on  = {ready_anim =  0, trig_anim = 2, actor_anim = ocb};
                    entity_funcs[id].off = {ready_anim =  1, trig_anim = 3, actor_anim = ocb + 1};
                end
            else
                return 0;
            end
        else
            entity_funcs[id].on       = tr5_switches[m_id].on;
            entity_funcs[id].off      = tr5_switches[m_id].off;
            entity_funcs[id].key      = tr5_key[m_id];
            entity_funcs[id].meshswap = tr5_puzzlehole_meshswap[m_id];
        end
    end
end;


function set_activation_offset(id)
    local model_id = getEntityModelID(id);
    local dx = 0.0;
    local dy = 360.0;
    local dz = 0.0;
    local  r = 128.0;

    if(getLevelVersion() < TR_II) then
        if(model_id == 56) then
            dy = 0.0;
            r = 160.0;
        end
    elseif(getLevelVersion() < TR_III) then
        if(model_id == 105) then
            dy = 0.0;
            r = 160.0;
        end
    elseif(getLevelVersion() < TR_IV) then
        if(model_id == 130) then
            dy = 0.0;
            r = 160.0;
        end
    end

    setEntityActivationOffset(id, dx, dy, dz, r);
end


function keyhole_init(id)    -- Key and puzzle holes

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, false);
    set_activation_offset(id);
    set_activation_data(id);
    entity_funcs[id].state = 0; -- 0 == idle, 1 - enabling;
    entity_funcs[id].activator_id = -1;

    entity_funcs[id].onSave = function()
        local addr = "\nentity_funcs[" .. id .. "].";
        local ret = addr .. "state = " .. entity_funcs[id].state .. ";";
        ret = ret .. addr .. "activator_id = " .. entity_funcs[id].activator_id .. ";";
        return ret;
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (getEntityActivity(object_id))) then
            return ENTITY_TRIGGERING_NOT_READY;
        end
        
        local t = getEntityAnim(activator_id, ANIM_TYPE_BASE);
        if(entity_funcs[object_id].on.ready_anim < 0 or entity_funcs[object_id].on.ready_anim == t) then
            if(entity_funcs[object_id].key ~= nil) then
                if(getItemsCount(activator_id, entity_funcs[object_id].key) <= 0) then
                    if(getActionChange(act.action) == 0) then
                        playSound(SOUND_NO);
                    end;
                    return ENTITY_TRIGGERING_NOT_READY;
                else
                    removeItem(activator_id, entity_funcs[object_id].key, 1);
                end;
            end;

            setEntityAnim(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].on.trig_anim, 0);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, entity_funcs[object_id].on.actor_anim, 0);
            noFixEntityCollision(activator_id);
            setEntityActivity(object_id, true);
            entity_funcs[object_id].activator_id = activator_id;
            entity_funcs[object_id].state = 1;
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;

        return ENTITY_TRIGGERING_NOT_READY;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(entity_funcs[object_id].state == 1) then
            local a, f, c = getEntityAnim(entity_funcs[object_id].activator_id, ANIM_TYPE_BASE);
            if(entity_funcs[object_id].on.switch_frame ~= nil) then
                c = entity_funcs[object_id].on.switch_frame;
            end
            if(f >= c - 1) then                                 -- check the end of animation
                if(entity_funcs[object_id].key ~= nil) then
                    if(entity_funcs[object_id].meshswap ~= nil) then
                        setEntityMeshswap(object_id, entity_funcs[object_id].meshswap); -- only for puzzleholes - do a meshswap
                    end
                    setEntityLock(object_id, 1);                -- lock filled hole
                end;
                setEntityActivity(object_id, false);
                entity_funcs[object_id].state = 0;
            end;
        end;
    end;
end

function switch_init(id)     -- Ordinary switches
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    set_activation_offset(id);
    set_activation_data(id);
    entity_funcs[id].state = 0; -- 0 == idle, 1 - enabling, 2 - disabling;

    entity_funcs[id].onSave = function()
        local addr = "\nentity_funcs[" .. id .. "].";
        return addr .. "state = " .. entity_funcs[id].state .. ";";
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (getEntityTimer(object_id) > 0.0) or (entity_funcs[object_id].state > 0)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end

        local t = getEntityAnim(object_id, ANIM_TYPE_BASE);
        local ret = 0;
        if(entity_funcs[object_id].on.ready_anim < 0 or entity_funcs[object_id].on.ready_anim == t) then
            ret = 1;
            setEntityAnim(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].on.trig_anim, 0);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, entity_funcs[object_id].on.actor_anim, 0);
            noFixEntityCollision(activator_id);
            setEntityActivity(object_id, true);
            entity_funcs[object_id].state = 1;
        elseif(entity_funcs[object_id].off.ready_anim == t and getEntityLock(object_id) ~= 1) then  -- Locked switches doesn't flip back!
            ret = 2;
            setEntityAnim(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].off.trig_anim, 0);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, entity_funcs[object_id].off.actor_anim, 0);
            setEntityActivity(object_id, true);
            noFixEntityCollision(activator_id);
            entity_funcs[object_id].state = 2;
        end;

        if(ret > 0) then
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((tick_state == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, 1);
            setEntitySectorStatus(object_id, 1);
            setEntityEvent(object_id, 0);
        end;

        if(entity_funcs[object_id].state == 1) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(entity_funcs[object_id].on.switch_frame ~= nil) then
                c = entity_funcs[object_id].on.switch_frame;
            end
            if(f >= c - 1) then                             -- check the end of animation
                setEntitySectorStatus(object_id, 1);        -- only for switches - turn on
                setEntityEvent(object_id, 1);
                entity_funcs[object_id].state = 0;
            end;
        elseif(entity_funcs[object_id].state == 2) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(entity_funcs[object_id].off.switch_frame ~= nil) then
                c = entity_funcs[object_id].off.switch_frame;
            end
            if(f >= c - 1) then                             -- check the end of animation
                setEntitySectorStatus(object_id, 1);        -- only for switches - turn off
                setEntityEvent(object_id, 0);
                entity_funcs[object_id].state = 0;
            end
        end;
    end;
end


function lever_switch_init(id)     -- Big switches (TR4) - lever
    
    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivationOffset(id, 0.0, -520.0, 0.0, 128);
    set_activation_data(id);
    entity_funcs[id].state = 0; -- 0 == idle, 1 - enabling, 2 - disabling;

    entity_funcs[id].onSave = function()
        local addr = "\nentity_funcs[" .. id .. "].";
        return addr .. "state = " .. entity_funcs[id].state .. ";";
    end;

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (getEntityTimer(object_id) > 0.0) or (entity_funcs[object_id].state > 0)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end

        local t = getEntityAnim(object_id, ANIM_TYPE_BASE);
        local ret = 0;
        if(entity_funcs[object_id].on.ready_anim < 0 or entity_funcs[object_id].on.ready_anim == t) then
            ret = 1;
            setEntityAnim(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].on.trig_anim, 0);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, entity_funcs[object_id].on.actor_anim, 0);
            noFixEntityCollision(activator_id);
            setEntityActivity(object_id, true);
            entity_funcs[object_id].state = 1;
        elseif(entity_funcs[object_id].off.ready_anim == t and getEntityLock(object_id) ~= 1) then  -- Locked switches doesn't flip back!
            ret = 2;
            setEntityAnim(object_id, ANIM_TYPE_BASE, entity_funcs[object_id].off.trig_anim, 0);
            setEntityAnim(activator_id, ANIM_TYPE_BASE, entity_funcs[object_id].off.actor_anim, 0);
            setEntityActivity(object_id, true);
            noFixEntityCollision(activator_id);
            entity_funcs[object_id].state = 2;
        end;

        if(ret > 0) then
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;
    
    entity_funcs[id].onLoop = function(object_id, tick_state)
        if((tick_state == TICK_STOPPED) and (getEntityEvent(object_id) ~= 0)) then
            setEntityAnimStateHeavy(object_id, ANIM_TYPE_BASE, 1);
            setEntitySectorStatus(object_id, 1);
            setEntityEvent(object_id, 0);
        end;

        if(entity_funcs[object_id].state == 1) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(entity_funcs[object_id].on.switch_frame ~= nil) then
                c = entity_funcs[object_id].on.switch_frame;
            end
            if(f >= c - 1) then                             -- check the end of animation
                setEntitySectorStatus(object_id, 1);        -- only for switches - turn on
                setEntityEvent(object_id, 1);
                setEntityActivationDirection(object_id, 0.0, -1.0, 0.0);
                setEntityActivationOffset(object_id, 0.0, 520.0, 0.0, 128);
                entity_funcs[object_id].state = 0;
            end;
        elseif(entity_funcs[object_id].state == 2) then
            local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
            if(entity_funcs[object_id].off.switch_frame ~= nil) then
                c = entity_funcs[object_id].off.switch_frame;
            end
            if(f >= c - 1) then                             -- check the end of animation
                setEntitySectorStatus(object_id, 1);        -- only for switches - turn off
                setEntityEvent(object_id, 0);
                setEntityActivationDirection(object_id, 0.0, 1.0, 0.0);
                setEntityActivationOffset(object_id, 0.0, -520.0, 0.0, 128);
                entity_funcs[object_id].state = 0;
            end
        end;
    end;
end


function WheelKnob_init(id)   -- Bulkdoors (TR2)

    setEntityTypeFlag(id, ENTITY_TYPE_INTERACTIVE);
    setEntityActivity(id, false);

    setEntityActivationDirection(id, 0.0, 1.0, 0.0, 0.70);
    setEntityActivationOffset(id, 0.0, 256.0, 0.0, 128.0);

    -- enable anims replacing from model 12
    setModelAnimReplaceFlag(12, 0, 0x01);
    setModelAnimReplaceFlag(12, 1, 0x01);
    setModelAnimReplaceFlag(12, 2, 0x01);
    setModelAnimReplaceFlag(12, 3, 0x01);
    setModelAnimReplaceFlag(12, 4, 0x01);
    setModelAnimReplaceFlag(12, 5, 0x01);
    setModelAnimReplaceFlag(12, 6, 0x01);
    setModelAnimReplaceFlag(12, 7, 0x01);
    setModelAnimReplaceFlag(12, 8, 0x01);
    setModelAnimReplaceFlag(12, 9, 0x01);
    setModelAnimReplaceFlag(12, 10, 0x01);
    setModelAnimReplaceFlag(12, 11, 0x01);
    setModelAnimReplaceFlag(12, 12, 0x01);
    setModelAnimReplaceFlag(12, 13, 0x01);
    setModelAnimReplaceFlag(12, 14, 0x01);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        if((object_id == nil) or (activator_id == nil)) then
            return ENTITY_TRIGGERING_NOT_READY;
        end;

        local a = getEntityAnim(object_id, ANIM_TYPE_BASE);
        if(a == 0) then
            setEntityActivity(object_id, true);
            entityRotateToTriggerZ(activator_id, object_id);
            entityMoveToTriggerActivationPoint(activator_id, object_id);
            setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
            setEntitySectorStatus(object_id, 1);                                -- it is a switch

            entitySSAnimEnsureExists(activator_id, ANIM_TYPE_MISK_1, 12);
            setEntityAnim(activator_id, ANIM_TYPE_MISK_1, 2, 0);
            entitySSAnimSetEnable(activator_id, ANIM_TYPE_MISK_1, 1);
            entitySSAnimSetEnable(activator_id, ANIM_TYPE_BASE, 0);
            return ENTITY_TRIGGERING_ACTIVATED;
        end;
        return ENTITY_TRIGGERING_NOT_READY;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(entitySSAnimGetEnable(player, ANIM_TYPE_MISK_1)) then
            local a, f, c = getEntityAnim(player, ANIM_TYPE_MISK_1);
            if((a == 2) and (f + 1 >= c)) then
                entitySSAnimSetEnable(player, ANIM_TYPE_MISK_1, 0);
                entitySSAnimSetEnable(player, ANIM_TYPE_BASE, 1);
            end;
        end;
    end;
end