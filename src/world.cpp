
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_rwops.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <map>
#include <list>

#include "core/gl_util.h"
#include "core/console.h"
#include "core/system.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/camera.h"
#include "render/frustum.h"
#include "render/render.h"
#include "render/bordered_texture_atlas.h"
#include "audio/audio.h"
#include "script/script.h"
#include "state_control/state_control.h"
#include "physics/physics.h"
#include "gui/gui.h"
#include "gui/gui_inventory.h"
#include "vt/vt_level.h"
#include "room.h"
#include "world.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "entity.h"
#include "character_controller.h"
#include "engine.h"
#include "gameflow.h"
#include "resource.h"
#include "inventory.h"
#include "trigger.h"


 struct world_s
{
    char                           *name;
    uint32_t                        id;
    uint32_t                        version;

    uint32_t                        rooms_count;
    struct room_s                  *rooms;

    uint32_t                        room_boxes_count;
    struct room_box_s              *room_boxes;

    struct box_overlap_s           *overlaps;
    uint32_t                        overlaps_count;

    uint32_t                        flip_count;             // Number of flips
    uint8_t                        *flip_map;               // Flipped room activity array.
    uint8_t                        *flip_state;             // Flipped room state array.
    uint16_t                        global_flip_state;

    bordered_texture_atlas         *tex_atlas;
    uint32_t                        tex_count;              // Number of textures
    GLuint                         *textures;               // OpenGL textures indexes

    uint32_t                        anim_sequences_count;   // Animated texture sequence count
    struct anim_seq_s              *anim_sequences;         // Animated textures

    uint32_t                        meshes_count;           // Base meshes count
    struct base_mesh_s             *meshes;                 // Base meshes data

    uint32_t                        sprites_count;          // Base sprites count
    struct sprite_s                *sprites;                // Base sprites data

    uint32_t                        skeletal_models_count;  // number of base skeletal models
    struct skeletal_model_s        *skeletal_models;        // base skeletal models data

    struct entity_s                *player;                 // this is an unique Lara's pointer =)
    struct skeletal_model_s        *sky_box;                // global skybox

    std::map<uint32_t, entity_p>    entity_tree;
    std::map<uint32_t, base_item_p> items_tree;

    uint32_t                        type;

    uint32_t                        cameras_sinks_count;    // Amount of cameras and sinks.
    struct static_camera_sink_s    *cameras_sinks;          // Cameras and sinks.
    uint32_t                        flyby_frames_count;
    struct camera_frame_s          *flyby_frames;
    uint32_t                        cinematic_frames_count;
    struct camera_frame_s          *cinematic_frames;
    struct flyby_camera_sequence_s *flyby_camera_sequences;
} global_world;


// private load level functions prototipes:
void World_SetEntityModelProperties(struct entity_s *ent);
void World_SetStaticMeshProperties(struct static_mesh_s *r_static);
void World_SetEntityFunction(struct entity_s *ent);
void World_ScriptsOpen(const char *path);
void World_AutoexecOpen();
// Create entity function from script, if exists.
bool Res_CreateEntityFunc(lua_State *lua, const char* func_name, int entity_id);


void World_GenTextures(class VT_Level *tr);
void World_GenAnimTextures(class VT_Level *tr);
void World_GenMeshes(class VT_Level *tr);
void World_GenSprites(class VT_Level *tr);
void World_GenBoxes(class VT_Level *tr);
void World_GenCameras(class VT_Level *tr);
void World_GenCinematicCameras(class VT_Level *tr);
void World_GenFlyByCameras(class VT_Level *tr);
void World_GenRoom(struct room_s *room, class VT_Level *tr);
void World_GenRooms(class VT_Level *tr);
void World_GenRoomFlipMap();
void World_GenSkeletalModels(class VT_Level *tr);
void World_GenEntities(class VT_Level *tr);
void World_GenBaseItems();
void World_GenSpritesBuffer();
void World_GenRoomProperties(class VT_Level *tr);
void World_GenRoomCollision();
void World_FixRooms();
void World_BuildNearRoomsList(struct room_s *room);
void World_BuildOverlappedRoomsList(struct room_s *room);

void World_Prepare()
{
    global_world.id = 0;
    global_world.name = NULL;
    global_world.type = 0x00;
    global_world.meshes = NULL;
    global_world.meshes_count = 0;
    global_world.sprites = NULL;
    global_world.sprites_count = 0;
    global_world.rooms_count = 0;
    global_world.rooms = 0;
    global_world.flip_map = NULL;
    global_world.flip_state = NULL;
    global_world.flip_count = 0;
    global_world.global_flip_state = 0;
    global_world.textures = NULL;
    global_world.type = 0;
    global_world.player = NULL;

    global_world.anim_sequences = NULL;
    global_world.anim_sequences_count = 0;

    global_world.tex_count = 0;
    global_world.textures = 0;
    global_world.room_boxes = NULL;
    global_world.room_boxes_count = 0;
    global_world.overlaps = NULL;
    global_world.overlaps_count = 0;
    global_world.cameras_sinks = NULL;
    global_world.cameras_sinks_count = 0;
    global_world.flyby_frames = NULL;
    global_world.flyby_frames_count = 0;
    global_world.cinematic_frames = NULL;
    global_world.cinematic_frames_count = 0;
    global_world.flyby_camera_sequences = NULL;
    global_world.skeletal_models = NULL;
    global_world.skeletal_models_count = 0;
    global_world.sky_box = NULL;
}


void World_Open(const char *path, int trv)
{
    VT_Level *tr = new VT_Level();
    tr->read_level(path, trv);
    tr->prepare_level();
    //tr_level->dump_textures();
    World_Clear();

    global_world.version = tr->game_version;

    World_ScriptsOpen(path);                // Open configuration scripts.
    Gui_DrawLoadScreen(200);

    World_GenTextures(tr);              // Generate OGL textures
    Gui_DrawLoadScreen(300);

    World_GenAnimTextures(tr);          // Generate animated textures
    Gui_DrawLoadScreen(320);

    World_GenMeshes(tr);                // Generate all meshes
    Gui_DrawLoadScreen(400);

    World_GenSprites(tr);               // Generate all sprites
    Gui_DrawLoadScreen(420);

    World_GenBoxes(tr);                 // Generate boxes.
    Gui_DrawLoadScreen(440);

    World_GenRooms(tr);                 // Build all rooms
    Gui_DrawLoadScreen(480);

    World_GenCameras(tr);               // Generate cameras & sinks.
    World_GenCinematicCameras(tr);
    World_GenFlyByCameras(tr);
    Gui_DrawLoadScreen(500);

    World_GenRoomFlipMap();             // Generate room flipmaps
    Gui_DrawLoadScreen(520);

    // Build all skeletal models. Must be generated before TR_Sector_Calculate() function.
    World_GenSkeletalModels(tr);
    Gui_DrawLoadScreen(600);

    World_GenEntities(tr);              // Build all moveables (entities)
    Gui_DrawLoadScreen(650);

    World_GenBaseItems();               // Generate inventory item entries.
    Gui_DrawLoadScreen(680);

    // Generate sprite buffers. Only now because entity generation adds new sprites
    World_GenSpritesBuffer();
    Gui_DrawLoadScreen(700);

    // Initialize audio.
    Audio_GenSamples(tr);
    Gui_DrawLoadScreen(750);

    World_GenRoomProperties(tr);
    Gui_DrawLoadScreen(800);

    World_GenRoomCollision();
    Gui_DrawLoadScreen(850);

    // Find and set skybox.
    global_world.sky_box = World_GetSkybox();
    Gui_DrawLoadScreen(860);

    // Generate entity functions.
    for(const std::pair<uint32_t, entity_p> &it : global_world.entity_tree)
    {
        World_SetEntityFunction(it.second);
    }
    Gui_DrawLoadScreen(910);

    // Load entity collision flags and ID overrides from script.

    Gui_DrawLoadScreen(940);

    // Process level autoexec loading.
    Audio_Init();
    World_AutoexecOpen();
    Gui_DrawLoadScreen(960);

    // Fix initial room states
    World_FixRooms();
    World_UpdateFlipCollisions();
    Gui_DrawLoadScreen(970);

    if(global_world.tex_atlas)
    {
        delete global_world.tex_atlas;
        global_world.tex_atlas = NULL;
    }

    delete tr;
}


