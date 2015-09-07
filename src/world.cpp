
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "core/gl_util.h"
#include "core/console.h"
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
#include "engine.h"
#include "engine_lua.h"
#include "engine_physics.h"
#include "script.h"
#include "gui.h"
#include "resource.h"
#include "inventory.h"


int  compEntityEQ(void *x, void *y);
int  compEntityLT(void *x, void *y);
void RBEntityFree(void *x);
void RBItemFree(void *x);
void World_GenRBTrees(struct world_s *world);
void World_SwapRoomPortals(world_p world, struct room_s *room, struct room_s *dest_room);

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
}


void World_Open(struct world_s *world, class VT_Level *tr)
{
    World_Clear(world);

    world->version = tr->game_version;

    Res_ScriptsOpen(world->version);    // Open configuration scripts.
    Gui_DrawLoadScreen(150);

    World_GenRBTrees(world);            // Generate red-black trees
    Gui_DrawLoadScreen(200);

    TR_GenTextures(world, tr);          // Generate OGL textures
    Gui_DrawLoadScreen(300);

    TR_GenAnimCommands(world, tr);      // Copy anim commands
    Gui_DrawLoadScreen(310);

    TR_GenAnimTextures(world, tr);      // Generate animated textures
    Gui_DrawLoadScreen(320);

    TR_GenMeshes(world, tr);            // Generate all meshes
    Gui_DrawLoadScreen(400);

    TR_GenSprites(world, tr);           // Generate all sprites
    Gui_DrawLoadScreen(420);

    TR_GenBoxes(world, tr);             // Generate boxes.
    Gui_DrawLoadScreen(440);

    TR_GenCameras(world, tr);           // Generate cameras & sinks.
    Gui_DrawLoadScreen(460);

    TR_GenRooms(world, tr);             // Build all rooms
    Gui_DrawLoadScreen(500);

    Res_GenRoomFlipMap(world);          // Generate room flipmaps
    Gui_DrawLoadScreen(520);

    // Build all skeletal models. Must be generated before TR_Sector_Calculate() function.

    TR_GenSkeletalModels(world, tr);
    Gui_DrawLoadScreen(600);

    TR_GenEntities(world, tr);          // Build all moveables (entities)
    Gui_DrawLoadScreen(650);

    Res_GenBaseItems(world);             // Generate inventory item entries.
    Gui_DrawLoadScreen(680);

    // Generate sprite buffers. Only now because entity generation adds new sprites

    Res_GenSpritesBuffer(world);
    Gui_DrawLoadScreen(700);

    TR_GenRoomProperties(world, tr);
    Gui_DrawLoadScreen(750);

    Res_GenRoomCollision(world);
    Gui_DrawLoadScreen(800);

    // Initialize audio.

    TR_GenSamples(world, tr);
    Gui_DrawLoadScreen(850);

    // Find and set skybox.

    world->sky_box = Res_GetSkybox(world, world->version);
    Gui_DrawLoadScreen(860);

    // Generate entity functions.

    if(world->entity_tree->root)
    {
        Res_GenEntityFunctions(world->entity_tree->root);
    }
    Gui_DrawLoadScreen(910);

    // Load entity collision flags and ID overrides from script.

    Res_ScriptsClose();
    Gui_DrawLoadScreen(940);

    // Generate VBOs for meshes.

    Res_GenVBOs(world);
    Gui_DrawLoadScreen(950);

    // Process level autoexec loading.

    Res_AutoexecOpen(world->version);
    Gui_DrawLoadScreen(960);

    // Fix initial room states

    Res_FixRooms(world);
    Gui_DrawLoadScreen(970);
}


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
