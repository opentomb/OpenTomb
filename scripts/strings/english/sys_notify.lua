-- OPENTOMB SYSTEM CONSOLE NOTIFICATIONS
-- by Lwmte, Apr 2015

--------------------------------------------------------------------------------
-- Here you have some global system console warnings which will frequently show.
--------------------------------------------------------------------------------

sys_notify[000]     = "You must enter entity ID!";
sys_notify[001]     = "Wrong arguments, must be %s.";
sys_notify[002]     = "Wrong arguments count, must be %s.";
sys_notify[003]     = "Can't find entity with ID #%d!";
sys_notify[004]     = "Wrong option index, %d is the maximum.";
sys_notify[005]     = "No entity or no character for entity #%d!";
sys_notify[006]     = "Wrong room ID: #%d!";
sys_notify[007]     = "There are no models with id #%d.";
sys_notify[008]     = "Wrong action number.";
sys_notify[009]     = "Can't create font. Possibly max. fonts? (%d / %d)";
sys_notify[010]     = "Can't create style. Possibly max. styles? (%d / %d)";
sys_notify[011]     = "Font with given ID doesn't exist or couldn't be removed!";
sys_notify[012]     = "Style with given ID doesn't exist or couldn't be removed!";
sys_notify[013]     = "Can't find skeletal model with id #%d!";
sys_notify[014]     = "Wrong animation number: #%d!";
sys_notify[015]     = "Wrong animation dispatch number: #%d!";
sys_notify[016]     = "Wrong frame number: #%d!";
sys_notify[017]     = "Wrong stream ID. Must be more or equal to 0.";
sys_notify[018]     = "Wrong sound ID. Must be in interval 0..%d.";
sys_notify[019]     = "Audio_Send error: no free channel.";
sys_notify[020]     = "Audio_Send error: no such sample.";
sys_notify[021]     = "Audio_Send: sample skipped - please retry!";
sys_notify[022]     = "Audio_Kill: sample %d isn't playing.";
sys_notify[023]     = "Flipmap bit %d state is FALSE - no rooms were swapped!";
sys_notify[024]     = "File not found - \"%s\"";
sys_notify[025]     = "Warning: image %s is not truecolor - not supported!";
sys_notify[026]     = "SDL couldn't load %s: %s";
sys_notify[027]     = "Bad frame offset";
sys_notify[028]     = "Error: can't open file!";
sys_notify[029]     = "Error: bad file format!";
sys_notify[030]     = "Invalid console line count value!";
sys_notify[031]     = "Flipmap with index %d doesn't exist!";
sys_notify[032]     = "Hair setup with index %d doesn't exist!";
sys_notify[033]     = "Can't create hair for character %d!";
sys_notify[034]     = "No hairs for character %d - nothing to remove.";
sys_notify[035]     = "Ragdoll setup with index %d is corrupted or doesn't exist!";
sys_notify[036]     = "Can't create ragdoll for entity %d!";
sys_notify[037]     = "Can't remove ragdoll for entity %d - possibly no ragdoll?";
sys_notify[038]     = "Load_Wad: track count is out of bounds - max. %d.";
sys_notify[039]     = "Load_Wad: can't seek at offset %X.";
sys_notify[040]     = "StreamPlay: CANCEL, track index is out of bounds (%d).";
sys_notify[041]     = "StreamPlay: CANCEL, track already playing (%d).";
sys_notify[042]     = "StreamPlay: CANCEL, wrong track index or broken script (%d).";
sys_notify[043]     = "StreamPlay: CANCEL, no free stream.";
sys_notify[044]     = "StreamPlay: CANCEL, stream load error.";
sys_notify[045]     = "StreamPlay: CANCEL, stream play error.";
sys_notify[046]     = "Wrong sector info!";
sys_notify[047]     = "Wrong bone number: %d";
sys_notify[048]     = "Warning: image %s could not be loaded, status is %d.";
sys_notify[049]     = "Wrong model ID: %d";
sys_notify[050]     = "Can't find entity %d or body number is more than %d!";
sys_notify[051]     = "Can't apply force to entity %d - no entity, body, or entity is not dynamic!";


sys_notify[1000]    = "Track opened (%s): channels = %d, sample rate = %d";
sys_notify[1001]    = "Reading file: \"%s\"";
sys_notify[1002]    = "Giving item %i x%i to entity %x";
sys_notify[1003]    = "Changing current level ID to %d";
sys_notify[1004]    = "Changing current game to %d";
sys_notify[1005]    = "Tomb Raider engine version: %d, map: \"%s\"";
sys_notify[1006]    = "Num Rooms: %d";
sys_notify[1007]    = "Num Textures: %d";
sys_notify[1008]    = "Console line spacing: %f";
sys_notify[1009]    = "Console line count: %d";
sys_notify[1010]    = "TRIGGER: timer - %d, mask - %02X";
sys_notify[1011]    = "Activate object %d by %d";
sys_notify[1012]    = "Loaded fader picture: %s";
sys_notify[1013]    = "Trigger array cleaned up successfully!";
sys_notify[1014]    = "Entity function array cleaned up successfully!";
sys_notify[1015]    = "Loaded PC-specific level file (VT loader used).";
sys_notify[1016]    = "Loaded PSX-specific level file.";
sys_notify[1017]    = "Loaded Dreamcast-specific level file.";
sys_notify[1018]    = "Loaded OpenTomb level file.";
sys_notify[1019]    = "Reading wad (%s): offset %X, size %X.";
sys_notify[1020]    = "Engine initialized!";
sys_notify[1021]    = "Current Lua stack index: %d";
sys_notify[1022]    = "Loading map: %s";

sys_notify[1023]    = "Available commands:";
sys_notify[1024]    = "  help - show help info";
sys_notify[1025]    = "  cls - clean console";
sys_notify[1026]    = "  setgamef(game, level) - load specified game version and level number";
sys_notify[1027]    = "  loadMap(\"file_name\") - forcibly load a level \"file_name\"";
sys_notify[1028]    = "  save, load - save and load game state in \"file_name\"";
sys_notify[1029]    = "  exit - close program";
sys_notify[1030]    = "  show_fps - switch show fps flag";
sys_notify[1031]    = "  free_look - switch camera mode";
sys_notify[1032]    = "  mlook - turn on mouse look";
sys_notify[1033]    = "  noclip - DOZY mode";
sys_notify[1034]    = "  cam_distance - camera distance to actor";
sys_notify[1035]    = "  time_scale - turn on VERY SLOW motion (debugging mode)";
sys_notify[1036]    = "  r_coll, r_portals, r_frustums, r_room_boxes, r_boxes, r_normals, r_skip_room - render modes";
sys_notify[1037]    = "Watch out for case sensitive commands!";