
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "core/gl_util.h"
#include "core/console.h"
#include "core/system.h"
#include "core/redblack.h"
#include "core/vmath.h"
#include "core/polygon.h"
#include "core/obb.h"
#include "render/camera.h"
#include "render/frustum.h"
#include "render/render.h"
#include "render/bordered_texture_atlas.h"
#include "vt/vt_level.h"
#include "audio.h"
#include "room.h"
#include "world.h"
#include "mesh.h"
#include "skeletal_model.h"
#include "entity.h"
#include "character_controller.h"
#include "anim_state_control.h"
#include "engine.h"
#include "engine_lua.h"
#include "engine_physics.h"
#include "script.h"
#include "gui.h"
#include "gameflow.h"
#include "resource.h"
#include "inventory.h"


void World_GenRBTrees(struct world_s *world);
int  compEntityEQ(void *x, void *y);
int  compEntityLT(void *x, void *y);
void RBEntityFree(void *x);
void RBItemFree(void *x);
void World_SwapRoomPortals(world_p world, struct room_s *room, struct room_s *dest_room);

// private load level functions prototipes:
void World_SetEntityModelProperties(world_p world, struct entity_s *ent);
void World_SetStaticMeshProperties(world_p world, struct static_mesh_s *r_static);
void World_SetEntityFunction(world_p world, struct entity_s *ent);
void World_GenEntityFunctions(world_p world, struct RedBlackNode_s *x);
void World_ScriptsOpen(world_p world);
void World_ScriptsClose(world_p world);
void World_AutoexecOpen(world_p world);
// Create entity function from script, if exists.
bool Res_CreateEntityFunc(lua_State *lua, const char* func_name, int entity_id);


void World_GenTextures(struct world_s *world, class VT_Level *tr);
void World_GenAnimCommands(struct world_s *world, class VT_Level *tr);
void World_GenAnimTextures(struct world_s *world, class VT_Level *tr);
void World_GenMeshes(struct world_s *world, class VT_Level *tr);
void World_GenSprites(struct world_s *world, class VT_Level *tr);
void World_GenBoxes(struct world_s *world, class VT_Level *tr);
void World_GenCameras(struct world_s *world, class VT_Level *tr);
void World_GenRoom(struct world_s *world, struct room_s *room, class VT_Level *tr);
void World_GenRooms(struct world_s *world, class VT_Level *tr);
void World_GenRoomFlipMap(struct world_s *world);
void World_GenSkeletalModels(struct world_s *world, class VT_Level *tr);
void World_GenEntities(struct world_s *world, class VT_Level *tr);
void World_GenBaseItems(struct world_s* world);
void World_GenSpritesBuffer(struct world_s *world);
void World_GenRoomProperties(struct world_s *world, class VT_Level *tr);
void World_GenRoomCollision(struct world_s *world);
void World_GenSamples(struct world_s *world, class VT_Level *tr);
void World_FixRooms(struct world_s *world);
void World_MakeEntityItems(world_p world, struct RedBlackNode_s *n);            // Assign pickup functions to previously created base items.

void World_Prepare(world_p world)
{
    world->id = 0;
    world->name = NULL;
    world->type = 0x00;
    world->meshes = NULL;
    world->meshes_count = 0;
    world->sprites = NULL;
    world->sprites_count = 0;
    world->room_count = 0;
    world->rooms = 0;
    world->flip_map = NULL;
    world->flip_state = NULL;
    world->flip_count = 0;
    world->textures = NULL;
    world->type = 0;
    world->entity_tree = NULL;
    world->items_tree = NULL;
    world->Character = NULL;

    world->audio_sources = NULL;
    world->audio_sources_count = 0;
    world->audio_buffers = NULL;
    world->audio_buffers_count = 0;
    world->audio_effects = NULL;
    world->audio_effects_count = 0;
    world->anim_sequences = NULL;
    world->anim_sequences_count = 0;
    world->stream_tracks = NULL;
    world->stream_tracks_count = 0;
    world->stream_track_map = NULL;
    world->stream_track_map_count = 0;

    world->tex_count = 0;
    world->textures = 0;
    world->room_boxes = NULL;
    world->room_box_count = 0;
    world->cameras_sinks = NULL;
    world->cameras_sinks_count = 0;
    world->skeletal_models = NULL;
    world->skeletal_model_count = 0;
    world->sky_box = NULL;
    world->anim_commands = NULL;
    world->anim_commands_count = 0;

    world->objects_flags_conf = NULL;
    world->ent_ID_override = NULL;
    world->level_script = NULL;
}


void World_Open(world_p world, class VT_Level *tr)
{
    World_Clear(world);

    world->version = tr->game_version;

    World_ScriptsOpen(world);           // Open configuration scripts.
    Gui_DrawLoadScreen(150);

    World_GenRBTrees(world);            // Generate red-black trees
    Gui_DrawLoadScreen(200);

    World_GenTextures(world, tr);       // Generate OGL textures
    Gui_DrawLoadScreen(300);

    World_GenAnimCommands(world, tr);   // Copy anim commands
    Gui_DrawLoadScreen(310);

    World_GenAnimTextures(world, tr);   // Generate animated textures
    Gui_DrawLoadScreen(320);

    World_GenMeshes(world, tr);         // Generate all meshes
    Gui_DrawLoadScreen(400);

    World_GenSprites(world, tr);        // Generate all sprites
    Gui_DrawLoadScreen(420);

    World_GenBoxes(world, tr);          // Generate boxes.
    Gui_DrawLoadScreen(440);

    World_GenCameras(world, tr);        // Generate cameras & sinks.
    Gui_DrawLoadScreen(460);

    World_GenRooms(world, tr);          // Build all rooms
    Gui_DrawLoadScreen(500);

    World_GenRoomFlipMap(world);        // Generate room flipmaps
    Gui_DrawLoadScreen(520);

    // Build all skeletal models. Must be generated before TR_Sector_Calculate() function.
    World_GenSkeletalModels(world, tr);
    Gui_DrawLoadScreen(600);

    World_GenEntities(world, tr);        // Build all moveables (entities)
    Gui_DrawLoadScreen(650);

    World_GenBaseItems(world);           // Generate inventory item entries.
    Gui_DrawLoadScreen(680);

    // Generate sprite buffers. Only now because entity generation adds new sprites
    World_GenSpritesBuffer(world);
    Gui_DrawLoadScreen(700);

    World_GenRoomProperties(world, tr);
    Gui_DrawLoadScreen(750);

    World_GenRoomCollision(world);
    Gui_DrawLoadScreen(800);

    // Initialize audio.
    World_GenSamples(world, tr);
    Gui_DrawLoadScreen(850);

    // Find and set skybox.
    world->sky_box = World_GetSkybox(world);
    Gui_DrawLoadScreen(860);

    // Generate entity functions.

    if(world->entity_tree->root)
    {
        World_GenEntityFunctions(world, world->entity_tree->root);
    }
    Gui_DrawLoadScreen(910);

    // Load entity collision flags and ID overrides from script.

    World_ScriptsClose(world);
    Gui_DrawLoadScreen(940);

    // Process level autoexec loading.
    World_AutoexecOpen(world);
    Gui_DrawLoadScreen(960);

    // Fix initial room states
    World_FixRooms(world);
    Gui_DrawLoadScreen(970);

    if(world->tex_atlas)
    {
        delete world->tex_atlas;
        world->tex_atlas = NULL;
    }
}


void World_Clear(world_p world)
{
    extern engine_container_p last_cont;

    last_cont = NULL;
    Engine_LuaClearTasks();
    // De-initialize and destroy all audio objects.
    Audio_DeInit();

    if(main_inventory_manager != NULL)
    {
        main_inventory_manager->setInventory(NULL);
        main_inventory_manager->setItemsType(1);                                // see base items
    }

    if(world->Character != NULL)
    {
        world->Character->self->room = NULL;
        world->Character->self->next = NULL;
        world->Character->current_sector = NULL;
    }

    /* entity empty must be done before rooms destroy */
    RB_Free(world->entity_tree);
    world->entity_tree = NULL;

    /* Now we can delete physics misc objects */
    Physics_CleanUpObjects();

    for(uint32_t i=0;i<world->room_count;i++)
    {
        Room_Clear(world->rooms+i);
    }
    free(world->rooms);
    world->rooms = NULL;

    free(world->flip_map);
    free(world->flip_state);
    world->flip_map = NULL;
    world->flip_state = NULL;
    world->flip_count = 0;

    if(world->room_box_count)
    {
        free(world->room_boxes);
        world->room_boxes = NULL;
        world->room_box_count = 0;
    }

    if(world->cameras_sinks_count)
    {
        free(world->cameras_sinks);
        world->cameras_sinks = NULL;
        world->cameras_sinks_count = 0;
    }

    /*sprite empty*/
    if(world->sprites_count)
    {
        free(world->sprites);
        world->sprites = NULL;
        world->sprites_count = 0;
    }

    /*items empty*/
    RB_Free(world->items_tree);
    world->items_tree = NULL;

    if(world->Character)
    {
        Entity_Clear(world->Character);
        free(world->Character);
        world->Character = NULL;
    }

    if(world->skeletal_model_count)
    {
        for(uint32_t i=0;i<world->skeletal_model_count;i++)
        {
            SkeletalModel_Clear(world->skeletal_models+i);
        }
        free(world->skeletal_models);
        world->skeletal_models = NULL;
        world->skeletal_model_count = 0;
    }

    /*mesh empty*/
    if(world->meshes_count)
    {
        for(uint32_t i=0;i<world->meshes_count;i++)
        {
            BaseMesh_Clear(world->meshes+i);
        }
        free(world->meshes);
        world->meshes = NULL;
        world->meshes_count = 0;
    }

    if(world->tex_count)
    {
        qglDeleteTextures(world->tex_count, world->textures);
        world->tex_count = 0;
        free(world->textures);
        world->textures = NULL;
    }

    if(world->tex_atlas)
    {
        delete world->tex_atlas;
        world->tex_atlas = NULL;
    }

    if(world->anim_sequences_count)
    {
        for(uint32_t i=0;i < world->anim_sequences_count;i++)
        {
            if(world->anim_sequences[i].frames_count != 0)
            {
                free(world->anim_sequences[i].frame_list);
                world->anim_sequences[i].frame_list = NULL;
                free(world->anim_sequences[i].frames);
                world->anim_sequences[i].frames = NULL;
            }
            world->anim_sequences[i].frames_count = 0;
        }
        world->anim_sequences_count = 0;
        free(world->anim_sequences);
        world->anim_sequences = NULL;
    }
}


