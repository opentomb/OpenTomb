#ifndef _L_MAIN_H_
#define _L_MAIN_H_

#include <SDL2/SDL_rwops.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tr_types.h"
#include "tr_versions.h"


// Audio map size is a size of effect ID array, which is used to translate
// global effect IDs to level effect IDs. If effect ID in audio map is -1
// (0xFFFF), it means that this effect is absent in current level.
// Normally, audio map size is a constant for each TR game version and
// won't change from level to level.

#define TR_AUDIO_MAP_SIZE_NONE (-1)
#define TR_AUDIO_MAP_SIZE_TR1  256
#define TR_AUDIO_MAP_SIZE_TR2  370
#define TR_AUDIO_MAP_SIZE_TR3  370
#define TR_AUDIO_MAP_SIZE_TR4  370
#define TR_AUDIO_MAP_SIZE_TR5  450

// Default range and pitch values are required for compatibility with
// TR1 and TR2 levels, as there is no such parameters in SoundDetails
// structures.

#define TR_AUDIO_DEFAULT_RANGE 8
#define TR_AUDIO_DEFAULT_PITCH 1.0       // 0.0 - only noise

/** \brief A complete TR level.
  *
  * This contains all necessary functions to load a TR level.
  * Some corrections to the data are done, like converting to OpenGLs coordinate system.
  * All indexes are converted, so they can be used directly.
  * Endian conversion is done at the lowest possible layer, most of the time this is in the read_bitxxx functions.
  */
class TR_Level 
{
      public:
        TR_Level()
        {
            this->game_version = TR_UNKNOWN;
            strncpy(this->sfx_path, "MAIN.SFX", 256);
            
            this->textile8_count = 0;
            this->textile16_count = 0;
            this->textile32_count = 0;
            this->textile8 = NULL;
            this->textile16 = NULL;
            this->textile32 = NULL;
            
            this->floor_data_size = 0;          // destroyed
            this->floor_data = NULL;            // destroyed
            this->mesh_indices_count = 0;       // destroyed
            this->mesh_indices = NULL;          // destroyed
            
            this->animations_count = 0;         // destroyed
            this->animations = NULL;            // destroyed
            this->state_changes_count = 0;      // destroyed
            this->state_changes = NULL;         // destroyed
            this->anim_dispatches_count = 0;    // destroyed
            this->anim_dispatches = NULL;       // destroyed
            this->anim_commands_count = 0;      // destroyed
            this->anim_commands = NULL;         // destroyed
            
            this->moveables_count = 0;          // destroyed
            this->moveables = NULL;             // destroyed
            this->static_meshes_count = 0;      // destroyed
            this->static_meshes = NULL;         // destroyed
            this->object_textures_count = 0;    // destroyed
            this->object_textures = NULL;       // destroyed
            this->animated_textures_count = 0;  // destroyed
            this->animated_textures_uv_count = 0; // destroyed
            this->animated_textures = NULL;     // destroyed
            this->sprite_textures_count = 0;    // destroyed
            this->sprite_textures = NULL;       // destroyed
            this->sprite_sequences_count = 0;   // destroyed
            this->sprite_sequences = NULL;      // destroyed
            this->cameras_count = 0;            // destroyed
            this->cameras = NULL;               // destroyed
            this->flyby_cameras_count = 0;      // destroyed
            this->flyby_cameras = NULL;         // destroyed
            this->sound_sources_count = 0;      // destroyed
            this->sound_sources = NULL;         // destroyed
            
            this->boxes_count = 0;              // destroyed
            this->boxes = NULL;                 // destroyed
            this->overlaps_count = 0;           // destroyed
            this->overlaps = NULL;              // destroyed
            this->zones = NULL;                 // destroyed
            this->items_count = 0;              // destroyed
            this->items = NULL;                 // destroyed
            this->ai_objects_count = 0;         // destroyed
            this->ai_objects = NULL;            // destroyed
            this->cinematic_frames_count = 0;   // destroyed
            this->cinematic_frames = NULL;      // destroyed
            
            this->demo_data_count = 0;          // destroyed
            this->demo_data = NULL;             // destroyed
            this->soundmap = NULL;              // destroyed
            this->sound_details_count = 0;      // destroyed
            this->sound_details = NULL;         // destroyed
            this->sample_indices_count = 0;     // destroyed
            this->sample_indices = NULL;        // destroyed
            this->samples_data_size = 0;
            this->samples_count = 0;            // destroyed
            this->samples_data = NULL;          // destroyed

            this->frame_data_size = 0;          // destroyed
            this->frame_data = NULL;            // destroyed
            this->mesh_tree_data_size = 0;      // destroyed
            this->mesh_tree_data = NULL;        // destroyed
            
            this->meshes_count = 0;             // destroyed
            this->meshes = NULL;                // destroyed
            this->rooms_count = 0;              // destroyed
            this->rooms = NULL;                 // destroyed
        }
        
