-- LUA config file

screen = 
{
    x = 50;
    y = 20;
    width = 800;
    height = 600;
    fullscreen = 0;
    fov = 75.0;
}

audio = 
{
    sound_volume = 0.9;
    music_volume = 0.7;
    use_effects = 1;
    listener_is_player = 0;
    stream_buffer_size = 128;
}

render = 
{
    mipmap_mode = 3;
    mipmaps = 3;                                -- It's not recommended to set it higher than 3 to prevent border bleeding.
    lod_bias = 0;
    anisotropy = 8;                             -- Maximum depends and is limited by hardware capabilities.
    antialias = 1;
    antialias_samples = 8;                      -- Maximum depends and is limited by hardware capabilities.
    z_depth = 24;                               -- Maximum and recommended is 24.
    texture_border = 16;
}

controls = 
{
    mouse_sensitivity = 25.0;

    use_joy = 0;                                -- Use joystick - yes (1) or no (0)
    joy_number = 0;                             -- If you have one joystick in system, it will be 0.
    joy_rumble = 0;                             -- Force feedback

    joy_move_axis_x = 0;                        -- Movement axes options.
    joy_move_axis_y = 1;
    joy_move_invert_x = 0;
    joy_move_invert_y = 0;
    joy_move_sensitivity = 1.5;
    joy_move_deadzone = 1500;

    joy_look_axis_x = 2;                        -- Look axes options.
    joy_look_axis_y = 3;
    joy_look_invert_x = 0;
    joy_look_invert_y = 1;
    joy_look_sensitivity = 1.5;
    joy_look_deadzone = 1500;
}

console = 
{
    font_path = "VeraMono.ttf";
    font_color = {r = 255, g = 255, b = 255};
    background_color = {r = 112, g = 30, b = 75, a = 202};
    smooth = 1;

    font_size = 16;
    line_size = 72;
    log_size = 16;
    lines_count = 128;
    showing_lines = 128;
    spacing = 1.5;
    show_cursor_period = 0.5;
    show = 0; 
}

-- Keys binding
-- Please note that on XInput game controllers (XBOX360 and such), triggers are NOT
-- coded as joystick buttons. Instead, they have unique names: JOY_TRIGGERLEFT and
-- JOY_TRIGGERRIGHT.

bind(act.jump, KEY_SPACE, JOY_3);
bind(act.action, KEY_LCTRL, JOY_1);
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

bind(act.screenshot, KEY_PRINTSCREEN);
bind(act.console, KEY_BACKQUOTE);
bind(act.savegame, KEY_F5);
bind(act.loadgame, KEY_F6);

bind(act.smallmedi, KEY_5);
bind(act.bigmedi, KEY_6);

-- CVAR's section. here you can create and delete CVAR's
cvars.show_fps = 1; 
cvars.free_look_speed = 2500;


-- AUTOEXEC LINES
setgame(1);
setlevel(1);

gravity(0, 0, -5700.0);
mlook(1);
freelook(0);
cam_distance(1024.0);
noclip(0);