uint32_t World_SpawnEntity(world_p world, uint32_t model_id, uint32_t room_id, float pos[3], float ang[3], int32_t id)
{
    if(world && world->entity_tree)
    {
        skeletal_model_p model = World_GetModelByID(world, model_id);
        if(model != NULL)
        {
            entity_p ent = World_GetEntityByID(world, id);
            RedBlackNode_p node = world->entity_tree->root;

            if(ent != NULL)
            {
                if(pos != NULL)
                {
                    vec3_copy(ent->transform+12, pos);
                }
                if(ang != NULL)
                {
                    vec3_copy(ent->angles, ang);
                    Entity_UpdateTransform(ent);
                }
                if(room_id < world->room_count)
                {
                    ent->self->room = world->rooms + room_id;
                    ent->current_sector = Room_GetSectorRaw(ent->self->room, ent->transform+12);
                }
                else
                {
                    ent->self->room = NULL;
                }

                return ent->id;
            }

            ent = Entity_Create();

            if(id < 0)
            {
                ent->id = 0;
                while(node != NULL)
                {
                    ent->id = *((uint32_t*)node->key) + 1;
                    node = node->right;
                }
            }
            else
            {
                ent->id = id;
            }

            if(pos != NULL)
            {
                vec3_copy(ent->transform+12, pos);
            }
            if(ang != NULL)
            {
                vec3_copy(ent->angles, ang);
                Entity_UpdateTransform(ent);
            }
            if(room_id < world->room_count)
            {
                ent->self->room = world->rooms + room_id;
                ent->current_sector = Room_GetSectorRaw(ent->self->room, ent->transform + 12);
            }
            else
            {
                ent->self->room = NULL;
            }

            ent->type_flags     = ENTITY_TYPE_SPAWNED;
            ent->state_flags    = ENTITY_STATE_ENABLED | ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
            ent->trigger_layout = 0x00;
            ent->OCB            = 0x00;
            ent->timer          = 0.0;

            ent->self->collision_type = COLLISION_NONE;
            ent->self->collision_shape = COLLISION_SHAPE_TRIMESH;
            ent->move_type          = 0x0000;
            ent->inertia_linear     = 0.0;
            ent->inertia_angular[0] = 0.0;
            ent->inertia_angular[1] = 0.0;
            ent->move_type          = 0;

            SSBoneFrame_CreateFromModel(ent->bf, model);
            Entity_SetAnimation(ent, 0, 0);                                     // Set zero animation and zero frame
            Physics_GenRigidBody(ent->physics, ent->bf, ent->transform);

            Entity_RebuildBV(ent);
            if(ent->self->room != NULL)
            {
                Room_AddObject(ent->self->room, ent->self);
            }
            World_AddEntity(world, ent);

            return ent->id;
        }
    }

    return 0xFFFFFFFF;
}


struct entity_s *World_GetEntityByID(world_p world, uint32_t id)
{
    entity_p ent = NULL;
    RedBlackNode_p node;

    if((world == NULL) || (world->entity_tree == NULL))
    {
        return NULL;
    }

    if((world->Character != NULL) && (world->Character->id == id))
    {
        return world->Character;
    }

    node = RB_SearchNode(&id, world->entity_tree);
    if(node != NULL)
    {
        ent = (entity_p)node->data;
    }

    return ent;
}


struct base_item_s *World_GetBaseItemByID(world_p world, uint32_t id)
{
    base_item_p item = NULL;
    RedBlackNode_p node;

    if((world == NULL) || (world->items_tree == NULL))
    {
        return NULL;
    }

    node = RB_SearchNode(&id, world->items_tree);
    if(node != NULL)
    {
        item = (base_item_p)node->data;
    }

    return item;
}


int World_AddEntity(world_p world, struct entity_s *entity)
{
    RB_InsertIgnore(&entity->id, entity, world->entity_tree);
    return 1;
}


int World_DeleteEntity(world_p world, struct entity_s *entity)
{
    RB_Delete(world->entity_tree, RB_SearchNode(&entity->id, world->entity_tree));
    return 1;
}


int World_CreateItem(world_p world, uint32_t item_id, uint32_t model_id, uint32_t world_model_id, uint16_t type, uint16_t count, const char *name)
{
    skeletal_model_p model = World_GetModelByID(world, model_id);
    if((model == NULL) || (world->items_tree == NULL))
    {
        return 0;
    }

    ss_bone_frame_p bf = (ss_bone_frame_p)malloc(sizeof(ss_bone_frame_t));
    SSBoneFrame_CreateFromModel(bf, model);

    base_item_p item = (base_item_p)malloc(sizeof(base_item_t));
    item->id = item_id;
    item->world_model_id = world_model_id;
    item->type = type;
    item->count = count;
    item->name[0] = 0;
    if(name)
    {
        strncpy(item->name, name, 64);
    }
    item->bf = bf;

    RB_InsertReplace(&item->id, item, world->items_tree);

    return 1;
}


int World_DeleteItem(world_p world, uint32_t item_id)
{
    RB_Delete(world->items_tree, RB_SearchNode(&item_id, world->items_tree));
    return 1;
}


struct sprite_s *World_GetSpriteByID(world_p world, uint32_t ID)
{
    sprite_p sp = world->sprites;
    for(uint32_t i=0;i<world->sprites_count;i++,sp++)
    {
        if(sp->id == ID)
        {
            return sp;
        }
    }

    return NULL;
}


struct skeletal_model_s *World_GetModelByID(world_p world, uint32_t id)
{
    long int i, min, max;

    min = 0;
    max = world->skeletal_model_count - 1;
    if(world->skeletal_models[min].id == id)
    {
        return world->skeletal_models + min;
    }
    if(world->skeletal_models[max].id == id)
    {
        return world->skeletal_models + max;
    }
    do
    {
        i = (min + max) / 2;
        if(world->skeletal_models[i].id == id)
        {
            return world->skeletal_models + i;
        }

        if(world->skeletal_models[i].id < id)
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


struct skeletal_model_s* World_GetSkybox(world_p world)
{
    switch(world->version)
    {
        case TR_II:
        case TR_II_DEMO:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR2);

        case TR_III:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR3);

        case TR_IV:
        case TR_IV_DEMO:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR4);

        case TR_V:
            return World_GetModelByID(world, TR_ITEM_SKYBOX_TR5);

        default:
            return NULL;
    };
}


struct room_s *World_FindRoomByPos(world_p world, float pos[3])
{
    room_p r = world->rooms;
    for(uint32_t i = 0; i < world->room_count; i++, r++)
    {
        if(r->active &&
           (pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2]) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }
    return NULL;
}


struct room_s *World_FindRoomByPosCogerrence(world_p world, float pos[3], struct room_s *old_room)
{
    if(old_room == NULL)
    {
        return World_FindRoomByPos(world, pos);
    }

    if(old_room->active &&
       (pos[0] >= old_room->bb_min[0]) && (pos[0] < old_room->bb_max[0]) &&
       (pos[1] >= old_room->bb_min[1]) && (pos[1] < old_room->bb_max[1]))
    {
        if((pos[2] >= old_room->bb_min[2]) && (pos[2] < old_room->bb_max[2]))
        {
            return old_room;
        }
        else if(pos[2] >= old_room->bb_max[2])
        {
            room_sector_p orig_sector = Room_GetSectorRaw(old_room, pos);
            if(orig_sector->sector_above != NULL)
            {
                return Room_CheckFlip(orig_sector->sector_above->owner_room);
            }
        }
        else if(pos[2] < old_room->bb_min[2])
        {
            room_sector_p orig_sector = Room_GetSectorRaw(old_room, pos);
            if(orig_sector->sector_below != NULL)
            {
                return Room_CheckFlip(orig_sector->sector_below->owner_room);
            }
        }
    }

    room_sector_p new_sector = Room_GetSectorRaw(old_room, pos);
    if((new_sector != NULL) && new_sector->portal_to_room)
    {
        return Room_CheckFlip(new_sector->portal_to_room);
    }

    for(uint16_t i=0;i<old_room->near_room_list_size;i++)
    {
        room_p r = old_room->near_room_list[i];
        if(r->active &&
           (pos[0] >= r->bb_min[0]) && (pos[0] < r->bb_max[0]) &&
           (pos[1] >= r->bb_min[1]) && (pos[1] < r->bb_max[1]) &&
           (pos[2] >= r->bb_min[2]) && (pos[2] < r->bb_max[2]))
        {
            return r;
        }
    }

    return World_FindRoomByPos(world, pos);
}


void World_SwapRoomToBase(world_p world, struct room_s *room)
{
    if((room->base_room != NULL) && (room->active == 1))            // If room is active alternate room
    {
        renderer.CleanList();
        Room_Disable(room);                                         // Disable current room
        Room_Disable(room->base_room);                              // Paranoid
        World_SwapRoomPortals(world, room, room->base_room);        // Update portals to match this room
        Room_SwapItems(room, room->base_room);                      // Update items to match this room
        Room_Enable(room->base_room);                               // Enable original room
    }
}