        virtual ~TR_Level()
        {
            uint32_t i;
            
            /**destroy all textiles**/
            if(this->textile8_count)
            {
                this->textile8_count = 0; 
                free(this->textile8); 
                this->textile8 = NULL; 
            }
            
            if(this->textile16_count)
            {
                this->textile16_count = 0; 
                free(this->textile16); 
                this->textile16 = NULL; 
            }
            
            if(this->textile32_count)
            {
                this->textile32_count = 0; 
                free(this->textile32); 
                this->textile32 = NULL; 
            }
            
            /**destroy other data**/
            if(this->floor_data_size)
            {
                this->floor_data_size = 0; 
                free(this->floor_data); 
                this->floor_data = NULL; 
            }
            
            if(this->mesh_indices_count)
            {
                this->mesh_indices_count = 0; 
                free(this->mesh_indices); 
                this->mesh_indices = NULL; 
            }
            
            if(this->animations_count)
            {
                this->animations_count = 0; 
                free(this->animations); 
                this->animations = NULL; 
            }
            
            if(this->state_changes_count)
            {
                this->state_changes_count = 0; 
                free(this->state_changes); 
                this->state_changes = NULL; 
            }
            
            if(this->anim_dispatches_count)
            {
                this->anim_dispatches_count = 0; 
                free(this->anim_dispatches); 
                this->anim_dispatches = NULL; 
            }
            
            if(this->anim_commands_count)
            {
                this->anim_commands_count = 0; 
                free(this->anim_commands); 
                this->anim_commands = NULL; 
            }            
            
            if(this->moveables_count)
            {
                this->moveables_count = 0; 
                free(this->moveables); 
                this->moveables = NULL; 
            }
            
            if(this->static_meshes_count)
            {
                this->static_meshes_count = 0; 
                free(this->static_meshes); 
                this->static_meshes = NULL; 
            }
            
            if(this->object_textures_count)
            {
                this->object_textures_count = 0; 
                free(this->object_textures); 
                this->object_textures = NULL; 
            }
            
            if(this->animated_textures_count)
            {
                this->animated_textures_count = 0;
                free(this->animated_textures); 
                this->animated_textures = NULL; 
            }
            
            if(this->sprite_textures_count)
            {
                this->sprite_textures_count = 0; 
                free(this->sprite_textures); 
                this->sprite_textures = NULL; 
            }
            
            if(this->sprite_sequences_count)
            {
                this->sprite_sequences_count = 0; 
                free(this->sprite_sequences); 
                this->sprite_sequences = NULL; 
            }
            
            if(this->cameras_count)
            {
                this->cameras_count = 0; 
                free(this->cameras); 
                this->cameras = NULL; 
            }
            
            if(this->flyby_cameras_count)
            {
                this->flyby_cameras_count = 0; 
                free(this->flyby_cameras); 
                this->flyby_cameras = NULL; 
            }
            
            if(this->sound_sources_count)
            {
                this->sound_sources_count = 0; 
                free(this->sound_sources); 
                this->sound_sources = NULL; 
            }
            
            if(this->boxes_count)
            {
                this->boxes_count = 0; 
                free(this->boxes); 
                this->boxes = NULL; 
                free(this->zones); 
                this->zones = NULL; 
            }
            
            if(this->overlaps_count)
            {
                this->overlaps_count = 0; 
                free(this->overlaps); 
                this->overlaps = NULL; 
            }
            
            if(this->items_count)
            {
                this->items_count = 0; 
                free(this->items); 
                this->items = NULL; 
            }
            
            if(this->ai_objects_count)
            {
                this->ai_objects_count = 0; 
                free(this->ai_objects); 
                this->ai_objects = NULL; 
            }
            
            if(this->cinematic_frames_count)
            {
                this->cinematic_frames_count = 0; 
                free(this->cinematic_frames); 
                this->cinematic_frames = NULL; 
            }

            if(this->demo_data_count)
            {
                this->demo_data_count = 0; 
                free(this->demo_data); 
                this->demo_data = NULL; 
            }
            
            if(this->soundmap)
            {
                free(this->soundmap);
                this->soundmap = NULL;
            }

            if(this->sound_details_count)
            {
                this->sound_details_count = 0; 
                free(this->sound_details); 
                this->sound_details = NULL; 
            }
            
            if(this->samples_data)
            {
                this->samples_data_size = 0;
                this->samples_count = 0; 
                free(this->samples_data); 
                this->samples_data = NULL; 
            }
            
            if(this->sample_indices_count)
            {
                this->sample_indices_count = 0; 
                free(this->sample_indices); 
                this->sample_indices = NULL; 
            }
           
            if(this->frame_data_size)
            {
                this->frame_data_size = 0; 
                free(this->frame_data); 
                this->frame_data = NULL; 
            }

            if(this->mesh_tree_data_size)
            {
                this->mesh_tree_data_size = 0; 
                free(this->mesh_tree_data); 
                this->mesh_tree_data = NULL; 
            }
            
            
            if(this->meshes_count)
            {
                for(i = 0; i < this->meshes_count; i ++)
                {
                    if(this->meshes[i].lights)
                    {
                        free(this->meshes[i].lights);
                        this->meshes[i].lights = NULL;
                    }
                    
                    if(this->meshes[i].num_textured_triangles)
                    {
                        free(this->meshes[i].textured_triangles);
                        this->meshes[i].textured_triangles = NULL;
                        this->meshes[i].num_textured_triangles = 0;
                    }
                    
                    if(this->meshes[i].num_textured_rectangles)
                    {
                        free(this->meshes[i].textured_rectangles);
                        this->meshes[i].textured_rectangles = NULL;
                        this->meshes[i].num_textured_rectangles = 0;
                    }
                    
                    if(this->meshes[i].num_coloured_triangles)
                    {
                        free(this->meshes[i].coloured_triangles);
                        this->meshes[i].coloured_triangles = NULL;
                        this->meshes[i].num_coloured_triangles = 0;
                    }
                    
                    if(this->meshes[i].num_coloured_rectangles)
                    {
                        free(this->meshes[i].coloured_rectangles);
                        this->meshes[i].coloured_rectangles = NULL;
                        this->meshes[i].num_coloured_rectangles = 0;
                    }
                    
                    if(this->meshes[i].normals)
                    {
                        free(this->meshes[i].normals);
                        this->meshes[i].normals = NULL;
                    }
                    
                    if(this->meshes[i].vertices)
                    {
                        free(this->meshes[i].vertices);
                        this->meshes[i].vertices = NULL;
                    }
                }
                this->meshes_count = 0; 
                free(this->meshes); 
                this->meshes = NULL; 
            }
            
            if(this->rooms_count)
            {
                for(i = 0; i < this->rooms_count; i ++)
                {
                    if(this->rooms[i].num_layers)
                    {
                        this->rooms[i].num_layers = 0;
                        free(this->rooms[i].layers);
                        this->rooms[i].layers = NULL;
                    }
                    
                    if(this->rooms[i].num_lights)
                    {
                        this->rooms[i].num_lights = 0;
                        free(this->rooms[i].lights);
                        this->rooms[i].lights = NULL;
                    }
                    
                    if(this->rooms[i].num_portals)
                    {
                        this->rooms[i].num_portals = 0;
                        free(this->rooms[i].portals);
                        this->rooms[i].portals = NULL;
                    }
                    
                    if(this->rooms[i].num_xsectors * this->rooms[i].num_zsectors)
                    {
                        this->rooms[i].num_xsectors = 0;
                        this->rooms[i].num_zsectors = 0;
                        free(this->rooms[i].sector_list);
                        this->rooms[i].sector_list = NULL;
                    }
                    
                    if(this->rooms[i].num_sprites)
                    {
                        this->rooms[i].num_sprites = 0;
                        free(this->rooms[i].sprites);
                        this->rooms[i].sprites = NULL;
                    }
                    
                    if(this->rooms[i].num_static_meshes)
                    {
                        this->rooms[i].num_static_meshes = 0;
                        free(this->rooms[i].static_meshes);
                        this->rooms[i].static_meshes = NULL;
                    }                    
                    
                    if(this->rooms[i].num_triangles)
                    {
                        this->rooms[i].num_triangles = 0;
                        free(this->rooms[i].triangles);
                        this->rooms[i].triangles = NULL;
                    }
                    
                    if(this->rooms[i].num_rectangles)
                    {
                        this->rooms[i].num_rectangles = 0;
                        free(this->rooms[i].rectangles);
                        this->rooms[i].rectangles = NULL;
                    }
                    
                    if(this->rooms[i].num_vertices)
                    {
                        this->rooms[i].num_vertices = 0;
                        free(this->rooms[i].vertices);
                        this->rooms[i].vertices = NULL;
                    }
                }
                this->rooms_count = 0;
                free(this->rooms);
                this->rooms = NULL;
            }
        }
        