void World_Clear()
{
    extern engine_container_p last_cont;

    last_cont = NULL;
    Script_LuaClearTasks();
    // De-initialize and destroy all audio objects.
    Audio_DeInit();

    if(main_inventory_manager != NULL)
    {
        main_inventory_manager->setInventory(NULL);
        main_inventory_manager->setItemsType(1);                                // see base items
    }

    global_world.player = NULL;

    /* entity empty must be done before rooms destroy */
    for(std::pair<const uint32_t, entity_p> &it : global_world.entity_tree)
    {
        Entity_Delete(it.second);
        it.second = NULL;
    }
    global_world.entity_tree.clear();

    /* Now we can delete physics misc objects */
    Physics_CleanUpObjects();

    for(uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        Room_Clear(global_world.rooms + i);
    }
    global_world.rooms_count = 0;
    free(global_world.rooms);
    global_world.rooms = NULL;

    if(global_world.flip_count)
    {
        global_world.flip_count = 0;
        global_world.global_flip_state = 0;
        free(global_world.flip_map);
        free(global_world.flip_state);
        global_world.flip_map = NULL;
        global_world.flip_state = NULL;
    }

    if(global_world.room_boxes_count)
    {
        global_world.room_boxes_count = 0;
        free(global_world.room_boxes);
        global_world.room_boxes = NULL;
    }

    if(global_world.overlaps_count)
    {
        global_world.overlaps_count = 0;
        free(global_world.overlaps);
        global_world.overlaps = NULL;
    }

    if(global_world.cameras_sinks_count)
    {
        global_world.cameras_sinks_count = 0;
        free(global_world.cameras_sinks);
        global_world.cameras_sinks = NULL;
    }

    if(global_world.flyby_frames_count)
    {
        global_world.flyby_frames_count = 0;
        free(global_world.flyby_frames);
        global_world.flyby_frames = NULL;
    }

    if(global_world.cinematic_frames_count)
    {
        global_world.cinematic_frames_count = 0;
        free(global_world.cinematic_frames);
        global_world.cinematic_frames = NULL;
    }

    for(flyby_camera_sequence_p s = global_world.flyby_camera_sequences; s;)
    {
        flyby_camera_sequence_p next = s->next;
        FlyBySequence_Clear(s);
        free(s);
        s = next;
    }
    global_world.flyby_camera_sequences = NULL;

    /*sprite empty*/
    if(global_world.sprites_count)
    {
        global_world.sprites_count = 0;
        free(global_world.sprites);
        global_world.sprites = NULL;
    }

    /*items empty*/
    for(std::pair<const uint32_t, base_item_p> &it : global_world.items_tree)
    {
        BaseItem_Delete(it.second);
        it.second = NULL;
    }
    global_world.items_tree.clear();

    if(global_world.skeletal_models_count)
    {
        for(uint32_t i = 0; i < global_world.skeletal_models_count; i++)
        {
            SkeletalModel_Clear(global_world.skeletal_models + i);
        }
        global_world.skeletal_models_count = 0;
        free(global_world.skeletal_models);
        global_world.skeletal_models = NULL;
    }

    /*mesh empty*/
    if(global_world.meshes_count)
    {
        for(uint32_t i = 0; i < global_world.meshes_count; i++)
        {
            BaseMesh_Clear(global_world.meshes+i);
        }
        global_world.meshes_count = 0;
        free(global_world.meshes);
        global_world.meshes = NULL;
    }

    if(global_world.tex_count)
    {
        qglDeleteTextures(global_world.tex_count, global_world.textures);
        global_world.tex_count = 0;
        free(global_world.textures);
        global_world.textures = NULL;
    }

    if(global_world.tex_atlas)
    {
        delete global_world.tex_atlas;
        global_world.tex_atlas = NULL;
    }

    if(global_world.anim_sequences_count)
    {
        for(uint32_t i = 0; i < global_world.anim_sequences_count; i++)
        {
            if(global_world.anim_sequences[i].frames_count != 0)
            {
                free(global_world.anim_sequences[i].frame_list);
                global_world.anim_sequences[i].frame_list = NULL;
                free(global_world.anim_sequences[i].frames);
                global_world.anim_sequences[i].frames = NULL;
            }
            global_world.anim_sequences[i].frames_count = 0;
        }
        global_world.anim_sequences_count = 0;
        free(global_world.anim_sequences);
        global_world.anim_sequences = NULL;
    }
}


int World_GetVersion()
{
    return global_world.version;
}


uint32_t World_SpawnEntity(uint32_t model_id, uint32_t room_id, float pos[3], float ang[3], int32_t id)
{
    skeletal_model_p model = World_GetModelByID(model_id);

    if(model)
    {
        entity_p entity = World_GetEntityByID(id);

        if(entity)
        {
            if(pos != NULL)
            {
                vec3_copy(entity->transform + 12, pos);
            }
            if(ang != NULL)
            {
                vec3_copy(entity->angles, ang);
                Entity_UpdateTransform(entity);
            }
            if(room_id < global_world.rooms_count)
            {
                Entity_MoveToRoom(entity, global_world.rooms + room_id);
                entity->self->sector = Room_GetSectorRaw(entity->self->room, entity->transform + 12);
            }
            else
            {
                Room_RemoveObject(entity->self->room, entity->self);
                entity->self->room = NULL;
            }

            return entity->id;
        }

        entity = Entity_Create();
        if(id < 0)
        {
            entity->id = 0;
            if(!global_world.entity_tree.empty())
            {
                entity->id = global_world.entity_tree.rbegin()->first + 1;
            }
        }
        else
        {
            entity->id = id;
        }

        if(pos != NULL)
        {
            vec3_copy(entity->transform + 12, pos);
        }
        if(ang != NULL)
        {
            vec3_copy(entity->angles, ang);
            Entity_UpdateTransform(entity);
        }
        if(room_id < global_world.rooms_count)
        {
            entity->self->room = global_world.rooms + room_id;
            Room_AddObject(entity->self->room, entity->self);
            entity->self->sector = Room_GetSectorRaw(entity->self->room, entity->transform + 12);
        }
        else
        {
            entity->self->room = NULL;
        }

        entity->type_flags     = ENTITY_TYPE_SPAWNED;
        entity->state_flags    = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE | ENTITY_STATE_COLLIDABLE;
        entity->trigger_layout = 0x00;
        entity->OCB            = 0x00;
        entity->timer          = 0.0;

        entity->self->collision_group = COLLISION_GROUP_KINEMATIC;
        entity->self->collision_mask = COLLISION_MASK_ALL;
        entity->self->collision_shape = COLLISION_SHAPE_TRIMESH;
        entity->move_type          = 0x0000;
        entity->move_type          = 0;

        SSBoneFrame_CreateFromModel(entity->bf, model);
        entity->bf->transform = entity->transform;
        Entity_SetAnimation(entity, ANIM_TYPE_BASE, 0, 0);
        Physics_GenRigidBody(entity->physics, entity->bf);

        Entity_RebuildBV(entity);
        if(entity->self->room != NULL)
        {
            Room_AddObject(entity->self->room, entity->self);
        }
        World_AddEntity(entity);
        return entity->id;
    }

    return ENTITY_ID_NONE;
}


struct entity_s *World_GetEntityByID(uint32_t id)
{
    entity_p ent = NULL;
    std::map<uint32_t, entity_p>::iterator it = global_world.entity_tree.find(id);

    if(it != global_world.entity_tree.end())
    {
        ent = it->second;
    }

    return ent;
}


void World_SetPlayer(struct entity_s *entity)
{
    int top = lua_gettop(engine_lua);
    global_world.player = entity;
    if(entity && entity->character)
    {
        lua_pushinteger(engine_lua, entity->id);
    }
    else
    {
        lua_pushinteger(engine_lua, ENTITY_ID_NONE);
    }
    lua_setglobal(engine_lua, "player");
    lua_settop(engine_lua, top);
}


struct entity_s *World_GetPlayer()
{
    return global_world.player;
}


void World_IterateAllEntities(int (*iterator)(struct entity_s *ent, void *data), void *data)
{
    std::list<uint32_t> delete_list;
    for(std::pair<const uint32_t, entity_p> &it : global_world.entity_tree)
    {
        if(it.second->state_flags & ENTITY_STATE_DELETED)
        {
            delete_list.push_back(it.first);
        }
        if(iterator(it.second, data))
        {
            break;
        }
    }
    for(uint32_t id : delete_list)
    {
        World_DeleteEntity(id);
    }
}


struct flyby_camera_sequence_s *World_GetFlyBySequences()
{
    return global_world.flyby_camera_sequences;
}


struct base_item_s *World_GetBaseItemByID(uint32_t id)
{
    base_item_p item = NULL;
    std::map<uint32_t, base_item_p>::iterator it = global_world.items_tree.find(id);

    if(it != global_world.items_tree.end())
    {
        item = it->second;
    }

    return item;
}


struct base_item_s *World_GetBaseItemByWorldModelID(uint32_t id)
{
    for(std::pair<const uint32_t, base_item_p> &it : global_world.items_tree)
    {
        if(it.second && (id == it.second->world_model_id))
        {
            return it.second;
        }
    }

    return NULL;
}


struct static_camera_sink_s *World_GetStaticCameraSink(uint32_t id)
{
    if(id < global_world.cameras_sinks_count)
    {
        return global_world.cameras_sinks + id;
    }
    return NULL;
}


struct camera_frame_s *World_GetCinematicFrame(uint32_t id)
{
    if(id < global_world.cinematic_frames_count)
    {
        return global_world.cinematic_frames + id;
    }
    return NULL;
}


void World_GetSkeletalModelsInfo(struct skeletal_model_s **models, uint32_t *models_count)
{
    *models = global_world.skeletal_models;
    *models_count = global_world.skeletal_models_count;
}


void World_GetRoomInfo(struct room_s **rooms, uint32_t *rooms_count)
{
    *rooms = global_world.rooms;
    *rooms_count = global_world.rooms_count;
}


void World_GetAnimSeqInfo(struct anim_seq_s **seq, uint32_t *seq_count)
{
    *seq = global_world.anim_sequences;
    *seq_count = global_world.anim_sequences_count;
}


void World_GetFlipInfo(uint8_t **flip_map, uint8_t **flip_state, uint32_t *flip_count)
{
    *flip_map = global_world.flip_map;
    *flip_state = global_world.flip_state;
    *flip_count = global_world.flip_count;
}


int World_AddAnimSeq(struct anim_seq_s *seq)
{
    anim_seq_p new_seqs = (anim_seq_p)realloc(global_world.anim_sequences, (global_world.anim_sequences_count + 1) * sizeof(anim_seq_t));
    if(new_seqs)
    {
        new_seqs[global_world.anim_sequences_count] = *seq;
        global_world.anim_sequences = new_seqs;
        global_world.anim_sequences_count++;
        return 1;
    }
    new_seqs = (anim_seq_p)malloc((global_world.anim_sequences_count + 1) * sizeof(anim_seq_t));
    if(new_seqs)
    {
        anim_seq_p old_seq = global_world.anim_sequences;
        if(old_seq)
        {
            memcpy(new_seqs, global_world.anim_sequences, global_world.anim_sequences_count * sizeof(anim_seq_t));
        }
        new_seqs[global_world.anim_sequences_count] = *seq;
        global_world.anim_sequences = new_seqs;
        global_world.anim_sequences_count++;
        if(old_seq)
        {
            free(old_seq);
        }
        return 1;
    }
    return 0;
}


int World_AddEntity(struct entity_s *entity)
{
    global_world.entity_tree[entity->id] = entity;

    return 1;
}


int World_DeleteEntity(uint32_t id)
{
    std::map<uint32_t, entity_p>::iterator it = global_world.entity_tree.find(id);

    if(it != global_world.entity_tree.end())
    {
        Entity_Delete(it->second);
        global_world.entity_tree.erase(it);
        return 1;
    }

    return 0;
}


int World_CreateItem(uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name)
{
    skeletal_model_p model = World_GetModelByID(model_id);
    if(model && (global_world.items_tree.count(item_id) == 0))
    {
        base_item_p item = BaseItem_Create(model, item_id);
        item->world_model_id = world_model_id;
        item->type = type;
        item->count = count;
        if(name)
        {
            strncpy(item->name, name, sizeof(item->name));
        }
        global_world.items_tree[item_id] = item;

        return 1;
    }

    return 0;
}