void World_SwapRoomToAlternate(world_p world, struct room_s *room)
{
    if((room->alternate_room != NULL) && (room->active == 1))       // If room is active base room
    {
        renderer.CleanList();
        Room_Disable(room);                                         // Disable current room
        Room_Disable(room->alternate_room);                         // Paranoid
        World_SwapRoomPortals(world, room, room->alternate_room);   // Update portals to match this room
        Room_SwapItems(room, room->alternate_room);                 // Update items to match this room
        Room_Enable(room->alternate_room);                          // Enable base room
    }
}


void World_BuildNearRoomsList(world_p world, struct room_s *room)
{
    room->near_room_list_size = 0;

    portal_p p = room->portals;
    for(uint16_t i = 0; i < room->portals_count; i++, p++)
    {
        Room_AddToNearRoomsList(room, p->dest_room);
    }

    uint16_t nc1 = room->near_room_list_size;

    for(uint16_t i = 0; i < nc1; i++)
    {
        room_p r = room->near_room_list[i];
        p = r->portals;
        for(uint16_t j = 0; j < r->portals_count; j++, p++)
        {
            Room_AddToNearRoomsList(room, p->dest_room);
        }
    }
}


void World_BuildOverlappedRoomsList(world_p world, struct room_s *room)
{
    room->overlapped_room_list_size = 0;

    for(uint32_t i = 0; i < world->room_count; i++)
    {
        if(Room_IsOverlapped(room, world->rooms + i))
        {
            room->overlapped_room_list[room->overlapped_room_list_size] = world->rooms + i;
            room->overlapped_room_list_size++;
        }
    }
}

/*
 * PRIVATE  WORLD  FUNCTIONS
 */

void World_GenRBTrees(struct world_s *world)
{
    world->entity_tree = RB_Init();
    world->entity_tree->rb_compEQ = compEntityEQ;
    world->entity_tree->rb_compLT = compEntityLT;
    world->entity_tree->rb_free_data = RBEntityFree;

    world->items_tree = RB_Init();
    world->items_tree->rb_compEQ = compEntityEQ;
    world->items_tree->rb_compLT = compEntityLT;
    world->items_tree->rb_free_data = RBItemFree;
}


int compEntityEQ(void *x, void *y)
{
    return (*((uint32_t*)x) == *((uint32_t*)y));
}


int compEntityLT(void *x, void *y)
{
    return (*((uint32_t*)x) < *((uint32_t*)y));
}


void RBEntityFree(void *x)
{
    Entity_Clear((entity_p)x);
    free(x);
}


void RBItemFree(void *x)
{
    free(((base_item_p)x)->bf->bone_tags);
    free(((base_item_p)x)->bf);
    free(x);
}


void World_SwapRoomPortals(world_p world, struct room_s *room, struct room_s *dest_room)
{
    //Update portals in room rooms
    for(uint32_t i = 0; i < world->room_count; i++)//For every room in the world itself
    {
        for(uint16_t j = 0; j < world->rooms[i].portals_count; j++)//For every portal in this room
        {
            if(world->rooms[i].portals[j].dest_room->id == room->id)//If a portal is linked to the input room
            {
                world->rooms[i].portals[j].dest_room = dest_room;//The portal destination room is the destination room!
                //Con_Printf("The current room %d! has room %d joined to it!", room->id, i);
            }
        }
        World_BuildNearRoomsList(world, world->rooms + i); //Rebuild room near list!
    }
}

/*
 * Load level functions
 */
/// that scripts will be enabled after world -> singleton
/*int lua_SetSectorFloorConfig(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 10)
    {
        Con_AddLine("Wrong arguments number, must be (room_id, index_x, index_y, penetration_config, diagonal_type, floor, z0, z1, z2, z3)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(!lua_isnil(lua, 4))  rs->floor_penetration_config = lua_tointeger(lua, 4);
    if(!lua_isnil(lua, 5))  rs->floor_diagonal_type = lua_tointeger(lua, 5);
    if(!lua_isnil(lua, 6))  rs->floor = lua_tonumber(lua, 6);
    rs->floor_corners[0][2] = lua_tonumber(lua, 7);
    rs->floor_corners[1][2] = lua_tonumber(lua, 8);
    rs->floor_corners[2][2] = lua_tonumber(lua, 9);
    rs->floor_corners[3][2] = lua_tonumber(lua, 10);

    return 0;
}

int lua_SetSectorCeilingConfig(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 10)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, penetration_config, diagonal_type, ceiling, z0, z1, z2, z3)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(!lua_isnil(lua, 4))  rs->ceiling_penetration_config = lua_tointeger(lua, 4);
    if(!lua_isnil(lua, 5))  rs->ceiling_diagonal_type = lua_tointeger(lua, 5);
    if(!lua_isnil(lua, 6))  rs->ceiling = lua_tonumber(lua, 6);
    rs->ceiling_corners[0][2] = lua_tonumber(lua, 7);
    rs->ceiling_corners[1][2] = lua_tonumber(lua, 8);
    rs->ceiling_corners[2][2] = lua_tonumber(lua, 9);
    rs->ceiling_corners[3][2] = lua_tonumber(lua, 10);

    return 0;
}

int lua_SetSectorPortal(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 4)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, portal_room_id)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    uint32_t p = lua_tointeger(lua, 4);
    if(p < engine_world.room_count)
    {
        rs->portal_to_room = engine_world.rooms + p;
    }

    return 0;
}

int lua_SetSectorFlags(lua_State * lua)
{
    int id, sx, sy, top;

    top = lua_gettop(lua);

    if(top < 7)
    {
        Con_AddLine("wrong arguments number, must be (room_id, index_x, index_y, fp_flag, ft_flag, cp_flag, ct_flag)", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    id = lua_tointeger(lua, 1);
    sx = lua_tointeger(lua, 2);
    sy = lua_tointeger(lua, 3);
    room_sector_p rs = TR_GetRoomSector(id, sx, sy);
    if(rs == NULL)
    {
        Con_AddLine("wrong sector info", FONTSTYLE_CONSOLE_WARNING);
        return 0;
    }

    if(!lua_isnil(lua, 4))  rs->floor_penetration_config = lua_tointeger(lua, 4);
    if(!lua_isnil(lua, 5))  rs->floor_diagonal_type = lua_tointeger(lua, 5);
    if(!lua_isnil(lua, 6))  rs->ceiling_penetration_config = lua_tointeger(lua, 6);
    if(!lua_isnil(lua, 7))  rs->ceiling_diagonal_type = lua_tointeger(lua, 7);

    return 0;
}*/


void World_ScriptsOpen(world_p world)
{
    char temp_script_name[256];
    Engine_GetLevelScriptName(world->version, temp_script_name, NULL);

    world->level_script = luaL_newstate();
    if(world->level_script)
    {
        luaL_openlibs(world->level_script);
        lua_register(world->level_script, "print", lua_print);
        //lua_register(world->level_script, "setSectorFloorConfig", lua_SetSectorFloorConfig);
        //lua_register(world->level_script, "setSectorCeilingConfig", lua_SetSectorCeilingConfig);
        //lua_register(world->level_script, "setSectorPortal", lua_SetSectorPortal);
        //lua_register(world->level_script, "setSectorFlags", lua_SetSectorFlags);

        luaL_dofile(world->level_script, "scripts/staticmesh/staticmesh_script.lua");

        if(Sys_FileFound(temp_script_name, 0))
        {
            int lua_err = luaL_dofile(world->level_script, temp_script_name);
            if(lua_err)
            {
                Sys_DebugLog("lua_out.txt", "%s", lua_tostring(world->level_script, -1));
                lua_pop(world->level_script, 1);
                lua_close(world->level_script);
                world->level_script = NULL;
            }
        }
    }

    world->objects_flags_conf = luaL_newstate();
    if(world->objects_flags_conf)
    {
        luaL_openlibs(world->objects_flags_conf);
        int lua_err = luaL_loadfile(world->objects_flags_conf, "scripts/entity/entity_properties.lua");
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(world->objects_flags_conf, -1));
            lua_pop(world->objects_flags_conf, 1);
            lua_close(world->objects_flags_conf);
            world->objects_flags_conf = NULL;
        }
        lua_err = lua_pcall(world->objects_flags_conf, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(world->objects_flags_conf, -1));
            lua_pop(world->objects_flags_conf, 1);
            lua_close(world->objects_flags_conf);
            world->objects_flags_conf = NULL;
        }
    }

    world->ent_ID_override = luaL_newstate();
    if(world->ent_ID_override)
    {
        luaL_openlibs(world->ent_ID_override);
        int lua_err = luaL_loadfile(world->ent_ID_override, "scripts/entity/entity_model_ID_override.lua");
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(world->ent_ID_override, -1));
            lua_pop(world->ent_ID_override, 1);
            lua_close(world->ent_ID_override);
            world->ent_ID_override = NULL;
        }
        lua_err = lua_pcall(world->ent_ID_override, 0, 0, 0);
        if(lua_err)
        {
            Sys_DebugLog("lua_out.txt", "%s", lua_tostring(world->ent_ID_override, -1));
            lua_pop(world->ent_ID_override, 1);
            lua_close(world->ent_ID_override);
            world->ent_ID_override = NULL;
        }
    }
}


void World_ScriptsClose(world_p world)
{
    if(world->objects_flags_conf)
    {
        lua_close(world->objects_flags_conf);
        world->objects_flags_conf = NULL;
    }

    if(world->ent_ID_override)
    {
        lua_close(world->ent_ID_override);
        world->ent_ID_override = NULL;
    }

    if(world->level_script)
    {
        lua_close(world->level_script);
        world->level_script = NULL;
    }
}


