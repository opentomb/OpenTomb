-- OPENTOMB ENTITY SWITCH SCRIPT
-- by TeslaRus, Lwmte - May 2014

--------------------------------------------------------------------------------
-- Switch script is used to assign certain Lara's animations to certain entities
-- which counts as switches, keyholes, puzzleholes and pickups. Also it is used
-- to assign puzzleholes' meshswaps.
--------------------------------------------------------------------------------

tr1_triggers = {};
tr2_triggers = {};
tr3_triggers = {};
tr4_triggers = {};
tr5_triggers = {};

tr1_puzzlehole_meshswap = {};
tr2_puzzlehole_meshswap = {};
tr3_puzzlehole_meshswap = {};
tr4_puzzlehole_meshswap = {};
tr5_puzzlehole_meshswap = {};

-- trX_triggers[<model_id>] = {on = {ready_anim = a, trig_anim = b, actor_anim = c}, off = {ready_anim = a, trig_anim = b, actor_anim = c}};

-- LARA:
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


tr1_triggers[55]  = tr_anim_bigwallswitch;
tr2_triggers[104] = tr_anim_bigwallswitch;
tr3_triggers[129] = tr_anim_bigwallswitch;

tr1_triggers[56]  = tr_anim_uwswitch;
tr2_triggers[105] = tr_anim_uwswitch;
tr3_triggers[130] = tr_anim_uwswitch;

tr2_triggers[93]  = tr_anim_smallwallswitch;
tr3_triggers[118] = tr_anim_smallwallswitch;

tr2_triggers[103] = tr_anim_smallbutton;
tr3_triggers[128] = tr_anim_smallbutton;

tr1_triggers[118] = tr_anim_puzzlehole;
tr1_triggers[119] = tr_anim_puzzlehole;
tr1_triggers[120] = tr_anim_puzzlehole;
tr1_triggers[121] = tr_anim_puzzlehole;

tr2_triggers[182] = tr_anim_puzzlehole;
tr2_triggers[183] = tr_anim_puzzlehole;
tr2_triggers[184] = tr_anim_puzzlehole;
tr2_triggers[185] = tr_anim_puzzlehole;

tr3_triggers[213] = tr_anim_puzzlehole;
tr3_triggers[214] = tr_anim_puzzlehole;
tr3_triggers[215] = tr_anim_puzzlehole;
tr3_triggers[216] = tr_anim_puzzlehole;

tr4_triggers[260] = tr_anim_puzzlehole;
tr4_triggers[261] = tr_anim_puzzlehole;
tr4_triggers[262] = tr_anim_puzzlehole;
tr4_triggers[263] = tr_anim_puzzlehole;
tr4_triggers[264] = tr_anim_puzzlehole;
tr4_triggers[265] = tr_anim_puzzlehole;
tr4_triggers[266] = tr_anim_puzzlehole;
tr4_triggers[267] = tr_anim_puzzlehole;
tr4_triggers[268] = tr_anim_puzzlehole;
tr4_triggers[269] = tr_anim_puzzlehole;
tr4_triggers[270] = tr_anim_puzzlehole;
tr4_triggers[271] = tr_anim_puzzlehole;

tr5_triggers[242] = tr_anim_puzzlehole;
tr5_triggers[243] = tr_anim_puzzlehole;
tr5_triggers[244] = tr_anim_puzzlehole;
tr5_triggers[245] = tr_anim_puzzlehole;
tr5_triggers[246] = tr_anim_puzzlehole;
tr5_triggers[247] = tr_anim_puzzlehole;
tr5_triggers[248] = tr_anim_puzzlehole;
tr5_triggers[249] = tr_anim_puzzlehole;

tr1_triggers[137] = tr_anim_keyhole;
tr1_triggers[138] = tr_anim_keyhole;
tr1_triggers[139] = tr_anim_keyhole;
tr1_triggers[140] = tr_anim_keyhole;

tr2_triggers[201] = tr_anim_keyhole;
tr2_triggers[202] = tr_anim_keyhole;
tr2_triggers[203] = tr_anim_keyhole;
tr2_triggers[204] = tr_anim_keyhole;

tr3_triggers[232] = tr_anim_keyhole;
tr3_triggers[233] = tr_anim_keyhole;
tr3_triggers[234] = tr_anim_keyhole;
tr3_triggers[235] = tr_anim_keyhole;

tr4_triggers[284] = tr_anim_keyhole;
tr4_triggers[285] = tr_anim_keyhole;
tr4_triggers[286] = tr_anim_keyhole;
tr4_triggers[287] = tr_anim_keyhole;
tr4_triggers[288] = tr_anim_keyhole;
tr4_triggers[289] = tr_anim_keyhole;
tr4_triggers[290] = tr_anim_keyhole;
tr4_triggers[291] = tr_anim_keyhole;
tr4_triggers[292] = tr_anim_keyhole;
tr4_triggers[293] = tr_anim_keyhole;
tr4_triggers[294] = tr_anim_keyhole;
tr4_triggers[295] = tr_anim_keyhole;