int World_DeleteItem(uint32_t item_id)
{
    std::map<uint32_t, base_item_p>::iterator it = global_world.items_tree.find(item_id);

    if(it != global_world.items_tree.end())
    {
        BaseItem_Delete(it->second);
        it->second = NULL;
        global_world.items_tree.erase(it);
    }

    return 1;
}


struct sprite_s *World_GetSpriteByID(uint32_t ID)
{
    sprite_p sp = global_world.sprites;
    for(uint32_t i = 0; i < global_world.sprites_count; i++, sp++)
    {
        if(sp->id == ID)
        {
            return sp;
        }
    }

    return NULL;
}


struct skeletal_model_s *World_GetModelByID(uint32_t id)
{
    long int i, min, max;

    min = 0;
    max = global_world.skeletal_models_count - 1;
    if(global_world.skeletal_models[min].id == id)
    {
        return global_world.skeletal_models + min;
    }
    if(global_world.skeletal_models[max].id == id)
    {
        return global_world.skeletal_models + max;
    }
    do
    {
        i = (min + max) / 2;
        if(global_world.skeletal_models[i].id == id)
        {
            return global_world.skeletal_models + i;
        }

        if(global_world.skeletal_models[i].id < id)
        {
            min = i;
        }
        else
        {
            max = i;
        }
    }
    while(min < max - 1);

    return NULL;
}


struct skeletal_model_s* World_GetSkybox()
{
    switch(global_world.version)
    {
        case TR_II:
        case TR_II_DEMO:
            return World_GetModelByID(TR_ITEM_SKYBOX_TR2);

        case TR_III:
            return World_GetModelByID(TR_ITEM_SKYBOX_TR3);

        case TR_IV:
        case TR_IV_DEMO:
            return World_GetModelByID(TR_ITEM_SKYBOX_TR4);

        case TR_V:
            return World_GetModelByID(TR_ITEM_SKYBOX_TR5);

        default:
            return NULL;
    };
}


struct room_s *World_GetRoomByID(uint32_t id)
{
    if(id < global_world.rooms_count)
    {
        return global_world.rooms + id;
    }
    return NULL;
}


struct room_s *World_FindRoomByPos(float pos[3])
{
    const float z_margin = TR_METERING_SECTORSIZE / 2.0f;
    room_p r = global_world.rooms;
    for(uint32_t i = 0; i < global_world.rooms_count; i++, r++)
    {
        if((r == r->real_room) &&
           (pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2] - z_margin) && (pos[2] < r->bb_max[2]))
        {
            room_sector_p orig_sector = Room_GetSectorRaw(r->real_room, pos);
            if(orig_sector && orig_sector->portal_to_room)
            {
                return orig_sector->portal_to_room->real_room;
            }
            return r->real_room;
        }
    }
    return NULL;
}


struct room_s *World_FindRoomByPosCogerrence(float pos[3], struct room_s *old_room)
{
    if(old_room == NULL)
    {
        return World_FindRoomByPos(pos);
    }

    old_room = old_room->real_room;
    room_sector_p orig_sector = Room_GetSectorRaw(old_room, pos);
    if(orig_sector && orig_sector->portal_to_room)
    {
        return orig_sector->portal_to_room->real_room;
    }

    if(orig_sector &&
       (pos[0] >= old_room->bb_min[0]) && (pos[0] < old_room->bb_max[0]) &&
       (pos[1] >= old_room->bb_min[1]) && (pos[1] < old_room->bb_max[1]))
    {
        if(orig_sector->room_below && (pos[2] < orig_sector->room_below->bb_max[2]))
        {
            return orig_sector->room_below->real_room;
        }
        else if((pos[2] >= old_room->bb_min[2]) && (pos[2] < old_room->bb_max[2]))
        {
            return old_room;
        }
        else if(orig_sector->room_above && (pos[2] >= orig_sector->room_above->bb_min[2]))
        {
            return orig_sector->room_above->real_room;
        }
    }

    const float z_margin = TR_METERING_SECTORSIZE / 2.0f;
    for(uint16_t i = 0; i < old_room->content->near_room_list_size; i++)
    {
        room_p r = old_room->content->near_room_list[i]->real_room;
        if((pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2] - z_margin) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }

    return World_FindRoomByPos(pos);
}


struct room_sector_s *World_GetRoomSector(int room_id, int x, int y)
{
    if((room_id >= 0) && ((uint32_t)room_id < global_world.rooms_count))
    {
        room_p room = global_world.rooms + room_id;
        if((x >= 0) && (y >= 0) && (x < room->sectors_x) && (y < room->sectors_y))
        {
            return room->content->sectors + x * room->sectors_y + y;
        }
    }

    return NULL;
}


uint32_t World_GetRoomBoxesCount()
{
    return global_world.room_boxes_count;
}


struct room_box_s *World_GetRoomBoxByID(uint32_t id)
{
    if(id < global_world.room_boxes_count)
    {
        return global_world.room_boxes + id;
    }

    return NULL;
}


void World_BuildNearRoomsList(struct room_s *room)
{
    room_sector_p rs = room->content->sectors;
    for(uint32_t i = 0; i < room->sectors_count; i++, rs++)
    {
        if(rs->portal_to_room)
        {
            Room_AddToNearRoomsList(room, rs->portal_to_room->real_room);
        }
    }

    int32_t list_1_size = room->content->near_room_list_size;
    for(int32_t j = -1; j < list_1_size; j++)
    {
        room_p r = (j < 0) ? room : room->content->near_room_list[j];
        rs = r->content->sectors;
        for(uint32_t i = 0; i < r->sectors_count; i++, rs++)
        {
            if(rs->room_above)
            {
                Room_AddToNearRoomsList(room, rs->room_above->real_room);
            }

            if(rs->room_below)
            {
                Room_AddToNearRoomsList(room, rs->room_below->real_room);
            }
        }
    }
}


void World_BuildOverlappedRoomsList(struct room_s *room)
{
    room_p r = global_world.rooms;
    for(uint32_t i = 0; i < global_world.rooms_count; ++i, ++r)
    {
        if((room->real_room != r->real_room) && Room_IsOverlapped(room, r))
        {
            Room_AddToOverlappedRoomsList(room, r);
            Room_AddToOverlappedRoomsList(r, room);
        }
    }
}

/*
 * WORLD  TRIGGERING  FUNCTIONS
 */

void World_UpdateFlipCollisions()
{
    room_p r = global_world.rooms;
    for(uint32_t i = 0; i < global_world.rooms_count; ++i, ++r)
    {
        if(r->real_room == r)
        {
            int num_tweens = r->sectors_count * 4;
            size_t buff_size = num_tweens * sizeof(sector_tween_t);
            sector_tween_p room_tween = (sector_tween_p)Sys_GetTempMem(buff_size);

            // Clear previous dynamic tweens
            Physics_DeleteObject(r->content->physics_alt_tween);
            r->content->physics_alt_tween = NULL;

            // Clear tween array.
            for(int j = 0; j < num_tweens; j++)
            {
                room_tween[j].ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
                room_tween[j].floor_tween_type   = TR_SECTOR_TWEEN_TYPE_NONE;
            }

            // Most difficult task with converting floordata collision to trimesh collision is
            // building inbetween polygons which will block out gaps between sector heights.
            num_tweens = Res_Sector_GenDynamicTweens(r, room_tween);
            if(num_tweens > 0)
            {
                r->content->physics_alt_tween = Physics_GenRoomRigidBody(r, NULL, 0, room_tween, num_tweens);
                if(r->content->physics_alt_tween)
                {
                    Physics_EnableObject(r->content->physics_alt_tween);
                }
            }

            Sys_ReturnTempMem(buff_size);
        }
    }
}


uint16_t World_GetGlobalFlipState()
{
    return global_world.global_flip_state;
}


void World_SetGlobalFlipState(int flip_state)
{
    room_p current_room = global_world.rooms;
    flip_state &= 0x00000001;
    for(uint32_t i = 0; i < global_world.rooms_count; i++, current_room++)
    {
        if(current_room->alternate_room_next || current_room->alternate_room_prev)
        {
            bool is_cycled = false;
            for(room_p room_it = current_room->alternate_room_next; room_it; room_it = room_it->alternate_room_next)
            {
                if(room_it == current_room)
                {
                    is_cycled = true;
                    break;
                }
            }
            if(current_room->alternate_room_next &&
               (!is_cycled || (current_room->alternate_room_next != current_room->real_room)) &&
               (( flip_state && !current_room->is_swapped) ||
                (!flip_state &&  current_room->is_swapped)))
            {
                current_room->is_swapped = !current_room->is_swapped;
                Room_DoFlip(current_room, current_room->alternate_room_next);
                global_world.global_flip_state = flip_state;
            }
        }
    }
    World_UpdateFlipCollisions();
}


int World_SetFlipState(uint32_t flip_index, uint32_t flip_state)
{
    int ret = 0;

    if(flip_index >= global_world.flip_count)
    {
        Con_Warning("wrong flipmap index");
        return 0;
    }

    if((global_world.flip_map[flip_index] == 0x1F) || (flip_state & 0x02))      // Check flipmap state.
    {
        room_p current_room = global_world.rooms;
        bool is_global_flip = global_world.version < TR_IV;
        if(global_world.flip_map[flip_index] != 0x1F)
        {
            flip_state = 0;
        }

        for(uint32_t i = 0; i < global_world.rooms_count; i++, current_room++)
        {
            if(is_global_flip || (current_room->content->alternate_group == flip_index))
            {
                bool is_cycled = false;
                for(room_p room_it = current_room->alternate_room_next; room_it; room_it = room_it->alternate_room_next)
                {
                    if(room_it == current_room)
                    {
                        is_cycled = true;
                        break;
                    }
                }
                if(current_room->alternate_room_next &&
                   (!is_cycled || (current_room->alternate_room_next != current_room->real_room)) &&
                   (( flip_state && !current_room->is_swapped) ||
                    (!flip_state &&  current_room->is_swapped)))
                {
                    current_room->is_swapped = !current_room->is_swapped;
                    Room_DoFlip(current_room, current_room->alternate_room_next);
                    ret = 1;
                }
            }
        }
        global_world.flip_state[flip_index] = flip_state & 0x01;
        global_world.global_flip_state = ret && is_global_flip && (flip_state & 0x01);
    }

    if(ret)
    {
        World_UpdateFlipCollisions();
    }

    return ret;
}