    int32_t game_version;                   ///< \brief game engine version.
    
    uint32_t textile8_count;
    uint32_t textile16_count;
    uint32_t textile32_count;
    tr_textile8_t *textile8;                ///< \brief 8-bit 256x256 textiles(TR1-3).
    tr2_textile16_t *textile16;             ///< \brief 16-bit 256x256 textiles(TR2-5).
    tr4_textile32_t *textile32;             ///< \brief 32-bit 256x256 textiles(TR4-5).
    uint32_t rooms_count;
    tr5_room_t *rooms;                      ///< \brief all rooms (normal and alternate).
    uint32_t floor_data_size;               ///< \brief the floor data size
    uint16_t *floor_data;                   ///< \brief the floor data.
    uint32_t meshes_count;
    tr4_mesh_t *meshes;                     ///< \brief all meshes (static and moveables).
    uint32_t mesh_indices_count;
    uint32_t *mesh_indices;                 ///< \brief mesh index table.
    uint32_t animations_count;
    tr_animation_t *animations;             ///< \brief animations for moveables.
    uint32_t state_changes_count;
    tr_state_change_t *state_changes;       ///< \brief state changes for moveables.
    uint32_t anim_dispatches_count;
    tr_anim_dispatch_t *anim_dispatches;    ///< \brief animation dispatches for moveables.
    uint32_t anim_commands_count;
    int16_t *anim_commands;                 ///< \brief animation commands for moveables.
    uint32_t moveables_count;
    tr_moveable_t *moveables;               ///< \brief data for the moveables.
    uint32_t static_meshes_count;
    tr_staticmesh_t *static_meshes;         ///< \brief data for the static meshes.
    uint32_t object_textures_count;
    tr4_object_texture_t *object_textures;  ///< \brief object texture definitions.
    uint32_t animated_textures_count;
    uint16_t *animated_textures;            ///< \brief animated textures.
    uint32_t animated_textures_uv_count;
    uint32_t sprite_textures_count;
    tr_sprite_texture_t *sprite_textures;   ///< \brief sprite texture definitions.
    uint32_t sprite_sequences_count;
    tr_sprite_sequence_t *sprite_sequences; ///< \brief sprite sequences for animation.
    uint32_t cameras_count;
    tr_camera_t *cameras;                   ///< \brief cameras.
    uint32_t flyby_cameras_count;
    tr4_flyby_camera_t *flyby_cameras;      ///< \brief flyby cameras.
    uint32_t sound_sources_count;
    tr_sound_source_t *sound_sources;       ///< \brief sound sources.
    uint32_t boxes_count;
    tr_box_t *boxes;                        ///< \brief boxes.
    tr2_zone_t *zones;                      ///< \brief zones.
    uint32_t overlaps_count;
    uint16_t *overlaps;                     ///< \brief overlaps.
    uint32_t items_count;
    tr2_item_t *items;                      ///< \brief items.
    tr_lightmap_t lightmap;                 ///< \brief ligthmap (TR1-3).
    tr2_palette_t palette;                  ///< \brief colour palette (TR1-3).
    tr2_palette_t palette16;                ///< \brief colour palette (TR2-3).
    uint32_t ai_objects_count;
    tr4_ai_object_t *ai_objects;            ///< \brief ai objects (TR4-5).
    uint32_t cinematic_frames_count;
    tr_cinematic_frame_t *cinematic_frames; ///< \brief cinematic frames (TR1-3).
    uint32_t demo_data_count;
    uint8_t *demo_data;                     ///< \brief demo data.
    int16_t *soundmap;                      ///< \brief soundmap (TR: 256 values TR2-4: 370 values TR5: 450 values).
    uint32_t sound_details_count;
    tr_sound_details_t *sound_details;      ///< \brief sound details.
    uint32_t samples_count;
    uint32_t samples_data_size;
    uint8_t *samples_data;                  ///< \brief samples.
    uint32_t sample_indices_count;
    uint32_t *sample_indices;               ///< \brief sample indices.

