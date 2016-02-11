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