int World_SetFlipMap(uint32_t flip_index, uint8_t flip_mask, uint8_t flip_operation)
{
    if(flip_index >= global_world.flip_count)
    {
        Con_Warning("wrong flipmap index");
        return 0;
    }

    if(flip_operation == TRIGGER_OP_XOR)
    {
        global_world.flip_map[flip_index] ^= flip_mask;
    }
    else if(flip_operation == TRIGGER_OP_AND_INV)
    {
        global_world.flip_map[flip_index] &= ~flip_mask;
    }
    else
    {
        global_world.flip_map[flip_index] |= flip_mask;
    }

    return 0;
}


uint32_t World_GetFlipMap(uint32_t flip_index)
{
    if(flip_index >= global_world.flip_count)
    {
        return 0xFFFFFFFF;
    }

    return global_world.flip_map[flip_index];
}


uint32_t World_GetFlipState(uint32_t flip_index)
{
    if(flip_index >= global_world.flip_count)
    {
        return 0xFFFFFFFF;
    }

    return global_world.flip_state[flip_index];
}

/*
 * PRIVATE  WORLD  FUNCTIONS
 */
void World_ScriptsOpen(const char *path)
{
    if(engine_lua)
    {
        Script_DoLuaFile(engine_lua, "scripts/level_preload.lua");
        {
            char temp_script_name[1024];
            int top = lua_gettop(engine_lua);
            Engine_GetLevelScriptNameLocal(path, global_world.version, temp_script_name, sizeof(temp_script_name));
            int lua_err = Script_DoLuaFile(engine_lua, temp_script_name);
            if(lua_err)
            {
                Sys_DebugLog("lua_out.txt", "%s", lua_tostring(engine_lua, -1));
                lua_settop(engine_lua, top);
                return;
            }

            lua_getglobal(engine_lua, "level_PreLoad");
            if(lua_isfunction(engine_lua, -1))
            {
                lua_CallAndLog(engine_lua, 0, 0, 0);
            }
            lua_settop(engine_lua, top);
        }
    }
}


void World_AutoexecOpen()
{
    int top = lua_gettop(engine_lua);
    Script_DoLuaFile(engine_lua, "scripts/autoexec.lua");    // do standart autoexec
    lua_getglobal(engine_lua, "level_PostLoad");
    if(lua_isfunction(engine_lua, -1))
    {
        lua_CallAndLog(engine_lua, 0, 0, 0);
    }
    lua_settop(engine_lua, top);
}


bool Res_CreateEntityFunc(lua_State *lua, const char* func_name, int entity_id)
{
    if(lua)
    {
        const char* func_template = "%s_init";
        char buf[256] = {0};
        int top = lua_gettop(lua);
        snprintf(buf, 256, func_template, func_name);
        lua_getglobal(lua, buf);

        if(lua_isfunction(lua, -1))
        {
            snprintf(buf, 256, "if(entity_funcs[%d]==nil) then entity_funcs[%d]={} end", entity_id, entity_id);
            luaL_loadstring(lua, buf);
            if (lua_CallAndLog(lua, 0, LUA_MULTRET, 0))
            {
                lua_pushinteger(lua, entity_id);
                lua_CallAndLog(lua, 1, 0, 0);
            }
            lua_settop(lua, top);
            return true;
        }
        else
        {
            lua_settop(lua, top);
            return false;
        }
    }
    return false;
}


void World_SetEntityModelProperties(struct entity_s *ent)
{
    if(engine_lua && ent->bf->animations.model)
    {
        int top = lua_gettop(engine_lua);
        lua_getglobal(engine_lua, "getEntityModelProperties");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_pushinteger(engine_lua, global_world.version);                  // engine version
            lua_pushinteger(engine_lua, ent->bf->animations.model->id);         // entity model id
            if (lua_CallAndLog(engine_lua, 2, 3, 0))
            {
                ent->self->collision_group = lua_tointeger(engine_lua, -3);     // get collision type flag
                ent->self->collision_shape = lua_tointeger(engine_lua, -2);     // get collision shape flag
                ent->bf->animations.model->hide = lua_tointeger(engine_lua, -1);// get info about model visibility
            }
        }
        lua_settop(engine_lua, top);
    }
}


void World_SetStaticMeshProperties(struct static_mesh_s *r_static)
{
    if(engine_lua)
    {
        int top = lua_gettop(engine_lua);
        lua_getglobal(engine_lua, "getStaticMeshProperties");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_pushinteger(engine_lua, r_static->object_id);
            if(lua_CallAndLog(engine_lua, 1, 3, 0))
            {
                if(!lua_isnil(engine_lua, -3))
                {
                    r_static->self->collision_group = lua_tointeger(engine_lua, -3);
                }
                if(!lua_isnil(engine_lua, -2))
                {
                    r_static->self->collision_shape = lua_tointeger(engine_lua, -2);
                }
                if(!lua_isnil(engine_lua, -1))
                {
                    r_static->hide = lua_tointeger(engine_lua, -1);
                }
            }
        }
        lua_settop(engine_lua, top);
    }
}


void World_SetEntityFunction(struct entity_s *ent)
{
    if(engine_lua && ent->bf->animations.model)
    {
        int top = lua_gettop(engine_lua);
        lua_getglobal(engine_lua, "getEntityFunction");
        if(lua_isfunction(engine_lua, -1))
        {
            lua_pushinteger(engine_lua, global_world.version);                  // engine version
            lua_pushinteger(engine_lua, ent->bf->animations.model->id);         // entity model id
            if (lua_CallAndLog(engine_lua, 2, 1, 0))
            {
                if(!lua_isnil(engine_lua, -1))
                {
                    Res_CreateEntityFunc(engine_lua, lua_tolstring(engine_lua, -1, 0), ent->id);
                }
            }
        }
        lua_settop(engine_lua, top);
    }
}

// Functions setting parameters from configuration scripts.
void World_GenTextures(class VT_Level *tr)
{
    int border_size = renderer.settings.texture_border;
    border_size = (border_size < 0) ? (0) : (border_size);
    border_size = (border_size > 128) ? (128) : (border_size);
    global_world.tex_atlas = new bordered_texture_atlas(border_size,
                                                  tr->textile32_count,
                                                  tr->textile32,
                                                  tr->object_textures_count,
                                                  tr->object_textures,
                                                  tr->sprite_textures_count,
                                                  tr->sprite_textures);

    global_world.tex_count = (uint32_t) global_world.tex_atlas->getNumAtlasPages();
    global_world.textures = (GLuint*)malloc(global_world.tex_count * sizeof(GLuint));

    qglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    qglPixelZoom(1, 1);
    global_world.tex_atlas->createTextures(global_world.textures);

    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // Mag filter is always linear.

    // Select mipmap mode
    switch(renderer.settings.mipmap_mode)
    {
        case 0:
            qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            break;

        case 1:
            qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            break;

        case 2:
            qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            break;

        case 3:
        default:
            qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            break;
    };

    // Set mipmaps number
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, renderer.settings.mipmaps);

    // Set anisotropy degree
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, renderer.settings.anisotropy);

    // Read lod bias
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, renderer.settings.lod_bias);
}


/**   Animated textures loading.
  *   Natively, animated textures stored as a stream of bitu16s, which
  *   is then parsed on the fly. What we do is parse this stream to the
  *   proper structures to be used later within renderer.
  */
void World_GenAnimTextures(class VT_Level *tr)
{
    uint16_t *pointer;
    uint16_t  num_sequences, num_uvrotates;
    polygon_t p0, p;

    p0.vertex_count = 0;
    p0.vertices = NULL;
    p.vertex_count = 0;
    p.vertices = NULL;
    Polygon_Resize(&p0, 3);
    Polygon_Resize(&p, 3);

    pointer       = tr->animated_textures;
    num_uvrotates = tr->animated_textures_uv_count;
    num_sequences = *(pointer++);   // First word in a stream is sequence count.

    if(num_sequences)
    {
        global_world.anim_sequences_count = num_sequences;
        global_world.anim_sequences = (anim_seq_p)calloc(num_sequences, sizeof(anim_seq_t));

        anim_seq_p seq = global_world.anim_sequences;
        for(uint16_t i = 0; i < num_sequences; i++, seq++)
        {
            seq->frames_count = *(pointer++) + 1;
            seq->frame_list   =  (uint32_t*)calloc(seq->frames_count, sizeof(uint32_t));

            // Fill up new sequence with frame list.
            seq->anim_type         = TR_ANIMTEXTURE_FORWARD;
            seq->frame_lock        = false; // by default anim is playing
            seq->uvrotate          = false; // by default uvrotate
            seq->reverse_direction = false; // Needed for proper reverse-type start-up.
            seq->frame_rate        = 0.05f; // Should be passed as 1 / FPS.
            seq->frame_time        = 0.0f;  // Reset frame time to initial state.
            seq->current_frame     = 0;     // Reset current frame to zero.

            for(uint16_t j = 0; j < seq->frames_count; j++)
            {
                seq->frame_list[j] = *(pointer++);  // Add one frame.
            }

            // UVRotate textures case.
            // In TR4-5, it is possible to define special UVRotate animation mode.
            // It is specified by num_uvrotates variable. If sequence belongs to
            // UVRotate range, each frame will be divided in half and continously
            // scrolled from one part to another by shifting UV coordinates.
            // In OpenTomb, we can have BOTH UVRotate and classic frames mode
            // applied to the same sequence, but there we specify compatibility
            // method for TR4-5.
            // I need to find logic of original levels + add script functions or
            // other sticks for corret textures animationg.
            if((i < num_uvrotates) && (seq->frames_count <= 2))
            {
                seq->uvrotate   = true;
                seq->frame_rate = 0.05f * 16;
            }
            seq->frames = (tex_frame_p)calloc(seq->frames_count, sizeof(tex_frame_t));
            global_world.tex_atlas->getCoordinates(&p0, seq->frame_list[0], false);
            for(uint16_t j = 0; j < seq->frames_count; j++)
            {
                if(seq->uvrotate)
                {
                    seq->frames[j].uvrotate_max = 0.5 * global_world.tex_atlas->getTextureHeight(seq->frame_list[j]);
                    seq->frames[j].current_uvrotate = 0.0;
                }
                global_world.tex_atlas->getCoordinates(&p, seq->frame_list[j], false);
                seq->frames[j].texture_index = p.texture_index;
                if(j > 0)   // j == 0 -> d == 0;
                {
                    ///@PARANOID: texture transformation may be not only move
                    GLfloat A0[2], B0[2], A[2], B[2], d;
                    A0[0] = p0.vertices[1].tex_coord[0] - p0.vertices[0].tex_coord[0];
                    A0[1] = p0.vertices[1].tex_coord[1] - p0.vertices[0].tex_coord[1];
                    B0[0] = p0.vertices[2].tex_coord[0] - p0.vertices[0].tex_coord[0];
                    B0[1] = p0.vertices[2].tex_coord[1] - p0.vertices[0].tex_coord[1];

                    A[0] = p.vertices[1].tex_coord[0] - p.vertices[0].tex_coord[0];
                    A[1] = p.vertices[1].tex_coord[1] - p.vertices[0].tex_coord[1];
                    B[0] = p.vertices[2].tex_coord[0] - p.vertices[0].tex_coord[0];
                    B[1] = p.vertices[2].tex_coord[1] - p.vertices[0].tex_coord[1];

                    d = A0[0] * B0[1] - A0[1] * B0[0];
                    seq->frames[j].mat[0 + 0 * 2] = (A[0] * B0[1] - A0[1] * B[0]) / d;
                    seq->frames[j].mat[1 + 0 * 2] =-(A[1] * B0[1] - A0[1] * B[1]) / d;
                    seq->frames[j].mat[0 + 1 * 2] =-(A0[0] * B[0] - A[0] * B0[0]) / d;
                    seq->frames[j].mat[1 + 1 * 2] = (A0[0] * B[1] - A[1] * B0[0]) / d;
                }
                else
                {
                    seq->frames[j].mat[0 + 0 * 2] = 1.0;
                    seq->frames[j].mat[1 + 0 * 2] = 0.0;
                    seq->frames[j].mat[0 + 1 * 2] = 0.0;
                    seq->frames[j].mat[1 + 1 * 2] = 1.0;
                }

                seq->frames[j].move[0] = p.vertices[0].tex_coord[0] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[0 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[0 + 1 * 2]);
                seq->frames[j].move[1] = p.vertices[0].tex_coord[1] - (p0.vertices[0].tex_coord[0] * seq->frames[j].mat[1 + 0 * 2] + p0.vertices[0].tex_coord[1] * seq->frames[j].mat[1 + 1 * 2]);
            } ///end for(uint16_t j = 0; j < seq->frames_count; j++)
        }  ///end for(uint16_t i = 0; i < num_sequences; i++, seq++)
    }
    Polygon_Clear(&p0);
    Polygon_Clear(&p);
}