void World_AutoexecOpen(world_p world)
{
    char temp_script_name[256];
    Engine_GetLevelScriptName(world->version, temp_script_name, "_autoexec");

    luaL_dofile(engine_lua, "scripts/autoexec.lua");    // do standart autoexec
    luaL_dofile(engine_lua, temp_script_name);          // do level-specific autoexec
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


void World_SetEntityModelProperties(world_p world, struct entity_s *ent)
{
    if(world->objects_flags_conf && ent->bf->animations.model)
    {
        int top = lua_gettop(world->objects_flags_conf);
        lua_getglobal(world->objects_flags_conf, "getEntityModelProperties");
        if(lua_isfunction(world->objects_flags_conf, -1))
        {
            lua_pushinteger(world->objects_flags_conf, engine_world.version);              // engine version
            lua_pushinteger(world->objects_flags_conf, ent->bf->animations.model->id);     // entity model id
            if (lua_CallAndLog(world->objects_flags_conf, 2, 4, 0))
            {
                ent->self->collision_type = lua_tointeger(world->objects_flags_conf, -4);      // get collision type flag
                ent->self->collision_shape = lua_tointeger(world->objects_flags_conf, -3);     // get collision shape flag
                ent->bf->animations.model->hide = lua_tointeger(world->objects_flags_conf, -2);// get info about model visibility
                ent->type_flags |= lua_tointeger(world->objects_flags_conf, -1);               // get traverse information
            }
        }
        lua_settop(world->objects_flags_conf, top);
    }

    if(world->level_script && ent->bf->animations.model)
    {
        int top = lua_gettop(world->level_script);
        lua_getglobal(world->level_script, "getEntityModelProperties");
        if(lua_isfunction(world->level_script, -1))
        {
            lua_pushinteger(world->level_script, engine_world.version);                // engine version
            lua_pushinteger(world->level_script, ent->bf->animations.model->id);       // entity model id
            if (lua_CallAndLog(world->level_script, 2, 4, 0))                                 // call that function
            {
                if(!lua_isnil(world->level_script, -4))
                {
                    ent->self->collision_type = lua_tointeger(world->level_script, -4);        // get collision type flag
                }
                if(!lua_isnil(world->level_script, -3))
                {
                    ent->self->collision_shape = lua_tointeger(world->level_script, -3);       // get collision shape flag
                }
                if(!lua_isnil(world->level_script, -2))
                {
                    ent->bf->animations.model->hide = lua_tointeger(world->level_script, -2);  // get info about model visibility
                }
                if(!lua_isnil(world->level_script, -1))
                {
                    ent->type_flags &= ~(ENTITY_TYPE_TRAVERSE | ENTITY_TYPE_TRAVERSE_FLOOR);
                    ent->type_flags |= lua_tointeger(world->level_script, -1);                 // get traverse information
                }
            }
        }
        lua_settop(world->level_script, top);
    }
}


void World_SetStaticMeshProperties(world_p world, struct static_mesh_s *r_static)
{
    if(world->level_script)
    {
        int top = lua_gettop(world->level_script);
        lua_getglobal(world->level_script, "getStaticMeshProperties");
        if(lua_isfunction(world->level_script, -1))
        {
            lua_pushinteger(world->level_script, r_static->object_id);
            if(lua_CallAndLog(world->level_script, 1, 3, 0))
            {
                if(!lua_isnil(world->level_script, -3))
                {
                    r_static->self->collision_type = lua_tointeger(world->level_script, -3);
                }
                if(!lua_isnil(world->level_script, -2))
                {
                    r_static->self->collision_shape = lua_tointeger(world->level_script, -2);
                }
                if(!lua_isnil(world->level_script, -1))
                {
                    r_static->hide = lua_tointeger(world->level_script, -1);
                }
            }
        }
        lua_settop(world->level_script, top);
    }
}


void World_SetEntityFunction(world_p world, struct entity_s *ent)
{
    if(world->objects_flags_conf && ent->bf->animations.model)
    {
        int top = lua_gettop(world->objects_flags_conf);
        lua_getglobal(world->objects_flags_conf, "getEntityFunction");
        if(lua_isfunction(world->objects_flags_conf, -1))
        {
            lua_pushinteger(world->objects_flags_conf, engine_world.version);          // engine version
            lua_pushinteger(world->objects_flags_conf, ent->bf->animations.model->id); // entity model id
            if (lua_CallAndLog(world->objects_flags_conf, 2, 1, 0))
            {
                if(!lua_isnil(world->objects_flags_conf, -1))
                {
                    Res_CreateEntityFunc(engine_lua, lua_tolstring(world->objects_flags_conf, -1, 0), ent->id);
                }
            }
        }
        lua_settop(world->objects_flags_conf, top);
    }
}

// Functions setting parameters from configuration scripts.
void World_GenEntityFunctions(world_p world, struct RedBlackNode_s *x)
{
    entity_p entity = (entity_p)x->data;

    World_SetEntityFunction(world, entity);

    if(x->left != NULL)
    {
        World_GenEntityFunctions(world, x->left);
    }
    if(x->right != NULL)
    {
        World_GenEntityFunctions(world, x->right);
    }
}


void World_GenTextures(struct world_s *world, class VT_Level *tr)
{
    int border_size = renderer.settings.texture_border;
    border_size = (border_size < 0)?(0):(border_size);
    border_size = (border_size > 128)?(128):(border_size);
    world->tex_atlas = new bordered_texture_atlas(border_size,
                                                  tr->textile32_count,
                                                  tr->textile32,
                                                  tr->object_textures_count,
                                                  tr->object_textures,
                                                  tr->sprite_textures_count,
                                                  tr->sprite_textures);

    world->tex_count = (uint32_t) world->tex_atlas->getNumAtlasPages();
    world->textures = (GLuint*)malloc(world->tex_count * sizeof(GLuint));

    qglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    qglPixelZoom(1, 1);
    world->tex_atlas->createTextures(world->textures);

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


void World_GenAnimCommands(struct world_s *world, class VT_Level *tr)
{
    world->anim_commands_count = tr->anim_commands_count;
    world->anim_commands = tr->anim_commands;
    tr->anim_commands = NULL;
    tr->anim_commands_count = 0;
}

/**   Animated textures loading.
  *   Natively, animated textures stored as a stream of bitu16s, which
  *   is then parsed on the fly. What we do is parse this stream to the
  *   proper structures to be used later within renderer.
  */
void World_GenAnimTextures(struct world_s *world, class VT_Level *tr)
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
        world->anim_sequences_count = num_sequences;
        world->anim_sequences = (anim_seq_p)calloc(num_sequences, sizeof(anim_seq_t));

        anim_seq_p seq = world->anim_sequences;
        for(uint16_t i = 0; i < num_sequences; i++,seq++)
        {
            seq->frames_count = *(pointer++) + 1;
            seq->frame_list   =  (uint32_t*)calloc(seq->frames_count, sizeof(uint32_t));

            // Fill up new sequence with frame list.
            seq->anim_type         = TR_ANIMTEXTURE_FORWARD;
            seq->frame_lock        = false; // by default anim is playing
            seq->uvrotate          = false; // by default uvrotate
            seq->reverse_direction = false; // Needed for proper reverse-type start-up.
            seq->frame_rate        = 0.05;  // Should be passed as 1 / FPS.
            seq->frame_time        = 0.0;   // Reset frame time to initial state.
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
                seq->frame_rate = 0.05 * 16;
            }
            seq->frames = (tex_frame_p)calloc(seq->frames_count, sizeof(tex_frame_t));
            engine_world.tex_atlas->getCoordinates(&p0, seq->frame_list[0], false);
            for(uint16_t j = 0; j < seq->frames_count; j++)
            {
                if(seq->uvrotate)
                {
                    seq->frames[j].uvrotate_max = 0.5 * world->tex_atlas->getTextureHeight(seq->frame_list[j]);
                    seq->frames[j].current_uvrotate = 0.0;
                }
                engine_world.tex_atlas->getCoordinates(&p, seq->frame_list[j], false);
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


void World_GenMeshes(struct world_s *world, class VT_Level *tr)
{
    base_mesh_p base_mesh;

    world->meshes_count = tr->meshes_count;
    base_mesh = world->meshes = (base_mesh_p)calloc(world->meshes_count, sizeof(base_mesh_t));
    for(uint32_t i = 0; i < world->meshes_count; i++, base_mesh++)
    {
        TR_GenMesh(base_mesh, i, world->anim_sequences, world->anim_sequences_count, world->tex_atlas, tr);
        BaseMesh_GenFaces(base_mesh);
    }
}


void World_GenSprites(struct world_s *world, class VT_Level *tr)
{
    sprite_p s;
    tr_sprite_texture_t *tr_st;

    if(tr->sprite_textures_count == 0)
    {
        world->sprites = NULL;
        world->sprites_count = 0;
        return;
    }

    world->sprites_count = tr->sprite_textures_count;
    s = world->sprites = (sprite_p)calloc(world->sprites_count, sizeof(sprite_t));

    for(uint32_t i = 0; i < world->sprites_count; i++, s++)
    {
        tr_st = &tr->sprite_textures[i];

        s->left = tr_st->left_side;
        s->right = tr_st->right_side;
        s->top = tr_st->top_side;
        s->bottom = tr_st->bottom_side;

        world->tex_atlas->getSpriteCoordinates(s->tex_coord, i, &s->texture_index);
    }

    for(uint32_t i = 0; i < tr->sprite_sequences_count; i++)
    {
        if((tr->sprite_sequences[i].offset >= 0) && ((uint32_t)tr->sprite_sequences[i].offset < world->sprites_count))
        {
            world->sprites[tr->sprite_sequences[i].offset].id = tr->sprite_sequences[i].object_id;
        }
    }
}


void World_GenBoxes(struct world_s *world, class VT_Level *tr)
{
    world->room_boxes = NULL;
    world->room_box_count = tr->boxes_count;

    if(world->room_box_count)
    {
        world->room_boxes = (room_box_p)malloc(world->room_box_count * sizeof(room_box_t));
        for(uint32_t i = 0; i < world->room_box_count; i++)
        {
            world->room_boxes[i].overlap_index = tr->boxes[i].overlap_index;
            world->room_boxes[i].true_floor =-tr->boxes[i].true_floor;
            world->room_boxes[i].x_min = tr->boxes[i].xmin;
            world->room_boxes[i].x_max = tr->boxes[i].xmax;
            world->room_boxes[i].y_min =-tr->boxes[i].zmax;
            world->room_boxes[i].y_max =-tr->boxes[i].zmin;
        }
    }
}


void World_GenCameras(struct world_s *world, class VT_Level *tr)
{
    world->cameras_sinks = NULL;
    world->cameras_sinks_count = tr->cameras_count;

    if(world->cameras_sinks_count)
    {
        world->cameras_sinks = (stat_camera_sink_p)malloc(world->cameras_sinks_count * sizeof(stat_camera_sink_t));
        for(uint32_t i = 0; i < world->cameras_sinks_count; i++)
        {
            world->cameras_sinks[i].x                   =  tr->cameras[i].x;
            world->cameras_sinks[i].y                   =  tr->cameras[i].z;
            world->cameras_sinks[i].z                   = -tr->cameras[i].y;
            world->cameras_sinks[i].room_or_strength    =  tr->cameras[i].room;
            world->cameras_sinks[i].flag_or_zone        =  tr->cameras[i].unknown1;
        }
    }
}


__inline void TR_vertex_to_arr(float v[3], tr5_vertex_t *tr_v)
{
    v[0] = tr_v->x;
    v[1] =-tr_v->z;
    v[2] = tr_v->y;
}

void World_GenRoom(struct world_s *world, struct room_s *room, class VT_Level *tr)
{
    portal_p p;
    room_p r_dest;
    tr5_room_t *tr_room = &tr->rooms[room->id];
    tr_staticmesh_t *tr_static;
    static_mesh_p r_static;
    tr_room_portal_t *tr_portal;
    room_sector_p sector;

    room->flags = tr->rooms[room->id].flags;
    room->frustum = NULL;
    room->active = 1;

    Mat4_E_macro(room->transform);
    room->transform[12] = tr->rooms[room->id].offset.x;                         // x = x;
    room->transform[13] =-tr->rooms[room->id].offset.z;                         // y =-z;
    room->transform[14] = tr->rooms[room->id].offset.y;                         // z = y;

    room->self = (engine_container_p)malloc(sizeof(engine_container_t));
    room->self->next = NULL;
    room->self->room = room;
    room->self->object = room;
    room->self->collision_type = COLLISION_TYPE_STATIC;
    room->self->collision_shape = COLLISION_SHAPE_TRIMESH;
    room->self->object_type = OBJECT_ROOM_BASE;

    room->near_room_list_size = 0;
    room->overlapped_room_list_size = 0;

    room->content = (room_content_p)malloc(sizeof(room_content_t));
    room->content->containers = NULL;
    room->content->physics_body = NULL;
    room->content->mesh = NULL;
    room->content->static_mesh = NULL;
    room->content->sprites = NULL;
    room->content->lights = NULL;
    room->content->light_mode = tr->rooms[room->id].light_mode;
    room->content->reverb_info = tr->rooms[room->id].reverb_info;
    room->content->water_scheme = tr->rooms[room->id].water_scheme;
    room->content->alternate_group = tr->rooms[room->id].alternate_group;
    room->content->ambient_lighting[0] = tr->rooms[room->id].light_colour.r * 2;
    room->content->ambient_lighting[1] = tr->rooms[room->id].light_colour.g * 2;
    room->content->ambient_lighting[2] = tr->rooms[room->id].light_colour.b * 2;

    TR_GenRoomMesh(room, room->id, world->anim_sequences, world->anim_sequences_count, world->tex_atlas, tr);
    if(room->content->mesh)
    {
        BaseMesh_GenFaces(room->content->mesh);
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
        r_static->self = (engine_container_p)calloc(1, sizeof(engine_container_t));
        r_static->self->room = room;
        r_static->self->object = room->content->static_mesh + i;
        r_static->self->object_type = OBJECT_STATIC_MESH;
        r_static->object_id = tr_room->static_meshes[i].object_id;
        r_static->mesh = world->meshes + tr->mesh_indices[tr_static->mesh];
        r_static->pos[0] = tr_room->static_meshes[i].pos.x;
        r_static->pos[1] =-tr_room->static_meshes[i].pos.z;
        r_static->pos[2] = tr_room->static_meshes[i].pos.y;
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
        r_static->obb->r = r_static->mesh->R;
        Mat4_E(r_static->transform);
        Mat4_Translate(r_static->transform, r_static->pos);
        Mat4_RotateZ(r_static->transform, r_static->rot[0]);
        r_static->was_rendered = 0;
        OBB_Rebuild(r_static->obb, r_static->vbb_min, r_static->vbb_max);
        OBB_Transform(r_static->obb);

        r_static->physics_body = NULL;
        r_static->hide = 0;

        // Disable static mesh collision, if flag value is 3 (TR1) or all bounding box
        // coordinates are equal (TR2-5).

        if((tr_static->flags == 3) ||
           ((r_static->cbb_min[0] == r_static->cbb_min[1]) && (r_static->cbb_min[1] == r_static->cbb_min[2]) &&
            (r_static->cbb_max[0] == r_static->cbb_max[1]) && (r_static->cbb_max[1] == r_static->cbb_max[2])) )
        {
            r_static->self->collision_type = COLLISION_NONE;
        }
        else
        {
            r_static->self->collision_type = COLLISION_TYPE_STATIC;
        }

        // Set additional static mesh properties from level script override.

        World_SetStaticMeshProperties(world, r_static);

        // Set static mesh collision.
        Physics_GenStaticMeshRigidBody(r_static);
    }

    /*
     * sprites loading section
     */
    room->content->sprites_count = tr_room->num_sprites;
    if(room->content->sprites_count != 0)
    {
        room->content->sprites = (room_sprite_p)calloc(room->content->sprites_count, sizeof(room_sprite_t));
        for(uint32_t i = 0; i < room->content->sprites_count; i++)
        {
            if((tr_room->sprites[i].texture >= 0) && ((uint32_t)tr_room->sprites[i].texture < world->sprites_count))
            {
                room->content->sprites[i].sprite = world->sprites + tr_room->sprites[i].texture;
                TR_vertex_to_arr(room->content->sprites[i].pos, &tr_room->vertices[tr_room->sprites[i].vertex].vertex);
                vec3_add(room->content->sprites[i].pos, room->content->sprites[i].pos, room->transform + 12);
            }
        }
    }

    /*
     * let us load sectors
     */
    room->sectors_x = tr_room->num_xsectors;
    room->sectors_y = tr_room->num_zsectors;
    room->sectors_count = room->sectors_x * room->sectors_y;
    room->sectors = (room_sector_p)malloc(room->sectors_count * sizeof(room_sector_t));

    /*
     * base sectors information loading and collisional mesh creation
     */

    // To avoid manipulating with unnecessary information, we declare simple
    // heightmap here, which will be operated with sector and floordata parsing,
    // then vertical inbetween polys will be constructed, and Bullet collisional
    // object will be created. Afterwards, this heightmap also can be used to
    // quickly detect slopes for pushable blocks and other entities that rely on
    // floor level.

    sector = room->sectors;
    for(uint32_t i = 0; i < room->sectors_count; i++, sector++)
    {
        // Filling base sectors information.

        sector->index_x = i / room->sectors_y;
        sector->index_y = i % room->sectors_y;

        sector->pos[0] = room->transform[12] + sector->index_x * TR_METERING_SECTORSIZE + 0.5 * TR_METERING_SECTORSIZE;
        sector->pos[1] = room->transform[13] + sector->index_y * TR_METERING_SECTORSIZE + 0.5 * TR_METERING_SECTORSIZE;
        sector->pos[2] = 0.5 * (tr_room->y_bottom + tr_room->y_top);

        sector->owner_room = room;
        sector->trigger = NULL;
        
        if(tr->game_version < TR_III)
        {
            sector->box_index = tr_room->sector_list[i].box_index;
            sector->material  = SECTOR_MATERIAL_STONE;
        }
        else
        {
            sector->box_index = (tr_room->sector_list[i].box_index & 0xFFF0) >> 4;
            sector->material  =  tr_room->sector_list[i].box_index & 0x000F;
        }

        if(sector->box_index == 0xFFFF) sector->box_index = -1;

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
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_above != 0xFF)
        {
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_GHOST;
        }
        else
        {
            room->sectors[i].ceiling_penetration_config = TR_PENETRATION_CONFIG_SOLID;
        }

        // Reset some sector parameters to avoid garbaged memory issues.
        room->sectors[i].portal_to_room = NULL;
        room->sectors[i].ceiling_diagonal_type = TR_SECTOR_DIAGONAL_TYPE_NONE;
        room->sectors[i].floor_diagonal_type   = TR_SECTOR_DIAGONAL_TYPE_NONE;

        // Now, we define heightmap cells position and draft (flat) height.
        // Draft height is derived from sector's floor and ceiling values, which are
        // copied into heightmap cells Y coordinates. As result, we receive flat
        // heightmap cell, which will be operated later with floordata.

        room->sectors[i].ceiling_corners[0][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[0][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[0][2] = (float)sector->ceiling;

        room->sectors[i].ceiling_corners[1][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[1][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[1][2] = (float)sector->ceiling;

        room->sectors[i].ceiling_corners[2][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[2][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[2][2] = (float)sector->ceiling;

        room->sectors[i].ceiling_corners[3][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[3][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].ceiling_corners[3][2] = (float)sector->ceiling;

        // BUILDING FLOOR HEIGHTMAP.

        // Features same steps as for the ceiling.

        if(sector->floor == TR_METERING_WALLHEIGHT)
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_WALL;
        }
        else if(tr_room->sector_list[i].room_below != 0xFF)
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_GHOST;
        }
        else
        {
            room->sectors[i].floor_penetration_config = TR_PENETRATION_CONFIG_SOLID;
        }

        room->sectors[i].floor_corners[0][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[0][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[0][2] = (float)sector->floor;

        room->sectors[i].floor_corners[1][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[1][1] = (float)sector->index_y * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[1][2] = (float)sector->floor;

        room->sectors[i].floor_corners[2][0] = (float)sector->index_x * TR_METERING_SECTORSIZE + TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[2][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[2][2] = (float)sector->floor;

        room->sectors[i].floor_corners[3][0] = (float)sector->index_x * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[3][1] = (float)sector->index_y * TR_METERING_SECTORSIZE;
        room->sectors[i].floor_corners[3][2] = (float)sector->floor;
    }

    /*
     *  load lights
     */
    room->content->light_count = tr_room->num_lights;
    if(room->content->light_count)
    {
        room->content->lights = (light_p)malloc(room->content->light_count * sizeof(light_t));
    }

    for(uint16_t i = 0; i < tr_room->num_lights; i++)
    {
        struct light_s *light = room->content->lights + i;
        switch(tr_room->lights[i].light_type)
        {
        case 0:
            light->light_type = LT_SUN;
            break;
        case 1:
            light->light_type = LT_POINT;
            break;
        case 2:
            light->light_type = LT_SPOTLIGHT;
            break;
        case 3:
            light->light_type = LT_SHADOW;
            break;
        default:
            light->light_type = LT_NULL;
            break;
        }

        light->pos[0] = tr_room->lights[i].pos.x;
        light->pos[1] = -tr_room->lights[i].pos.z;
        light->pos[2] = tr_room->lights[i].pos.y;
        light->pos[3] = 1.0f;

        if(light->light_type == LT_SHADOW)
        {
            light->colour[0] = -(tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
            light->colour[1] = -(tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
            light->colour[2] = -(tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
            light->colour[3] = 1.0f;
        }
        else
        {
            light->colour[0] = (tr_room->lights[i].color.r / 255.0f) * tr_room->lights[i].intensity;
            light->colour[1] = (tr_room->lights[i].color.g / 255.0f) * tr_room->lights[i].intensity;
            light->colour[2] = (tr_room->lights[i].color.b / 255.0f) * tr_room->lights[i].intensity;
            light->colour[3] = 1.0f;
        }

        light->inner = tr_room->lights[i].r_inner;
        light->outer = tr_room->lights[i].r_outer;
        light->length = tr_room->lights[i].length;
        light->cutoff = tr_room->lights[i].cutoff;

        light->falloff = 0.001f / light->outer;
    }


    /*
     * portals loading / calculation!!!
     */
    room->portals_count = tr_room->num_portals;
    p = room->portals = (portal_p)calloc(room->portals_count, sizeof(portal_t));
    tr_portal = tr_room->portals;
    for(uint16_t i = 0; i < room->portals_count; i++, p++, tr_portal++)
    {
        r_dest = world->rooms + tr_portal->adjoining_room;
        p->vertex_count = 4;                                                    // in original TR all portals are axis aligned rectangles
        p->vertex = (float*)malloc(3*p->vertex_count*sizeof(float));
        p->dest_room = r_dest;
        p->current_room = room;
        TR_vertex_to_arr(p->vertex  , &tr_portal->vertices[3]);
        vec3_add(p->vertex, p->vertex, room->transform+12);
        TR_vertex_to_arr(p->vertex+3, &tr_portal->vertices[2]);
        vec3_add(p->vertex+3, p->vertex+3, room->transform+12);
        TR_vertex_to_arr(p->vertex+6, &tr_portal->vertices[1]);
        vec3_add(p->vertex+6, p->vertex+6, room->transform+12);
        TR_vertex_to_arr(p->vertex+9, &tr_portal->vertices[0]);
        vec3_add(p->vertex+9, p->vertex+9, room->transform+12);
        vec3_add(p->centre, p->vertex, p->vertex+3);
        vec3_add(p->centre, p->centre, p->vertex+6);
        vec3_add(p->centre, p->centre, p->vertex+9);
        p->centre[0] /= 4.0;
        p->centre[1] /= 4.0;
        p->centre[2] /= 4.0;
        Portal_GenNormale(p);

        /*
         * Portal position fix...
         */
        // X_MIN
        if((p->norm[0] > 0.999) && (((int)p->centre[0])%2))
        {
            float pos[3] = {1.0, 0.0, 0.0};
            Portal_Move(p, pos);
        }

        // Y_MIN
        if((p->norm[1] > 0.999) && (((int)p->centre[1])%2))
        {
            float pos[3] = {0.0, 1.0, 0.0};
            Portal_Move(p, pos);
        }

        // Z_MAX
        if((p->norm[2] <-0.999) && (((int)p->centre[2])%2))
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

    room->bb_min[0] = room->transform[12] + TR_METERING_SECTORSIZE;
    room->bb_min[1] = room->transform[13] + TR_METERING_SECTORSIZE;
    room->bb_max[0] = room->transform[12] + TR_METERING_SECTORSIZE * room->sectors_x - TR_METERING_SECTORSIZE;
    room->bb_max[1] = room->transform[13] + TR_METERING_SECTORSIZE * room->sectors_y - TR_METERING_SECTORSIZE;

    /*
     * alternate room pointer calculation if one exists.
     */
    room->alternate_room = NULL;
    room->base_room = NULL;

    if((tr_room->alternate_room >= 0) && ((uint32_t)tr_room->alternate_room < tr->rooms_count))
    {
        room->alternate_room = world->rooms + tr_room->alternate_room;
    }
}


void World_GenRooms(struct world_s *world, class VT_Level *tr)
{
    world->room_count = tr->rooms_count;
    room_p r = world->rooms = (room_p)malloc(world->room_count * sizeof(room_t));
    for(uint32_t i = 0; i < world->room_count; i++, r++)
    {
        r->id = i;
        World_GenRoom(world, r, tr);
    }
}


void World_GenRoomFlipMap(struct world_s *world)
{
    // Flipmap count is hardcoded, as no original levels contain such info.
    world->flip_count = FLIPMAP_MAX_NUMBER;

    world->flip_map   = (uint8_t*)malloc(world->flip_count * sizeof(uint8_t));
    world->flip_state = (uint8_t*)malloc(world->flip_count * sizeof(uint8_t));

    memset(world->flip_map,   0, world->flip_count);
    memset(world->flip_state, 0, world->flip_count);
}


void World_GenSkeletalModels(struct world_s *world, class VT_Level *tr)
{
    skeletal_model_p smodel;
    tr_moveable_t *tr_moveable;

    world->skeletal_model_count = tr->moveables_count;
    smodel = world->skeletal_models = (skeletal_model_p)calloc(world->skeletal_model_count, sizeof(skeletal_model_t));

    for(uint32_t i = 0; i < world->skeletal_model_count; i++, smodel++)
    {
        tr_moveable = &tr->moveables[i];
        smodel->id = tr_moveable->object_id;
        smodel->mesh_count = tr_moveable->num_meshes;
        TR_GenSkeletalModel(smodel, i, world->meshes, world->anim_commands, tr);
        SkeletalModel_FillTransparency(smodel);
    }
}


void World_GenEntities(struct world_s *world, class VT_Level *tr)
{
    int top;

    tr2_item_t *tr_item;
    entity_p entity;

    for(uint32_t i = 0;i < tr->items_count; i++)
    {
        tr_item = &tr->items[i];
        entity = Entity_Create();
        entity->id = i;
        entity->transform[12] = tr_item->pos.x;
        entity->transform[13] =-tr_item->pos.z;
        entity->transform[14] = tr_item->pos.y;
        entity->angles[0] = tr_item->rotation;
        entity->angles[1] = 0.0;
        entity->angles[2] = 0.0;
        Entity_UpdateTransform(entity);
        if((tr_item->room >= 0) && ((uint32_t)tr_item->room < world->room_count))
        {
            entity->self->room = world->rooms + tr_item->room;
        }
        else
        {
            entity->self->room = NULL;
        }

        entity->trigger_layout  = (tr_item->flags & 0x3E00) >> 9;   ///@FIXME: Ignore INVISIBLE and CLEAR BODY flags for a moment.
        entity->OCB             = tr_item->ocb;
        entity->timer           = 0.0;

        entity->self->collision_type = COLLISION_TYPE_KINEMATIC;
        entity->self->collision_shape = COLLISION_SHAPE_TRIMESH;
        entity->move_type          = 0x0000;
        entity->inertia_linear     = 0.0;
        entity->inertia_angular[0] = 0.0;
        entity->inertia_angular[1] = 0.0;
        entity->move_type          = 0;

        entity->bf->animations.model = World_GetModelByID(world, tr_item->object_id);

        if(world->ent_ID_override)
        {
            if(entity->bf->animations.model == NULL)
            {
                top = lua_gettop(world->ent_ID_override);                       // save LUA stack
                lua_getglobal(world->ent_ID_override, "getOverridedID");        // add to the up of stack LUA's function
                lua_pushinteger(world->ent_ID_override, tr->game_version);      // add to stack first argument
                lua_pushinteger(world->ent_ID_override, tr_item->object_id);    // add to stack second argument
                if (lua_CallAndLog(world->ent_ID_override, 2, 1, 0))            // call that function
                {
                    entity->bf->animations.model = World_GetModelByID(world, lua_tointeger(world->ent_ID_override, -1));
                }
                lua_settop(world->ent_ID_override, top);                               // restore LUA stack
            }

            top = lua_gettop(world->ent_ID_override);                                  // save LUA stack
            lua_getglobal(world->ent_ID_override, "getOverridedAnim");                 // add to the up of stack LUA's function
            lua_pushinteger(world->ent_ID_override, tr->game_version);                 // add to stack first argument
            lua_pushinteger(world->ent_ID_override, tr_item->object_id);               // add to stack second argument
            if (lua_CallAndLog(world->ent_ID_override, 2, 1, 0))                       // call that function
            {
                int replace_anim_id = lua_tointeger(world->ent_ID_override, -1);
                if(replace_anim_id > 0)
                {
                    skeletal_model_s* replace_anim_model = World_GetModelByID(world, replace_anim_id);
                    animation_frame_p ta;
                    uint16_t tc;
                    SWAPT(entity->bf->animations.model->animations, replace_anim_model->animations, ta);
                    SWAPT(entity->bf->animations.model->animation_count, replace_anim_model->animation_count, tc);
                }
            }
            lua_settop(world->ent_ID_override, top);                                   // restore LUA stack
        }

        if(entity->bf->animations.model == NULL)
        {
            // SPRITE LOADING
            sprite_p sp = World_GetSpriteByID(world, tr_item->object_id);
            if(sp && entity->self->room)
            {
                room_sprite_p rsp;
                int sz = ++entity->self->room->content->sprites_count;
                entity->self->room->content->sprites = (room_sprite_p)realloc(entity->self->room->content->sprites, sz * sizeof(room_sprite_t));
                rsp = entity->self->room->content->sprites + sz - 1;
                rsp->sprite = sp;
                rsp->pos[0] = entity->transform[12];
                rsp->pos[1] = entity->transform[13];
                rsp->pos[2] = entity->transform[14];
                rsp->was_rendered = 0;
            }

            Entity_Clear(entity);
            free(entity);
            continue;                                                           // that entity has no model. may be it is a some trigger or look at object
        }

        if(tr->game_version < TR_II && tr_item->object_id == 83)                ///@FIXME: brutal magick hardcode! ;-)
        {
            Entity_Clear(entity);                                               // skip PSX save model
            free(entity);
            continue;
        }

        SSBoneFrame_CreateFromModel(entity->bf, entity->bf->animations.model);

        if(0 == tr_item->object_id)                                             // Lara is unical model
        {
            skeletal_model_p tmp, LM;                                           // LM - Lara Model

            entity->move_type = MOVE_ON_FLOOR;
            world->Character = entity;
            entity->self->collision_type = COLLISION_TYPE_ACTOR;
            entity->self->collision_shape = COLLISION_SHAPE_TRIMESH_CONVEX;
            entity->bf->animations.model->hide = 0;
            entity->type_flags |= ENTITY_TYPE_TRIGGER_ACTIVATOR;
            LM = (skeletal_model_p)entity->bf->animations.model;

            top = lua_gettop(engine_lua);
            lua_pushinteger(engine_lua, entity->id);
            lua_setglobal(engine_lua, "player");
            lua_settop(engine_lua, top);

            switch(tr->game_version)
            {
                case TR_I:
                    if(gameflow_manager.CurrentLevelID == 0)
                    {
                        LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_ALTERNATE_TR1);
                        if(LM)
                        {
                            // In TR1, Lara has unified head mesh for all her alternate skins.
                            // Hence, we copy all meshes except head, to prevent Potato Raider bug.
                            SkeletalModel_CopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count - 1);
                        }
                    }
                    break;

                case TR_III:
                    LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_TR3);
                    if(LM)
                    {
                        SkeletalModel_CopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                        tmp = World_GetModelByID(world, 11);                   // moto / quadro cycle animations
                        if(tmp)
                        {
                            SkeletalModel_CopyMeshes(tmp->mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                        }
                    }
                    break;

                case TR_IV:
                case TR_IV_DEMO:
                case TR_V:
                    LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_TR45);                         // base skeleton meshes
                    if(LM)
                    {
                        SkeletalModel_CopyMeshes(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                    }
                    LM = World_GetModelByID(world, TR_ITEM_LARA_SKIN_JOINTS_TR45);                 // skin skeleton meshes
                    if(LM)
                    {
                        SkeletalModel_CopyMeshesToSkinned(world->skeletal_models[0].mesh_tree, LM->mesh_tree, world->skeletal_models[0].mesh_count);
                    }
                    SkeletalModel_FillSkinnedMeshMap(&world->skeletal_models[0]);
                    break;
            };

            for(uint16_t j = 0; j < entity->bf->bone_tag_count; j++)
            {
                entity->bf->bone_tags[j].mesh_base = entity->bf->animations.model->mesh_tree[j].mesh_base;
                entity->bf->bone_tags[j].mesh_skin = entity->bf->animations.model->mesh_tree[j].mesh_skin;
                entity->bf->bone_tags[j].mesh_slot = NULL;
            }
            Entity_SetAnimation(world->Character, TR_ANIMATION_LARA_STAY_IDLE, 0);
            Physics_GenRigidBody(entity->physics, entity->bf, entity->transform);
            Character_Create(entity);
            entity->character->Height = 768.0;
            entity->character->state_func = State_Control_Lara;

            continue;
        }

        Entity_SetAnimation(entity, 0, 0);                                      // Set zero animation and zero frame
        Entity_RebuildBV(entity);
        Room_AddObject(entity->self->room, entity->self);
        World_AddEntity(world, entity);
        World_SetEntityModelProperties(world, entity);
        Physics_GenRigidBody(entity->physics, entity->bf, entity->transform);

        if(!(entity->state_flags & ENTITY_STATE_ENABLED) || !(entity->self->collision_type & 0x0001))
        {
            Entity_DisableCollision(entity);
        }
    }
}


void World_GenBaseItems(struct world_s* world)
{
    lua_CallVoidFunc(engine_lua, "genBaseItems");

    if(world->items_tree && world->items_tree->root)
    {
        World_MakeEntityItems(world, world->items_tree->root);
    }
}


void World_GenSpritesBuffer(struct world_s *world)
{
    for (uint32_t i = 0; i < world->room_count; i++)
    {
        Room_GenSpritesBuffer(&world->rooms[i]);
    }
}


void World_GenRoomProperties(struct world_s *world, class VT_Level *tr)
{
    room_p r = world->rooms;
    for(uint32_t i = 0; i < world->room_count; i++, r++)
    {
        if(r->alternate_room != NULL)
        {
            r->alternate_room->base_room = r;   // Refill base room pointer.
        }

        // Fill heightmap and translate floordata.
        for(uint32_t j = 0; j < r->sectors_count; j++)
        {
            Res_Sector_TranslateFloorData(world->rooms, world->room_count, r->sectors + j, tr);
        }

        // Generate links to the near rooms.
        World_BuildNearRoomsList(world, r);
        // Generate links to the overlapped rooms.
        World_BuildOverlappedRoomsList(world, r);

        // Basic sector calculations.
        Res_RoomSectorsCalculate(world->rooms, world->room_count, i, tr);
    }
}


void World_GenRoomCollision(struct world_s *world)
{
    room_p r = world->rooms;

    /*
    if(level_script != NULL)
    {
        lua_CallVoidFunc(level_script, "doTuneSector");
    }
    */

    for(uint32_t i = 0; i < world->room_count; i++, r++)
    {
        // Inbetween polygons array is later filled by loop which scans adjacent
        // sector heightmaps and fills the gaps between them, thus creating inbetween
        // polygon. Inbetweens can be either quad (if all four corner heights are
        // different), triangle (if one corner height is similar to adjacent) or
        // ghost (if corner heights are completely similar). In case of quad inbetween,
        // two triangles are added to collisional trimesh, in case of triangle inbetween,
        // we add only one, and in case of ghost inbetween, we ignore it.

        int num_heightmaps = (r->sectors_x * r->sectors_y);
        int num_tweens = (num_heightmaps * 4);
        sector_tween_s *room_tween   = new sector_tween_s[num_tweens];

        // Clear tween array.

        for(int j = 0; j < num_tweens; j++)
        {
            room_tween[j].ceiling_tween_type = TR_SECTOR_TWEEN_TYPE_NONE;
            room_tween[j].floor_tween_type   = TR_SECTOR_TWEEN_TYPE_NONE;
        }

        // Most difficult task with converting floordata collision to trimesh collision is
        // building inbetween polygons which will block out gaps between sector heights.
        Res_Sector_GenTweens(r, room_tween);

        // Final step is sending actual sectors to Bullet collision model. We do it here.
        Physics_GenRoomRigidBody(r, room_tween, num_tweens);

        delete[] room_tween;
    }
}


void World_GenSamples(struct world_s *world, class VT_Level *tr)
{
    uint8_t      *pointer = tr->samples_data;
    int8_t        flag;
    uint32_t      ind1, ind2;
    uint32_t      comp_size, uncomp_size;
    uint32_t      i;

    // Generate new buffer array.
    world->audio_buffers_count = tr->samples_count;
    world->audio_buffers = (ALuint*)malloc(world->audio_buffers_count * sizeof(ALuint));
    memset(world->audio_buffers, 0, sizeof(ALuint) * world->audio_buffers_count);
    alGenBuffers(world->audio_buffers_count, world->audio_buffers);

    // Generate stream track map array.
    // We use scripted amount of tracks to define map bounds.
    // If script had no such parameter, we define map bounds by default.
    world->stream_track_map_count = lua_GetNumTracks(engine_lua);
    if(world->stream_track_map_count == 0) world->stream_track_map_count = TR_AUDIO_STREAM_MAP_SIZE;
    world->stream_track_map = (uint8_t*)malloc(world->stream_track_map_count * sizeof(uint8_t));
    memset(world->stream_track_map, 0, sizeof(uint8_t) * world->stream_track_map_count);

    // Generate new audio effects array.
    world->audio_effects_count = tr->sound_details_count;
    world->audio_effects =  (audio_effect_t*)malloc(tr->sound_details_count * sizeof(audio_effect_t));
    memset(world->audio_effects, 0xFF, sizeof(audio_effect_t) * tr->sound_details_count);

    // Generate new audio emitters array.
    world->audio_emitters_count = tr->sound_sources_count;
    world->audio_emitters = (audio_emitter_t*)malloc(tr->sound_sources_count * sizeof(audio_emitter_t));
    memset(world->audio_emitters, 0, sizeof(audio_emitter_t) * tr->sound_sources_count);

    // Copy sound map.
    world->audio_map = tr->soundmap;
    tr->soundmap = NULL;                   /// without it VT destructor free(tr->soundmap)

    // Cycle through raw samples block and parse them to OpenAL buffers.

    // Different TR versions have different ways of storing samples.
    // TR1:     sample block size, sample block, num samples, sample offsets.
    // TR2/TR3: num samples, sample offsets. (Sample block is in MAIN.SFX.)
    // TR4/TR5: num samples, (uncomp_size-comp_size-sample_data) chain.
    //
    // Hence, we specify certain parse method for each game version.

    if(pointer)
    {
        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                world->audio_map_count = TR_AUDIO_MAP_SIZE_TR1;

                for(i = 0; i < world->audio_buffers_count-1; i++)
                {
                    pointer = tr->samples_data + tr->sample_indices[i];
                    uint32_t size = tr->sample_indices[(i+1)] - tr->sample_indices[i];
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, size);
                }
                i = world->audio_buffers_count-1;
                Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, (tr->samples_count - tr->sample_indices[i]));
                break;

            case TR_II:
            case TR_II_DEMO:
            case TR_III:
                world->audio_map_count = (tr->game_version == TR_III)?(TR_AUDIO_MAP_SIZE_TR3):(TR_AUDIO_MAP_SIZE_TR2);
                ind1 = 0;
                ind2 = 0;
                flag = 0;
                i = 0;
                while(pointer < tr->samples_data + tr->samples_data_size - 4)
                {
                    pointer = tr->samples_data + ind2;
                    if(!memcmp(pointer, "RIFF", 4))
                    {
                        if(flag == 0x00)
                        {
                            ind1 = ind2;
                            flag = 0x01;
                        }
                        else
                        {
                            uncomp_size = ind2 - ind1;
                            Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], tr->samples_data + ind1, uncomp_size);
                            i++;
                            if(i > world->audio_buffers_count - 1)
                            {
                                break;
                            }
                            ind1 = ind2;
                        }
                    }
                    ind2++;
                }
                uncomp_size = tr->samples_data_size - ind1;
                pointer = tr->samples_data + ind1;
                if(i < world->audio_buffers_count)
                {
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, uncomp_size);
                }
                break;

            case TR_IV:
            case TR_IV_DEMO:
            case TR_V:
                world->audio_map_count = (tr->game_version == TR_V)?(TR_AUDIO_MAP_SIZE_TR5):(TR_AUDIO_MAP_SIZE_TR4);

                for(i = 0; i < tr->samples_count; i++)
                {
                    // Parse sample sizes.
                    // Always use comp_size as block length, as uncomp_size is used to cut raw sample data.
                    uncomp_size = *((uint32_t*)pointer);
                    pointer += 4;
                    comp_size   = *((uint32_t*)pointer);
                    pointer += 4;

                    // Load WAV sample into OpenAL buffer.
                    Audio_LoadALbufferFromWAV_Mem(world->audio_buffers[i], pointer, comp_size, uncomp_size);

                    // Now we can safely move pointer through current sample data.
                    pointer += comp_size;
                }
                break;

            default:
                world->audio_map_count = TR_AUDIO_MAP_SIZE_NONE;
                free(tr->samples_data);
                tr->samples_data = NULL;
                tr->samples_data_size = 0;
                return;
        }

        free(tr->samples_data);
        tr->samples_data = NULL;
        tr->samples_data_size = 0;
    }

    // Cycle through SoundDetails and parse them into native OpenTomb
    // audio effects structure.
    for(i = 0; i < world->audio_effects_count; i++)
    {
        if(tr->game_version < TR_III)
        {
            world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 32767.0; // Max. volume in TR1/TR2 is 32767.
            world->audio_effects[i].chance = tr->sound_details[i].chance;
        }
        else if(tr->game_version > TR_III)
        {
            world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            world->audio_effects[i].chance = tr->sound_details[i].chance * 255;
        }
        else
        {
            world->audio_effects[i].gain   = (float)(tr->sound_details[i].volume) / 255.0; // Max. volume in TR3 is 255.
            world->audio_effects[i].chance = tr->sound_details[i].chance * 127;
        }

        world->audio_effects[i].rand_gain_var  = 50;
        world->audio_effects[i].rand_pitch_var = 50;

        world->audio_effects[i].pitch = (float)(tr->sound_details[i].pitch) / 127.0 + 1.0;
        world->audio_effects[i].range = (float)(tr->sound_details[i].sound_range) * 1024.0;

        world->audio_effects[i].rand_pitch = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_PITCH);
        world->audio_effects[i].rand_gain  = (tr->sound_details[i].flags_2 & TR_AUDIO_FLAG_RAND_VOLUME);

        switch(tr->game_version)
        {
            case TR_I:
            case TR_I_DEMO:
            case TR_I_UB:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    case 0x02:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    case 0x01:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    default:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;

            case TR_II:
            case TR_II_DEMO:
                switch(tr->sound_details[i].num_samples_and_flags_1 & 0x03)
                {
                    /*case 0x02:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;*/
                    case 0x01:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_REWIND;
                        break;
                    case 0x03:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_LOOPED;
                        break;
                    default:
                        world->audio_effects[i].loop = TR_AUDIO_LOOP_NONE;
                }
                break;

            default:
                world->audio_effects[i].loop = (tr->sound_details[i].num_samples_and_flags_1 & TR_AUDIO_LOOP_LOOPED);
                break;
        }

        world->audio_effects[i].sample_index =  tr->sound_details[i].sample;
        world->audio_effects[i].sample_count = (tr->sound_details[i].num_samples_and_flags_1 >> 2) & TR_AUDIO_SAMPLE_NUMBER_MASK;
    }

    // Try to override samples via script.
    // If there is no script entry exist, we just leave default samples.
    // NB! We need to override samples AFTER audio effects array is inited, as override
    //     routine refers to existence of certain audio effect in level.

    Audio_LoadOverridedSamples(world);

    // Hardcoded version-specific fixes!

    switch(world->version)
    {
        case TR_I:
        case TR_I_DEMO:
        case TR_I_UB:
            // Fix for underwater looped sound.
            if ((world->audio_map[TR_AUDIO_SOUND_UNDERWATER]) >= 0)
            {
                world->audio_effects[(world->audio_map[TR_AUDIO_SOUND_UNDERWATER])].loop = TR_AUDIO_LOOP_LOOPED;
            }
            break;
        case TR_II:
            // Fix for helicopter sound range.
            if ((world->audio_map[297]) >= 0)
            {
                world->audio_effects[(world->audio_map[297])].range *= 10.0;
            }
            break;
    }

    // Cycle through sound emitters and
    // parse them to native OpenTomb sound emitters structure.

    for(i = 0; i < world->audio_emitters_count; i++)
    {
        world->audio_emitters[i].emitter_index = i;
        world->audio_emitters[i].sound_index   =  tr->sound_sources[i].sound_id;
        world->audio_emitters[i].position[0]   =  tr->sound_sources[i].x;
        world->audio_emitters[i].position[1]   =  tr->sound_sources[i].z;
        world->audio_emitters[i].position[2]   = -tr->sound_sources[i].y;
        world->audio_emitters[i].flags         =  tr->sound_sources[i].flags;
    }
}


void World_FixRooms(struct world_s *world)
{
    room_p r = world->rooms;
    for(uint32_t i = 0;i < world->room_count; i++, r++)
    {
        if(r->base_room != NULL)
        {
            Room_Disable(r);
        }

        // Isolated rooms may be used for rolling ball trick (for ex., LEVEL4.PHD).
        // Hence, this part is commented.

        /*
        if((r->portal_count == 0) && (world->room_count > 1))
        {
            Room_Disable(r);
        }
        */
    }
}


void World_MakeEntityItems(world_p world, struct RedBlackNode_s *n)
{
    base_item_p item = (base_item_p)n->data;

    for(uint32_t i = 0; i < world->room_count; i++)
    {
        engine_container_p cont = world->rooms[i].content->containers;
        for(; cont; cont = cont->next)
        {
            if(cont->object_type == OBJECT_ENTITY)
            {
                entity_p ent = (entity_p)cont->object;
                if(ent->bf->animations.model->id == item->world_model_id)
                {
                    char buf[64] = {0};
                    snprintf(buf, 64, "if(entity_funcs[%d]==nil) then entity_funcs[%d]={} end", ent->id, ent->id);
                    luaL_dostring(engine_lua, buf);
                    snprintf(buf, 32, "pickup_init(%d, %d);", ent->id, item->id);
                    luaL_dostring(engine_lua, buf);
                    Entity_DisableCollision(ent);
                }
            }
        }
    }

    if(n->right)
    {
        World_MakeEntityItems(world, n->right);
    }

    if(n->left)
    {
        World_MakeEntityItems(world, n->left);
    }
}