    uint32_t frame_data_size;               ///< \brief frame data array size
    uint16_t *frame_data;                   ///< \brief frame data array
    uint32_t mesh_tree_data_size;
    uint32_t *mesh_tree_data;
        
    char     sfx_path[256];
        
    void read_level(const char *filename, int32_t game_version);
    void read_level(SDL_RWops * const src, int32_t game_version);
    tr_mesh_thee_tag_t get_mesh_tree_tag_for_model(tr_moveable_t *model, int index);
    void get_anim_frame_data(tr5_vertex_t min_max_pos[3], tr5_vertex_t *rotations, int meshes_count, tr_animation_t *anim, int frame);
    
    protected:
    uint32_t num_textiles;          ///< \brief number of 256x256 textiles.
    uint32_t num_room_textiles;     ///< \brief number of 256x256 room textiles (TR4-5).
    uint32_t num_obj_textiles;      ///< \brief number of 256x256 object textiles (TR4-5).
    uint32_t num_bump_textiles;     ///< \brief number of 256x256 bump textiles (TR4-5).
    uint32_t num_misc_textiles;     ///< \brief number of 256x256 misc textiles (TR4-5).
    bool read_32bit_textiles;       ///< \brief are other 32bit textiles than misc ones read?

    int8_t read_bit8(SDL_RWops * const src);
    uint8_t read_bitu8(SDL_RWops * const src);
    int16_t read_bit16(SDL_RWops * const src);
    uint16_t read_bitu16(SDL_RWops * const src);
    int32_t read_bit32(SDL_RWops * const src);
    uint32_t read_bitu32(SDL_RWops * const src);
    float read_float(SDL_RWops * const src);
    float read_mixfloat(SDL_RWops * const src);