void World_GenMeshes(class VT_Level *tr)
{
    base_mesh_p base_mesh;

    global_world.meshes_count = tr->meshes_count;
    base_mesh = global_world.meshes = (base_mesh_p)calloc(global_world.meshes_count, sizeof(base_mesh_t));
    for(uint32_t i = 0; i < global_world.meshes_count; i++, base_mesh++)
    {
        TR_GenMesh(base_mesh, i, global_world.anim_sequences, global_world.anim_sequences_count, global_world.tex_atlas, tr);
        BaseMesh_GenFaces(base_mesh);
    }
}


void World_GenSprites(class VT_Level *tr)
{
    sprite_p s;
    tr_sprite_texture_t *tr_st;

    if(tr->sprite_textures_count == 0)
    {
        global_world.sprites = NULL;
        global_world.sprites_count = 0;
        return;
    }

    global_world.sprites_count = tr->sprite_textures_count;
    s = global_world.sprites = (sprite_p)calloc(global_world.sprites_count, sizeof(sprite_t));

    for(uint32_t i = 0; i < global_world.sprites_count; i++, s++)
    {
        tr_st = &tr->sprite_textures[i];

        s->left = tr_st->left_side;
        s->right = tr_st->right_side;
        s->top = tr_st->top_side;
        s->bottom = tr_st->bottom_side;

        global_world.tex_atlas->getSpriteCoordinates(s->tex_coord, i, &s->texture_index);
    }

    for(uint32_t i = 0; i < tr->sprite_sequences_count; i++)
    {
        if((tr->sprite_sequences[i].offset >= 0) && ((uint32_t)tr->sprite_sequences[i].offset < global_world.sprites_count))
        {
            global_world.sprites[tr->sprite_sequences[i].offset].id = tr->sprite_sequences[i].object_id;
        }
    }
}


void World_GenBoxes(class VT_Level *tr)
{
    global_world.overlaps = NULL;
    global_world.overlaps_count = tr->overlaps_count;

    if(global_world.overlaps_count)
    {
        global_world.overlaps = (box_overlap_p)malloc(global_world.overlaps_count * sizeof(box_overlap_t));
        for(uint32_t i = 0; i < global_world.overlaps_count; i++)
        {
            global_world.overlaps[i].box = tr->overlaps[i] & 0x7FFF;
            global_world.overlaps[i].end = (tr->overlaps[i] & 0x8000) ? (1) : (0);
        }
        global_world.overlaps[global_world.overlaps_count - 1].end = 1;         // never believe user data
    }

    global_world.room_boxes = NULL;
    global_world.room_boxes_count = tr->boxes_count;

    if(global_world.room_boxes_count)
    {
        global_world.room_boxes = (room_box_p)malloc(global_world.room_boxes_count * sizeof(room_box_t));
        for(uint32_t i = 0; i < global_world.room_boxes_count; i++)
        {
            room_box_p r_box = global_world.room_boxes + i;
            uint16_t ov_index = 0x7FFF & tr->boxes[i].overlap_index;
            r_box->overlaps = NULL;
            if((tr->boxes[i].overlap_index >= 0) && (ov_index < global_world.overlaps_count))
            {
                r_box->overlaps = global_world.overlaps + ov_index;
            }
            else
            {
                Con_Printf("box = %d, bad overlap index = %d", i, ov_index);
            }
            r_box->id = i;
            r_box->is_blockable = (tr->boxes[i].overlap_index & 0x8000) ? (0x01) : (0x00);
            r_box->is_blocked = 0x00;
            r_box->bb_min[0] = tr->boxes[i].xmin;
            r_box->bb_min[1] =-tr->boxes[i].zmax;
            r_box->bb_min[2] = tr->boxes[i].true_floor;
            r_box->bb_max[0] = tr->boxes[i].xmax;
            r_box->bb_max[1] =-tr->boxes[i].zmin;
            r_box->bb_max[2] = tr->boxes[i].true_floor + TR_METERING_SECTORSIZE;

            r_box->zone.GroundZone1_Normal = tr->zones[i].GroundZone1_Normal;
            r_box->zone.GroundZone2_Normal = tr->zones[i].GroundZone2_Normal;
            r_box->zone.GroundZone3_Normal = tr->zones[i].GroundZone3_Normal;
            r_box->zone.GroundZone4_Normal = tr->zones[i].GroundZone4_Normal;
            r_box->zone.FlyZone_Normal = tr->zones[i].FlyZone_Normal;
            r_box->zone.GroundZone1_Alternate = tr->zones[i].GroundZone1_Alternate;
            r_box->zone.GroundZone2_Alternate = tr->zones[i].GroundZone2_Alternate;
            r_box->zone.GroundZone3_Alternate = tr->zones[i].GroundZone3_Alternate;
            r_box->zone.GroundZone4_Alternate = tr->zones[i].GroundZone4_Alternate;
            r_box->zone.FlyZone_Alternate = tr->zones[i].FlyZone_Alternate;
        }
    }
}


void World_GenCameras(class VT_Level *tr)
{
    global_world.cameras_sinks = NULL;
    global_world.cameras_sinks_count = tr->cameras_count;

    if(global_world.cameras_sinks_count)
    {
        global_world.cameras_sinks = (static_camera_sink_p)malloc(global_world.cameras_sinks_count * sizeof(static_camera_sink_t));
        for(uint32_t i = 0; i < global_world.cameras_sinks_count; i++)
        {
            global_world.cameras_sinks[i].pos[0]              =  tr->cameras[i].x;
            global_world.cameras_sinks[i].pos[1]              =  tr->cameras[i].z;
            global_world.cameras_sinks[i].pos[2]              = -tr->cameras[i].y;
            global_world.cameras_sinks[i].locked              =  0;
            global_world.cameras_sinks[i].room_or_strength    =  tr->cameras[i].room;
            global_world.cameras_sinks[i].flag_or_zone        =  tr->cameras[i].unknown1;
        }
    }
}


void World_GenCinematicCameras(class VT_Level *tr)
{
    global_world.cinematic_frames = NULL;
    global_world.cinematic_frames_count = tr->cinematic_frames_count;

    if(global_world.cinematic_frames_count)
    {
        global_world.cinematic_frames = (camera_frame_p)calloc(global_world.cinematic_frames_count, sizeof(camera_frame_t));
        for(uint32_t i = 0; i < global_world.cinematic_frames_count; i++)
        {
            global_world.cinematic_frames[i].pos[0] = tr->cinematic_frames[i].posx;
            global_world.cinematic_frames[i].pos[1] = tr->cinematic_frames[i].posz;
            global_world.cinematic_frames[i].pos[2] =-tr->cinematic_frames[i].posy;
            global_world.cinematic_frames[i].target[0] = tr->cinematic_frames[i].targetx;
            global_world.cinematic_frames[i].target[1] = tr->cinematic_frames[i].targetz;
            global_world.cinematic_frames[i].target[2] =-tr->cinematic_frames[i].targety;
            global_world.cinematic_frames[i].fov = (float)(tr->cinematic_frames[i].fov) / 16384.0f * 90.0f;
            global_world.cinematic_frames[i].roll = (float)(tr->cinematic_frames[i].roll) / 16384.0f * 90.0f;
        }
    }
}

