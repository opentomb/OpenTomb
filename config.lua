screen =
{
    x = 50;
    y = 20;
    width = 720;
    height = 400;
    fullscreen = false;
    fov = 75.0;
    debug_info = false;
}

audio =
{
    sound_volume = 0.8;
    music_volume = 0.9;
    use_effects = true;
    listener_is_player = false;
    stream_buffer_size = 128;
}

render =
{
    use_gl3 = true;                             -- Disable if you have old videocard or driver.
    mipmap_mode = 3;                            -- 0 = no filter, 1/2 = bilinear filter, 3 = trilinear filter.
    mipmaps = 3;                                -- It's not recommended to set it higher than 3 to prevent border bleeding.
    lod_bias = 0;
    anisotropy = 4;                             -- Maximum depends and is limited by hardware capabilities.
    antialias = true;
    antialias_samples = 4;                      -- Maximum depends and is limited by hardware capabilities.
    z_depth = 24;                               -- Maximum and recommended is 24.
    texture_border = 16;                        -- Needed to prevent mipmap border bleeding.
    save_texture_memory = false;                -- Alternative texture generation algorithm, slower but less memory used.
    fog_color = {r = 255, g = 255, b = 255};
}

controls =
{
    mouse_sensitivity = 25.0;

    use_joy = false;                            -- Use joystick - yes (1) or no (0)
    joy_number = 0;                             -- If you have one joystick in system, it will be 0.
    joy_rumble = false;                         -- Force feedback

    joy_move_axis_x = 0;                        -- Movement axes options.
    joy_move_axis_y = 1;
    joy_move_invert_x = false;
    joy_move_invert_y = false;
    joy_move_sensitivity = 1.5;
    joy_move_deadzone = 1500;

    joy_look_axis_x = 2;                        -- Look axes options.
    joy_look_axis_y = 3;
    joy_look_invert_x = false;
    joy_look_invert_y = true;
    joy_look_sensitivity = 1.5;
    joy_look_deadzone = 1500;
}

console =
{
    background_color = {r = 0, g = 0, b = 0, a = 200};

    line_size = 72;
    log_size = 16;
    lines_count = 128;
    showing_lines = 40;
    spacing = 0.5;
    show_cursor_period = 0.5;
    show = false;
}

-- Keys binding
-- Please note that on XInput game controllers (XBOX360 and such), triggers are NOT
-- coded as joystick buttons. Instead, they have unique names: JOY_TRIGGERLEFT and
-- JOY_TRIGGERRIGHT.

dofile("scripts/config/control_constants.lua");

bind(act.jump, KEY_SPACE, JOY_3);
bind(act.action, KEY_LCTRL, JOY_1);
bind(act.drawweapon, KEY_F);
bind(act.roll, KEY_X, JOY_2);
bind(act.sprint, KEY_CAPSLOCK, JOY_TRIGGERRIGHT);
bind(act.crouch, KEY_V, JOY_TRIGGERLEFT);
bind(act.walk, KEY_LSHIFT, JOY_11);

bind(act.up, KEY_W);
bind(act.down, KEY_S);
bind(act.left, KEY_A);
bind(act.right, KEY_D);

bind(act.lookup, KEY_UP, JOY_POVUP);
bind(act.lookdown, KEY_DOWN, JOY_POVDOWN);
bind(act.lookleft, KEY_LEFT, JOY_POVLEFT);
bind(act.lookright, KEY_RIGHT, JOY_POVRIGHT);

bind(act.inventory, KEY_ESCAPE);
bind(act.screenshot, KEY_PRINTSCREEN);
bind(act.console, KEY_F12);
bind(act.savegame, KEY_F5);
bind(act.loadgame, KEY_F6);

bind(act.smallmedi, KEY_9);
bind(act.bigmedi, KEY_0);