    void read_mesh_data(SDL_RWops * const src);
    void read_frame_moveable_data(SDL_RWops * const src);

    void read_tr_colour(SDL_RWops * const src, tr2_colour_t & colour);
    void read_tr_vertex16(SDL_RWops * const src, tr5_vertex_t & vertex);
    void read_tr_vertex32(SDL_RWops * const src, tr5_vertex_t & vertex);
    void read_tr_face3(SDL_RWops * const src, tr4_face3_t & face);
    void read_tr_face4(SDL_RWops * const src, tr4_face4_t & face);
    void read_tr_textile8(SDL_RWops * const src, tr_textile8_t & textile);
    void read_tr_lightmap(SDL_RWops * const src, tr_lightmap_t & lightmap);
    void read_tr_palette(SDL_RWops * const src, tr2_palette_t & palette);
    void read_tr_box(SDL_RWops * const src, tr_box_t & box);
    void read_tr_zone(SDL_RWops * const src, tr2_zone_t & zone);
    void read_tr_room_sprite(SDL_RWops * const src, tr_room_sprite_t & room_sprite);
    void read_tr_room_portal(SDL_RWops * const src, tr_room_portal_t & portal);
    void read_tr_room_sector(SDL_RWops * const src, tr_room_sector_t & room_sector);
    void read_tr_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert);
    void read_tr_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture);
    void read_tr_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture);
    void read_tr_sprite_sequence(SDL_RWops * const src, tr_sprite_sequence_t & sprite_sequence);
    void read_tr_mesh(SDL_RWops * const src, tr4_mesh_t & mesh);
    void read_tr_state_changes(SDL_RWops * const src, tr_state_change_t & state_change);
    void read_tr_anim_dispatches(SDL_RWops * const src, tr_anim_dispatch_t & anim_dispatch);
    void read_tr_animation(SDL_RWops * const src, tr_animation_t & animation);
    void read_tr_moveable(SDL_RWops * const src, tr_moveable_t & moveable);
    void read_tr_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr_cinematic_frame(SDL_RWops * const src, tr_cinematic_frame_t & cf);
    void read_tr_staticmesh(SDL_RWops * const src, tr_staticmesh_t & mesh);
    void read_tr_level(SDL_RWops * const src, bool demo_or_ub);

    void read_tr2_colour4(SDL_RWops * const src, tr2_colour_t & colour);
    void read_tr2_palette16(SDL_RWops * const src, tr2_palette_t & palette16);
    void read_tr2_textile16(SDL_RWops * const src, tr2_textile16_t & textile);
    void read_tr2_box(SDL_RWops * const src, tr_box_t & box);
    void read_tr2_zone(SDL_RWops * const src, tr2_zone_t & zone);
    void read_tr2_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr2_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr2_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr2_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr2_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr2_level(SDL_RWops * const src, bool demo);

    void read_tr3_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr3_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
    void read_tr3_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr3_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr3_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr3_level(SDL_RWops * const src);

    void read_tr4_vertex_float(SDL_RWops * const src, tr5_vertex_t & vertex);
    void read_tr4_textile32(SDL_RWops * const src, tr4_textile32_t & textile);
    void read_tr4_face3(SDL_RWops * const src, tr4_face3_t & meshface);
    void read_tr4_face4(SDL_RWops * const src, tr4_face4_t & meshface);
    void read_tr4_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr4_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex);
     void read_tr4_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh);
    void read_tr4_room(SDL_RWops * const src, tr5_room_t & room);
    void read_tr4_item(SDL_RWops * const src, tr2_item_t & item);
    void read_tr4_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert);
    void read_tr4_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture);
    void read_tr4_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture);
    void read_tr4_mesh(SDL_RWops * const src, tr4_mesh_t & mesh);
    void read_tr4_animation(SDL_RWops * const src, tr_animation_t & animation);
    void read_tr4_level(SDL_RWops * const _src);

    void read_tr5_room_light(SDL_RWops * const src, tr5_room_light_t & light);
    void read_tr5_room_layer(SDL_RWops * const src, tr5_room_layer_t & layer);
    void read_tr5_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & vert);
    void read_tr5_room(SDL_RWops * const orgsrc, tr5_room_t & room);
    void read_tr5_moveable(SDL_RWops * const src, tr_moveable_t & moveable);
    void read_tr5_level(SDL_RWops * const src);
};

#endif // _L_MAIN_H_