void World_GenFlyByCameras(class VT_Level *tr)
{
    global_world.flyby_frames = NULL;
    global_world.flyby_frames_count = tr->flyby_cameras_count;

    if(global_world.flyby_frames_count)
    {
        uint32_t start_index = 0;
        flyby_camera_sequence_p *last_seq_ptr = &global_world.flyby_camera_sequences;
        global_world.flyby_frames = (camera_frame_p)malloc(global_world.flyby_frames_count * sizeof(camera_frame_t));
        for(uint32_t i = 0; i < global_world.flyby_frames_count; i++)
        {
            union
            {
                camera_flags_t flags;
                uint16_t       flags_ui;
            };
            flags_ui  =  tr->flyby_cameras[i].flags;

            global_world.flyby_frames[i].flags           =  flags;
            global_world.flyby_frames[i].pos[0]          =  tr->flyby_cameras[i].pos_x;
            global_world.flyby_frames[i].pos[1]          =  tr->flyby_cameras[i].pos_z;
            global_world.flyby_frames[i].pos[2]          = -tr->flyby_cameras[i].pos_y;
            global_world.flyby_frames[i].target[0]       =  tr->flyby_cameras[i].target_x;
            global_world.flyby_frames[i].target[1]       =  tr->flyby_cameras[i].target_z;
            global_world.flyby_frames[i].target[2]       = -tr->flyby_cameras[i].target_y;

            global_world.flyby_frames[i].fov             =  tr->flyby_cameras[i].fov;
            global_world.flyby_frames[i].roll            =  tr->flyby_cameras[i].roll;
            global_world.flyby_frames[i].timer           =  tr->flyby_cameras[i].timer;
            global_world.flyby_frames[i].speed           =  tr->flyby_cameras[i].speed;

            global_world.flyby_frames[i].sequence        =  tr->flyby_cameras[i].sequence;
            global_world.flyby_frames[i].index           =  tr->flyby_cameras[i].index;

            if((tr->flyby_cameras[i].room_id >= 0) && ((uint32_t)tr->flyby_cameras[i].room_id < global_world.rooms_count))
            {
                global_world.flyby_frames[i].room            =  global_world.rooms + tr->flyby_cameras[i].room_id;
            }

            if((i + 1 == global_world.flyby_frames_count) || (tr->flyby_cameras[i].sequence != tr->flyby_cameras[i + 1].sequence))
            {
                *last_seq_ptr = FlyBySequence_Create(global_world.flyby_frames + start_index, i - start_index + 1);
                if(*last_seq_ptr)
                {
                    last_seq_ptr = &(*last_seq_ptr)->next;
                }
                start_index = i + 1;
            }
        }
        *last_seq_ptr = NULL;
    }
}


__inline void TR_vertex_to_arr(float v[3], tr5_vertex_t *tr_v)
{
    v[0] = tr_v->x;
    v[1] =-tr_v->z;
    v[2] = tr_v->y;
}