tr5_triggers[258] = tr_anim_keyhole;
tr5_triggers[259] = tr_anim_keyhole;
tr5_triggers[260] = tr_anim_keyhole;
tr5_triggers[261] = tr_anim_keyhole;
tr5_triggers[262] = tr_anim_keyhole;
tr5_triggers[263] = tr_anim_keyhole;
tr5_triggers[264] = tr_anim_keyhole;
tr5_triggers[265] = tr_anim_keyhole;

function trigger_activate(trig_id, actor_id, func)
    local m_id = getModelID(trig_id);
    
    if(m_id == nil or m_id < 0) then
        return;
    end
    local on  = {};
    local off = {};
    local key = nil;

    if(cvars.engine_version < TR_II) then
        if(tr1_triggers[m_id] ~= nil) then
            on       = tr1_triggers[m_id].on;
            off      = tr1_triggers[m_id].off;
            key      = tr1_key[m_id];
            meshswap = tr1_puzzlehole_meshswap[m_id];
        else
            return;
        end
    elseif(cvars.engine_version < TR_III) then
        if(tr2_triggers[m_id] ~= nil) then
            on       = tr2_triggers[m_id].on;
            off      = tr2_triggers[m_id].off;
            meshswap = tr2_puzzlehole_meshswap[m_id];
        else
            return;
        end
    elseif(cvars.engine_version < TR_IV) then
        if(tr3_triggers[m_id] ~= nil) then
            on       = tr3_triggers[m_id].on;
            off      = tr3_triggers[m_id].off;
            meshswap = tr3_puzzlehole_meshswap[m_id];
        else
            return;
        end
    elseif(cvars.engine_version < TR_V) then
        if(tr4_triggers[m_id] ~= nil) then
            on       = tr4_triggers[m_id].on;
            off      = tr4_triggers[m_id].off;
            meshswap = tr4_puzzlehole_meshswap[m_id];
        else
            return;
        end
    else
        if(tr5_triggers[m_id] == nil) then
            if(m_id >= 266 and m_id <= 274) then
                local ocb = getEntityOCB(trig_id);
                if(ocb == 0) then
                    on  = tr_anim_bigwallswitch.on;
                    off = tr_anim_bigwallswitch.off;
                elseif(ocb == 1 or ocb == 2) then
                    on  = tr_anim_hole.on;
                    off = tr_anim_hole.off;
                elseif(ocb == 3) then
                    on  = tr_anim_bigbutton.on;
                    off = tr_anim_bigbutton.off;
                elseif(ocb == 4) then
                    on  = tr_anim_smallwallswitch.on;
                    off = tr_anim_smallwallswitch.off;
                elseif(ocb == 5) then
                    on  = tr_anim_smallbutton.on;
                    off = tr_anim_smallbutton.off;
                elseif(ocb >= 6 and ocb <= 10) then
                    on  = tr_anim_valve.on;
                    off = tr_anim_valve.off;
                elseif(ocb > 10) then
                    on  = {ready_anim =  0, trig_anim = 2, actor_anim = ocb};
                    off = {ready_anim =  1, trig_anim = 3, actor_anim = ocb + 1};
                end
            else
                return;
            end
        else
            on       = tr5_triggers[m_id].on;
            off      = tr5_triggers[m_id].off;
            meshswap = tr5_puzzlehole_meshswap[m_id];
        end
    end

    local t = getEntityAnim(trig_id);
    if(on.ready_anim < 0 or on.ready_anim == t) then
    
        if(key ~= nil) then
            if(getItemsCount(player, key) <= 0) then
                if(getActionChange(act.action) == 0) then
                    playsound(2);
                end;
                return;
            else
                removeItem(player, key, 1);
            end;
        end;
        
        setEntityAnim(trig_id, on.trig_anim, 0);
        setEntityAnim(actor_id, on.actor_anim, 0);
        setEntityActivity(trig_id, 1);
        addTask(
        function()
            local a, f, c = getEntityAnim(actor_id);
            if(on.switch_frame ~= nil) then
                c = on.switch_frame
            end
            if(f >= c - 1) then                                 -- check the end of animation
                if(meshswap ~= nil) then
                    setEntityMeshswap(trig_id, meshswap)
                end
                func(1);                                        -- turn on
                return nil;
            end
            return true;
        end);
    elseif(off.ready_anim == t) then
        setEntityAnim(trig_id, off.trig_anim, 0);
        setEntityAnim(actor_id, off.actor_anim, 0);
        --setEntityActivity(trig_id, 1);
        addTask(
        function()
            local a, f, c = getEntityAnim(actor_id);
            if(off.switch_frame ~= nil) then
                c = off.switch_frame
            end
            if(f >= c - 1) then                                 -- check the end of animation
                func(0);                                        -- turn off
                return nil;
            end
            return true;
        end);
    end
end