void World_GenRoom(struct room_s *room, class VT_Level *tr)
{
    portal_p p;
    room_p r_dest;
    tr5_room_t *tr_room = &tr->rooms[room->id];
    tr_staticmesh_t *tr_static;
    static_mesh_p r_static;
    tr_room_portal_t *tr_portal;
    room_sector_p sector;

    room->frustum = NULL;
    room->containers = NULL;
    room->is_in_r_list = 0;
    room->is_swapped = 0;

    Mat4_E_macro(room->transform);
    TR_vertex_to_arr(room->transform + 12, &tr->rooms[room->id].offset);

    room->self = Container_Create();
    room->self->next = NULL;
    room->self->room = room;
    room->self->object = room;
    room->self->collision_group = COLLISION_GROUP_STATIC_ROOM;
    room->self->collision_mask = COLLISION_MASK_ALL;
    room->self->collision_shape = COLLISION_SHAPE_TRIMESH;
    room->self->object_type = OBJECT_ROOM_BASE;

    room->content = (room_content_p)malloc(sizeof(room_content_t));
    room->original_content = room->content;
    room->content->original_room_id = room->id;
    room->content->room_flags = tr->rooms[room->id].flags;
    room->content->near_room_list_size = 0;
    room->content->near_room_list = NULL;
    room->content->overlapped_room_list_size = 0;
    room->content->overlapped_room_list = NULL;
    room->content->physics_body = NULL;
    room->content->physics_alt_tween = NULL;
    room->content->mesh = NULL;
    room->content->static_mesh = NULL;
    room->content->sprites = NULL;
    room->content->sprites_vertices = NULL;
    room->content->lights_count = 0;
    room->content->lights = NULL;
    room->content->light_mode = tr->rooms[room->id].light_mode;
    room->content->reverb_info = tr->rooms[room->id].reverb_info;
    room->content->water_scheme = tr->rooms[room->id].water_scheme;
    room->content->alternate_group = tr->rooms[room->id].alternate_group;
    room->content->ambient_lighting[0] = tr->rooms[room->id].light_colour.r * 2;
    room->content->ambient_lighting[1] = tr->rooms[room->id].light_colour.g * 2;
    room->content->ambient_lighting[2] = tr->rooms[room->id].light_colour.b * 2;

    TR_GenRoomMesh(room, room->id, global_world.anim_sequences, global_world.anim_sequences_count, global_world.tex_atlas, tr);
    if(room->content->mesh)
    {
        BaseMesh_GenFaces(room->content->mesh);
    }
    /*
     * let us load sectors
     */
    room->sectors_x = tr_room->num_xsectors;
    room->sectors_y = tr_room->num_zsectors;
    room->sectors_count = room->sectors_x * room->sectors_y;
    room->content->sectors = (room_sector_p)malloc(room->sectors_count * sizeof(room_sector_t));

    /*
     * base sectors information loading and collisional mesh creation
     */

    // To avoid manipulating with unnecessary information, we declare simple
    // heightmap here, which will be operated with sector and floordata parsing,
    // then vertical inbetween polys will be constructed, and Bullet collisional
    // object will be created. Afterwards, this heightmap also can be used to
    // quickly detect slopes for pushable blocks and other entities that rely on
    // floor level.
    sector = room->content->sectors;
    for(uint32_t i = 0; i < room->sectors_count; i++, sector++)
    {
        // Filling base sectors information.

        sector->index_x = i / room->sectors_y;
        sector->index_y = i % room->sectors_y;

        sector->pos[0] = room->transform[12 + 0] + sector->index_x * TR_METERING_SECTORSIZE + 0.5f * TR_METERING_SECTORSIZE;
        sector->pos[1] = room->transform[12 + 1] + sector->index_y * TR_METERING_SECTORSIZE + 0.5f * TR_METERING_SECTORSIZE;
        sector->pos[2] = 0.5f * (tr_room->y_bottom + tr_room->y_top);

        sector->owner_room = room;
        sector->trigger = NULL;
        sector->box = NULL;

        if(tr->game_version < TR_III)
        {
            if(tr_room->sector_list[i].box_index < global_world.room_boxes_count)
            {
                sector->box = global_world.room_boxes + tr_room->sector_list[i].box_index;
            }
            sector->material  = SECTOR_MATERIAL_STONE;
        }
        else
        {
            uint16_t box = (tr_room->sector_list[i].box_index & 0xFFF0) >> 4;
            if(box < global_world.room_boxes_count)
            {
                sector->box = global_world.room_boxes + box;
            }
            sector->material  = (tr_room->sector_list[i].box_index & 0x000F);
        }

        sector->flags = 0;  // Clear sector flags.

        sector->floor      = -TR_METERING_STEP * (int)tr_room->sector_list[i].floor;
        sector->ceiling    = -TR_METERING_STEP * (int)tr_room->sector_list[i].ceiling;
        sector->trig_index = tr_room->sector_list[i].fd_index;

        // BUILDING CEILING HEIGHTMAP.

        // Penetration config is used later to build inbetween vertical collision polys.
        // If sector's penetration config is a wall, we simply build a vertical plane to
        // isolate this sector from top to bottom. Also, this allows to trick out wall
        // sectors inside another wall sectors to be ignored completely when building
        // collisional mesh.
        // Door penetration config means that we should either ignore sector collision
        // completely (classic door) or ignore one of the triangular sector parts (TR3+).

        if(sector->ceiling == TR_METERING_WALLHEIGHT)
        {
            room->content->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_above != 0xFF)
        {
            room->content->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_GHOST;
        }
        else
        {
            room->content->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;
        }

        // Reset some sector parameters to avoid garbaged memory issues.
        room->content->sectors[i].portal_to_room = NULL;
        room->content->sectors[i].ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
        room->content->sectors[i].floor_diagonal_type   = TR_SECTOR_DIAGONAL_TYPE_NONE;

        // Now, we define heightmap cells position and draft (flat) height.
        // Draft height is derived from sector's floor and ceiling values, which are
        // copied into heightmap cells Y coordinates. As result, we receive flat
        // heightmap cell, which will be operated later with floordata.

        room->content->sectors[i].ceiling_corners[0][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[0][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[0][2] = (float)sector->ceiling;

        room->content->sectors[i].ceiling_corners[1][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[1][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[1][2] = (float)sector->ceiling;

        room->content->sectors[i].ceiling_corners[2][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[2][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[2][2] = (float)sector->ceiling;

        room->content->sectors[i].ceiling_corners[3][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[3][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->content->sectors[i].ceiling_corners[3][2] = (float)sector->ceiling;

        // BUILDING FLOOR HEIGHTMAP.

        // Features same steps as for the ceiling.

        if(sector->floor == TR_METERING_WALLHEIGHT)
        {
            room->content->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_below != 0xFF)
        {
            room->content->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_GHOST;
        }
        else
        {
            room->content->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
        }

        room->content->sectors[i].floor_corners[0][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[0][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[0][2] = (float)sector->floor;

        room->content->sectors[i].floor_corners[1][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[1][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[1][2] = (float)sector->floor;

        room->content->sectors[i].floor_corners[2][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[2][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[2][2] = (float)sector->floor;

        room->content->sectors[i].floor_corners[3][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[3][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->content->sectors[i].floor_corners[3][2] = (float)sector->floor;
    }

    /*
     *  load lights
     */
    room->content->lights_count = tr_room->num_lights;
    if(room->content->lights_count > 0)
    {
        room->content->lights = (light_p)malloc(room->content->lights_count * sizeof(light_t));
        for(uint16_t i = 0; i < tr_room->num_lights; i++)
        {
            Res_RoomLightCalculate(room->content->lights + i, tr_room->lights + i);
        }
    }

    /*
     * portals loading / calculation!!!
     */
    room->content->portals_count = tr_room->num_portals;
    p = room->content->portals = (portal_p)calloc(room->content->portals_count, sizeof(portal_t));
    tr_portal = tr_room->portals;
    for(uint16_t i = 0; i < room->content->portals_count; i++, p++, tr_portal++)
    {
        r_dest = global_world.rooms + tr_portal->adjoining_room;
        p->vertex_count = 4;                                                    // in original TR all portals are axis aligned rectangles
        p->vertex = (float*)malloc(3 * p->vertex_count * sizeof(float));
        p->dest_room = r_dest;
        TR_vertex_to_arr(p->vertex, &tr_portal->vertices[3]);
        vec3_add(p->vertex, p->vertex, room->transform + 12);
        TR_vertex_to_arr(p->vertex + 3, &tr_portal->vertices[2]);
        vec3_add(p->vertex + 3, p->vertex + 3, room->transform + 12);
        TR_vertex_to_arr(p->vertex + 6, &tr_portal->vertices[1]);
        vec3_add(p->vertex + 6, p->vertex + 6, room->transform + 12);
        TR_vertex_to_arr(p->vertex + 9, &tr_portal->vertices[0]);
        vec3_add(p->vertex + 9, p->vertex + 9, room->transform + 12);
        vec3_add(p->centre, p->vertex, p->vertex + 3);
        vec3_add(p->centre, p->centre, p->vertex + 6);
        vec3_add(p->centre, p->centre, p->vertex + 9);
        p->centre[0] /= 4.0f;
        p->centre[1] /= 4.0f;
        p->centre[2] /= 4.0f;
        Portal_GenNormale(p);

        /*
         * Portal position fix...
         */
        // X_MIN
        if((p->norm[0] > 0.999) && (((int)p->centre[0]) % 2))
        {
            float pos[3] = {1.0, 0.0, 0.0};
            Portal_Move(p, pos);
        }

        // Y_MIN
        if((p->norm[1] > 0.999) && (((int)p->centre[1]) % 2))
        {
            float pos[3] = {0.0, 1.0, 0.0};
            Portal_Move(p, pos);
        }

        // Z_MAX
        if((p->norm[2] <-0.999) && (((int)p->centre[2]) % 2))
        {
            float pos[3] = {0.0, 0.0, -1.0};
            Portal_Move(p, pos);
        }
    }

    /*
     * room borders calculation
     */
    room->bb_min[2] = tr_room->y_bottom;
    room->bb_max[2] = tr_room->y_top;

    room->bb_min[0] = room->transform[12 + 0] + TR_METERING_SECTORSIZE;
    room->bb_min[1] = room->transform[12 + 1] + TR_METERING_SECTORSIZE;
    room->bb_max[0] = room->transform[12 + 0] + TR_METERING_SECTORSIZE * room->sectors_x - TR_METERING_SECTORSIZE;
    room->bb_max[1] = room->transform[12 + 1] + TR_METERING_SECTORSIZE * room->sectors_y - TR_METERING_SECTORSIZE;

    room->obb = OBB_Create();
    room->obb->transform = room->transform;
    {
        float bb_min[3], bb_max[3];
        vec3_sub(bb_min, room->bb_min, room->transform + 12);
        vec3_sub(bb_max, room->bb_max, room->transform + 12);
        OBB_Rebuild(room->obb, bb_min, bb_max);
        OBB_Transform(room->obb);
    }
    /*
     * alternate room pointer calculation if one exists.
     */
    room->alternate_room_next = NULL;
    room->alternate_room_prev = NULL;
    room->real_room = room;
    if((tr_room->alternate_room >= 0) && ((uint32_t)tr_room->alternate_room < tr->rooms_count))
    {
        room->alternate_room_next = global_world.rooms + tr_room->alternate_room;
    }

    /*
     *  let us load static room meshes
     */
    room->content->static_mesh_count = tr_room->num_static_meshes;
    if(room->content->static_mesh_count)
    {
        room->content->static_mesh = (static_mesh_p)calloc(room->content->static_mesh_count, sizeof(static_mesh_t));
    }

    r_static = room->content->static_mesh;
    for(uint16_t i = 0; i < tr_room->num_static_meshes; i++, r_static++)
    {
        tr_static = tr->find_staticmesh_id(tr_room->static_meshes[i].object_id);
        if(tr_static == NULL)
        {
            room->content->static_mesh_count--;
            r_static--;
            continue;
        }
        TR_vertex_to_arr(r_static->pos, &tr_room->static_meshes[i].pos);
        r_static->self = Container_Create();
        r_static->self->room = room;
        r_static->self->sector = Room_GetSectorRaw(room, r_static->pos);
        r_static->self->object = room->content->static_mesh + i;
        r_static->self->object_type = OBJECT_STATIC_MESH;
        r_static->self->collision_group = COLLISION_GROUP_STATIC_OBLECT;
        r_static->self->collision_mask = COLLISION_MASK_ALL;
        r_static->object_id = tr_room->static_meshes[i].object_id;
        r_static->mesh = global_world.meshes + tr->mesh_indices[tr_static->mesh];

        r_static->rot[0] = tr_room->static_meshes[i].rotation;
        r_static->rot[1] = 0.0;
        r_static->rot[2] = 0.0;
        r_static->tint[0] = tr_room->static_meshes[i].tint.r * 2;
        r_static->tint[1] = tr_room->static_meshes[i].tint.g * 2;
        r_static->tint[2] = tr_room->static_meshes[i].tint.b * 2;
        r_static->tint[3] = tr_room->static_meshes[i].tint.a * 2;
        r_static->obb = OBB_Create();

        r_static->cbb_min[0] = tr_static->collision_box[0].x;
        r_static->cbb_min[1] =-tr_static->collision_box[0].z;
        r_static->cbb_min[2] = tr_static->collision_box[1].y;
        r_static->cbb_max[0] = tr_static->collision_box[1].x;
        r_static->cbb_max[1] =-tr_static->collision_box[1].z;
        r_static->cbb_max[2] = tr_static->collision_box[0].y;

        r_static->vbb_min[0] = tr_static->visibility_box[0].x;
        r_static->vbb_min[1] =-tr_static->visibility_box[0].z;
        r_static->vbb_min[2] = tr_static->visibility_box[1].y;
        r_static->vbb_max[0] = tr_static->visibility_box[1].x;
        r_static->vbb_max[1] =-tr_static->visibility_box[1].z;
        r_static->vbb_max[2] = tr_static->visibility_box[0].y;

        r_static->obb->transform = r_static->transform;
        r_static->obb->radius = r_static->mesh->radius;
        Mat4_E(r_static->transform);
        Mat4_Translate(r_static->transform, r_static->pos);
        float ang = r_static->rot[0] * M_PI / 180.0f;
        Mat4_RotateZ_SinCos(r_static->transform, sinf(ang), cosf(ang));
        OBB_Rebuild(r_static->obb, r_static->vbb_min, r_static->vbb_max);
        OBB_Transform(r_static->obb);

        r_static->physics_body = NULL;
        r_static->hide = 0;

        // Disable static mesh collision, if flag value is 3 (TR1) or all bounding box
        // coordinates are equal (TR2-5).

        if((tr_static->flags == 3) ||
           ((r_static->cbb_min[0] == r_static->cbb_min[1]) && (r_static->cbb_min[1] == r_static->cbb_min[2]) &&
            (r_static->cbb_max[0] == r_static->cbb_max[1]) && (r_static->cbb_max[1] == r_static->cbb_max[2])))
        {
            r_static->self->collision_group = COLLISION_NONE;
        }

        // Set additional static mesh properties from level script override.

        World_SetStaticMeshProperties(r_static);

        // Set static mesh collision.
        Physics_GenStaticMeshRigidBody(r_static);
    }

    /*
     * sprites loading section
     */
    room->content->sprites_count = tr_room->num_sprites;
    if(room->content->sprites_count != 0)
    {
        uint32_t actual_sprites_count = 0;
        room->content->sprites = (room_sprite_p)calloc(room->content->sprites_count, sizeof(room_sprite_t));
        for(uint32_t i = 0; i < room->content->sprites_count; i++)
        {
            if((tr_room->sprites[i].texture >= 0) && ((uint32_t)tr_room->sprites[i].texture < global_world.sprites_count))
            {
                room_sprite_p rs = room->content->sprites + actual_sprites_count;
                rs->sprite = global_world.sprites + tr_room->sprites[i].texture;
                TR_vertex_to_arr(rs->pos, &tr_room->vertices[tr_room->sprites[i].vertex].vertex);
                vec3_add(rs->pos, rs->pos, room->transform + 12);
                actual_sprites_count++;
            }
        }
        room->content->sprites_count = actual_sprites_count;
        if(actual_sprites_count == 0)
        {
            free(room->content->sprites);
            room->content->sprites = NULL;
        }
    }
}


void World_GenRooms(class VT_Level *tr)
{
    global_world.rooms_count = tr->rooms_count;
    room_p r = global_world.rooms = (room_p)malloc(global_world.rooms_count * sizeof(room_t));
    for(uint32_t i = 0; i < global_world.rooms_count; i++, r++)
    {
        r->id = i;
        World_GenRoom(r, tr);
    }
}


void World_GenRoomFlipMap()
{
    // Flipmap count is hardcoded, as no original levels contain such info.
    global_world.flip_count = FLIPMAP_MAX_NUMBER;

    global_world.flip_map   = (uint8_t*)malloc(global_world.flip_count * sizeof(uint8_t));
    global_world.flip_state = (uint8_t*)malloc(global_world.flip_count * sizeof(uint8_t));

    memset(global_world.flip_map,   0, global_world.flip_count);
    memset(global_world.flip_state, 0, global_world.flip_count);
}


void World_GenSkeletalModels(class VT_Level *tr)
{
    skeletal_model_p smodel;
    tr_moveable_t *tr_moveable;

    global_world.skeletal_models_count = tr->moveables_count;
    smodel = global_world.skeletal_models = (skeletal_model_p)calloc(global_world.skeletal_models_count, sizeof(skeletal_model_t));

    for(uint32_t i = 0; i < global_world.skeletal_models_count; i++, smodel++)
    {
        tr_moveable = &tr->moveables[i];
        smodel->id = tr_moveable->object_id;
        smodel->mesh_count = tr_moveable->num_meshes;
        TR_GenSkeletalModel(smodel, i, global_world.meshes, tr);
        SkeletalModel_FillTransparency(smodel);
    }
}


void World_GenEntities(class VT_Level *tr)
{
    int top;
    tr2_item_t *tr_item;
    entity_p entity;

    for(uint32_t i = 0; i < tr->items_count; i++)
    {
        tr_item = &tr->items[i];
        entity = Entity_Create();
        entity->id = i;
        TR_vertex_to_arr(entity->transform + 12, &tr_item->pos);
        entity->angles[0] = tr_item->rotation;
        entity->angles[1] = 0.0f;
        entity->angles[2] = 0.0f;
        Entity_UpdateTransform(entity);
        if((tr_item->room >= 0) && ((uint32_t)tr_item->room < global_world.rooms_count))
        {
            entity->self->room = global_world.rooms + tr_item->room;
            entity->self->sector = Room_GetSectorRaw(entity->self->room, entity->transform + 12);
        }
        else
        {
            entity->self->room = NULL;
            entity->self->sector = NULL;
        }

        entity->trigger_layout  = (tr_item->flags & 0x3E00) >> 9;               ///@FIXME: Ignore CLEAR BODY flags for a moment.
        entity->OCB             = tr_item->ocb;
        entity->timer           = 0.0;
        entity->state_flags &= (tr_item->flags & 0x0100) ? (0x0000) : (0xFFFF);

        entity->self->collision_group = COLLISION_GROUP_KINEMATIC;
        entity->self->collision_mask  = COLLISION_MASK_ALL;
        entity->self->collision_shape = COLLISION_SHAPE_TRIMESH;
        entity->move_type             = MOVE_STATIC_POS;

        entity->bf->animations.model = World_GetModelByID(tr_item->object_id);

        if(engine_lua)
        {
            if(entity->bf->animations.model == NULL)
            {
                top = lua_gettop(engine_lua);                                   // save LUA stack
                lua_getglobal(engine_lua, "getOverridedID");                    // add to the up of stack LUA's function
                lua_pushinteger(engine_lua, tr->game_version);                  // add to stack first argument
                lua_pushinteger(engine_lua, tr_item->object_id);                // add to stack second argument
                if(lua_CallAndLog(engine_lua, 2, 1, 0))                         // call that function
                {
                    entity->bf->animations.model = World_GetModelByID(lua_tointeger(engine_lua, -1));
                }
                lua_settop(engine_lua, top);                                    // restore LUA stack
            }
        }

        if(entity->bf->animations.model == NULL)
        {
            // SPRITE LOADING
            sprite_p sp = World_GetSpriteByID(tr_item->object_id);
            if(sp && entity->self->room)
            {
                room_sprite_p rsp;
                int sz = ++entity->self->room->content->sprites_count;
                entity->self->room->content->sprites = (room_sprite_p)realloc(entity->self->room->content->sprites, sz * sizeof(room_sprite_t));
                rsp = entity->self->room->content->sprites + sz - 1;
                rsp->sprite = sp;
                rsp->pos[0] = entity->transform[12 + 0];
                rsp->pos[1] = entity->transform[12 + 1];
                rsp->pos[2] = entity->transform[12 + 2];
            }

            Entity_Delete(entity);
            continue;                                                           // that entity has no model. may be it is a some trigger or look at object
        }

        SSBoneFrame_CreateFromModel(entity->bf, entity->bf->animations.model);
        entity->bf->transform = entity->transform;

        Entity_SetAnimation(entity, ANIM_TYPE_BASE, 0, 0);                      // Set zero animation and zero frame
        Entity_RebuildBV(entity);
        Room_AddObject(entity->self->room, entity->self);
        entity->self->sector = Room_GetSectorRaw(entity->self->room, entity->transform + 12);
        World_AddEntity(entity);
        World_SetEntityModelProperties(entity);
        Physics_GenRigidBody(entity->physics, entity->bf);
        Entity_UpdateRigidBody(entity, 1);

        if(!(entity->state_flags & ENTITY_STATE_ENABLED) || (entity->self->collision_group == COLLISION_NONE))
        {
            Entity_DisableCollision(entity);
        }
    }
}


void World_GenBaseItems()
{
    Script_CallVoidFunc(engine_lua, "genBaseItems");
}


void World_GenSpritesBuffer()
{
    for (uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        Room_GenSpritesBuffer(&global_world.rooms[i]);
    }
}


static room_p WorldRoom_FindRealRoomInSequence(room_p room)
{
    room_p room_with_min_id = room;

    for(uint16_t i = 0; i < room->content->portals_count; ++i)
    {
        room_p outer_room = room->content->portals[i].dest_room;
        for(uint16_t j = 0; j < outer_room->content->portals_count; ++j)
        {
            room_p real_room = outer_room->content->portals[j].dest_room;
            if(room == real_room)
            {
                return real_room;
            }

            for(room_p room_it = room->alternate_room_prev; room_it; room_it = room_it->alternate_room_prev)
            {
                if(room_it == real_room)
                {
                    return real_room;
                }
                if(room_it == room)
                {
                    break;
                }
            }

            for(room_p room_it = room->alternate_room_next; room_it; room_it = room_it->alternate_room_next)
            {
                if(room_it == real_room)
                {
                    return real_room;
                }
                if(room_it == room)
                {
                    break;
                }
            }
        }
    }

    if(!room->alternate_room_prev)
    {
        return room;
    }

    for(room_p room_it = room->alternate_room_prev; room_it; room_it = room_it->alternate_room_prev)
    {
        if(!room_it->alternate_room_prev)
        {
            return room_it;
        }
        if(room_with_min_id->id > room_it->id)
        {
            room_with_min_id = room_it;
        }
        if(room_it == room)
        {
            break;
        }
    }

    for(room_p room_it = room->alternate_room_next; room_it; room_it = room_it->alternate_room_next)
    {
        if(room_with_min_id->id > room_it->id)
        {
            room_with_min_id = room_it;
        }
        if(room_it == room)
        {
            break;
        }
    }

    return room_with_min_id;
}


void World_GenRoomProperties(class VT_Level *tr)
{
    for(uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        room_p r = global_world.rooms + i;
        if(r->alternate_room_next)
        {
            r->real_room = NULL;
            r->alternate_room_next->real_room = NULL;                           // HACK for next real room calculation
            r->alternate_room_next->alternate_room_prev = r;                    // fill base room pointers
        }
    }

    for(uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        room_p r = global_world.rooms + i;
        if(!r->real_room)
        {
            room_p real_room = WorldRoom_FindRealRoomInSequence(r);             // call it once per alt rooms sequence
            r->real_room = real_room;
            for(room_p room_it = r->alternate_room_next; room_it; room_it = room_it->alternate_room_next)
            {
                room_it->real_room = real_room;
                if(room_it == r)
                {
                    break;
                }
            }
            for(room_p room_it = r->alternate_room_prev; room_it; room_it = room_it->alternate_room_prev)
            {
                room_it->real_room = real_room;
                if(room_it == r)
                {
                    break;
                }
            }
        }
    }

    for(uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        room_p r = global_world.rooms + i;
        // Fill heightmap and translate floordata.
        for(uint32_t j = 0; j < r->sectors_count; j++)
        {
            room_sector_p rs = r->content->sectors + j;
            Res_Sector_TranslateFloorData(global_world.rooms, global_world.rooms_count, rs, tr);
            for(trigger_command_p cmd = (rs->trigger) ? (rs->trigger->commands) : (NULL); cmd; cmd = cmd->next)
            {
                if(cmd->function == TR_FD_TRIGFUNC_PLAYTRACK)
                {
                    Audio_CacheTrack(cmd->operands);
                }
            }
        }

        // Basic sector calculations.
        Res_RoomSectorsCalculate(global_world.rooms, global_world.rooms_count, i, tr);
    }

    for(uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        // Generate links to the overlapped rooms.
        World_BuildOverlappedRoomsList(global_world.rooms + i);
        // Generate links to the near rooms.
        World_BuildNearRoomsList(global_world.rooms + i);
    }

    for(uint32_t i = 0; i < global_world.rooms_count; i++)
    {
        room_p r = global_world.rooms + i;
        for(uint16_t j = 0; j < r->content->near_room_list_size; ++j)
        {
            Room_AddToNearRoomsList(r->content->near_room_list[j], r);
        }
    }
}


void World_GenRoomCollision()
{
    room_p r = global_world.rooms;

    if(r == NULL)
    {
        return;
    }

    for(uint32_t i = 0; i < global_world.rooms_count; i++, r++)
    {
        // Inbetween polygons array is later filled by loop which scans adjacent
        // sector heightmaps and fills the gaps between them, thus creating inbetween
        // polygon. Inbetweens can be either quad (if all four corner heights are
        // different), triangle (if one corner height is similar to adjacent) or
        // ghost (if corner heights are completely similar). In case of quad inbetween,
        // two triangles are added to collisional trimesh, in case of triangle inbetween,
        // we add only one, and in case of ghost inbetween, we ignore it.

        int num_tweens = r->sectors_count * 4;
        size_t buff_size = num_tweens * sizeof(sector_tween_t);
        sector_tween_p room_tween = (sector_tween_p)Sys_GetTempMem(buff_size);

        // Clear tween array.

        for(int j = 0; j < num_tweens; j++)
        {
            room_tween[j].ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
            room_tween[j].floor_tween_type   = TR_SECTOR_TWEEN_TYPE_NONE;
        }

        // Most difficult task with converting floordata collision to trimesh collision is
        // building inbetween polygons which will block out gaps between sector heights.
        num_tweens = Res_Sector_GenStaticTweens(r, room_tween);

        // Final step is sending actual sectors to Bullet collision model. We do it here.
        r->content->physics_body = Physics_GenRoomRigidBody(r, r->content->sectors, r->sectors_count, room_tween, num_tweens);
        r->self->collision_group = COLLISION_GROUP_STATIC_ROOM;                 // meshtree
        r->self->collision_shape = COLLISION_SHAPE_TRIMESH;

        Sys_ReturnTempMem(buff_size);
    }
}


void World_FixRooms()
{
    room_p r = global_world.rooms;

    if(r == NULL)
    {
        return;
    }

    for(uint32_t i = 0; i < global_world.rooms_count; i++, r++)
    {
        if(r->real_room != r)
        {
            Room_Disable(r);
        }
    }
}
