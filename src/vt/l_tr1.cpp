/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include <SDL2/SDL.h>
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

#define RCSID "$Id: l_tr1.cpp,v 1.16 2002/09/20 15:59:02 crow Exp $"

/** \brief reads rgb colour.
  *
  * Reads three rgb colour components. The read 6-bit values get shifted, so they are 8-bit.
  * The alpha value of tr2_colour_t gets set to 0.
  */
void TR_Level::read_tr_colour(SDL_RWops * const src, tr2_colour_t & colour)
{
    // read 6 bit color and change to 8 bit
    colour.r = read_bitu8(src) << 2;
    colour.g = read_bitu8(src) << 2;
    colour.b = read_bitu8(src) << 2;
    colour.a = 0;
}

/** \brief reads three 16-bit vertex components.
  *
  * The values get converted from bit16 to float. y and z are negated to fit OpenGLs coordinate system.
  */
void TR_Level::read_tr_vertex16(SDL_RWops * const src, tr5_vertex_t & vertex)
{
    // read vertex and change coordinate system
    vertex.x = (float)read_bit16(src);
    vertex.y = (float)-read_bit16(src);
    vertex.z = (float)-read_bit16(src);
}

/** \brief reads three 32-bit vertex components.
  *
  * The values get converted from bit32 to float. y and z are negated to fit OpenGLs coordinate system.
  */
void TR_Level::read_tr_vertex32(SDL_RWops * const src, tr5_vertex_t & vertex)
{
    // read vertex and change coordinate system
    vertex.x = (float)read_bit32(src);
    vertex.y = (float)-read_bit32(src);
    vertex.z = (float)-read_bit32(src);
}

/** \brief reads a triangle definition.
  *
  * The lighting value is set to 0, as it is only in TR4-5.
  */
void TR_Level::read_tr_face3(SDL_RWops * const src, tr4_face3_t & meshface)
{
    meshface.vertices[0] = read_bitu16(src);
    meshface.vertices[1] = read_bitu16(src);
    meshface.vertices[2] = read_bitu16(src);
    meshface.texture = read_bitu16(src);
    // lighting only in TR4-5
    meshface.lighting = 0;
}

/** \brief reads a triangle definition.
  *
  * The lighting value is set to 0, as it is only in TR4-5.
  */
void TR_Level::read_tr_face4(SDL_RWops * const src, tr4_face4_t & meshface)
{
    meshface.vertices[0] = read_bitu16(src);
    meshface.vertices[1] = read_bitu16(src);
    meshface.vertices[2] = read_bitu16(src);
    meshface.vertices[3] = read_bitu16(src);
    meshface.texture = read_bitu16(src);
    // only in TR4-TR5
    meshface.lighting = 0;
}

/// \brief reads a 8-bit 256x256 textile.
void TR_Level::read_tr_textile8(SDL_RWops * const src, tr_textile8_t & textile)
{
    for (int i = 0; i < 256; i++)
        if (SDL_RWread(src, textile.pixels[i], 1, 256) < 256)
                        Sys_extError("read_tr_textile8");
}

/// \brief reads the lightmap.
void TR_Level::read_tr_lightmap(SDL_RWops * const src, tr_lightmap_t & lightmap)
{
    for (int i = 0; i < (32 * 256); i++)
        lightmap.map[i] = read_bitu8(src);
}

/// \brief reads the 256 colour palette values.
void TR_Level::read_tr_palette(SDL_RWops * const src, tr2_palette_t & palette)
{
    for (int i = 0; i < 256; i++)
        read_tr_colour(src, palette.colour[i]);
}

void TR_Level::read_tr_box(SDL_RWops * const src, tr_box_t & box)
{
    box.zmax =-read_bit32(src);
    box.zmin =-read_bit32(src);
    box.xmin = read_bit32(src);
    box.xmax = read_bit32(src);
    box.true_floor =-read_bit16(src);
    box.overlap_index = read_bit16(src);
}

/// \brief reads a room sprite definition.
void TR_Level::read_tr_room_sprite(SDL_RWops * const src, tr_room_Sprite & room_sprite)
{
    room_sprite.vertex = read_bit16(src);
    room_sprite.texture = read_bit16(src);
}

/** \brief reads a room portal definition.
  *
  * A check is preformed to see wether the normal lies on a coordinate axis, if not an exception gets thrown.
  */
void TR_Level::read_tr_room_portal(SDL_RWops * const src, tr_room_portal_t & portal)
{
    portal.adjoining_room = read_bitu16(src);
    read_tr_vertex16(src, portal.normal);
    read_tr_vertex16(src, portal.vertices[0]);
    read_tr_vertex16(src, portal.vertices[1]);
    read_tr_vertex16(src, portal.vertices[2]);
    read_tr_vertex16(src, portal.vertices[3]);
    if ((portal.normal.x == 1.0f) && (portal.normal.y == 0.0f) && (portal.normal.z == 0.0f))
        return;
    if ((portal.normal.x == -1.0f) && (portal.normal.y == 0.0f) && (portal.normal.z == 0.0f))
        return;
    if ((portal.normal.x == 0.0f) && (portal.normal.y == 1.0f) && (portal.normal.z == 0.0f))
        return;
    if ((portal.normal.x == 0.0f) && (portal.normal.y == -1.0f) && (portal.normal.z == 0.0f))
        return;
    if ((portal.normal.x == 0.0f) && (portal.normal.y == 0.0f) && (portal.normal.z == 1.0f))
        return;
    if ((portal.normal.x == 0.0f) && (portal.normal.y == 0.0f) && (portal.normal.z == -1.0f))
        return;
        Sys_extWarn("read_tr_room_portal: normal not on world axis");
}

/// \brief reads a room sector definition.
void TR_Level::read_tr_room_sector(SDL_RWops * const src, tr_room_sector_t & sector)
{
    sector.fd_index = read_bitu16(src);
    sector.box_index = read_bitu16(src);
    sector.room_below = read_bitu8(src);
    sector.floor = read_bit8(src);
    sector.room_above = read_bitu8(src);
    sector.ceiling = read_bit8(src);
}

/** \brief reads a room light definition.
  *
  * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
  * intensity2 and fade2 are introduced in TR2 and are set to intensity1 and fade1 for TR1.
  */
void TR_Level::read_tr_room_light(SDL_RWops * const src, tr5_room_light_t & light)
{
    read_tr_vertex32(src, light.pos);
    // read and make consistent
    light.intensity1 = (8191 - read_bitu16(src)) << 2;
    light.fade1 = read_bitu32(src);
    // only in TR2
    light.intensity2 = light.intensity1;

    light.intensity = light.intensity1;
    light.intensity /= 4096.0f;

    if(light.intensity > 1.0f)
        light.intensity = 1.0f;

    light.fade2 = light.fade1;

    light.r_outer = light.fade1;
    light.r_inner = light.fade1 / 2;

    light.light_type = 0x01; // Point light

    // all white
    light.color.r = 0xff;
    light.color.g = 0xff;
    light.color.b = 0xff;
}

/** \brief reads a room vertex definition.
  *
  * lighting1 gets converted, so it matches the 0-32768 range introduced in TR3.
  * lighting2 is introduced in TR2 and is set to lighting1 for TR1.
  * attributes is introduced in TR2 and is set 0 for TR1.
  * All other values are introduced in TR5 and get set to appropiate values.
  */
void TR_Level::read_tr_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex)
{
    read_tr_vertex16(src, room_vertex.vertex);
    // read and make consistent
    room_vertex.lighting1 = (8191 - read_bit16(src)) << 2;
    // only in TR2
    room_vertex.lighting2 = room_vertex.lighting1;
    room_vertex.attributes = 0;
    // only in TR5
    room_vertex.normal.x = 0;
    room_vertex.normal.y = 0;
    room_vertex.normal.z = 0;
    room_vertex.colour.r = room_vertex.lighting1 / 32768.0f;
    room_vertex.colour.g = room_vertex.lighting1 / 32768.0f;
    room_vertex.colour.b = room_vertex.lighting1 / 32768.0f;
    room_vertex.colour.a = 1.0f;
}

/** \brief reads a room staticmesh definition.
  *
  * rotation gets converted to float and scaled appropiatly.
  * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
  * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
  */
void TR_Level::read_tr_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh)
{
    read_tr_vertex32(src, room_static_mesh.pos);
    room_static_mesh.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    room_static_mesh.intensity1 = read_bit16(src);
    room_static_mesh.object_id = read_bitu16(src);
    // make consistent
    if (room_static_mesh.intensity1 >= 0)
        room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
    // only in TR2
    room_static_mesh.intensity2 = room_static_mesh.intensity1;

    room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = (room_static_mesh.intensity2 / 16384.0f);
    room_static_mesh.tint.a = 1.0f;
}

/** \brief reads a room definition.
  *
  * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
  * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
  * light_mode is only in TR2 and is set 0 for TR1.
  * light_colour is only in TR3-4 and gets set appropiatly.
  */
void TR_Level::read_tr_room(SDL_RWops * const src, tr5_room_t & room)
{
    uint32_t num_data_words;
    uint32_t i;
    int64_t pos;

    // read and change coordinate system
    room.offset.x = (float)read_bit32(src);
    room.offset.y = 0;
    room.offset.z = (float)-read_bit32(src);
    room.y_bottom = (float)-read_bit32(src);
    room.y_top = (float)-read_bit32(src);

    num_data_words = read_bitu32(src);

    pos = SDL_RWseek(src, 0, RW_SEEK_CUR);

    room.num_layers = 0;

    room.num_vertices = read_bitu16(src);
    room.vertices = (tr5_room_vertex_t*)calloc(room.num_vertices, sizeof(tr5_room_vertex_t));
    for (i = 0; i < room.num_vertices; i++)
        read_tr_room_vertex(src, room.vertices[i]);

    room.num_rectangles = read_bitu16(src);
    room.rectangles = (tr4_face4_t*)malloc(room.num_rectangles * sizeof(tr4_face4_t));
    for (i = 0; i < room.num_rectangles; i++)
        read_tr_face4(src, room.rectangles[i]);

    room.num_triangles = read_bitu16(src);
    if(room.num_triangles > 0)
        room.triangles = (tr4_face3_t*)malloc(room.num_triangles * sizeof(tr4_face3_t));
    for (i = 0; i < room.num_triangles; i++)
        read_tr_face3(src, room.triangles[i]);

    room.num_sprites = read_bitu16(src);
    if(room.num_sprites > 0)
        room.sprites = (tr_room_Sprite*)malloc(room.num_sprites * sizeof(tr_room_Sprite));
    for (i = 0; i < room.num_sprites; i++)
        read_tr_room_sprite(src, room.sprites[i]);

    // set to the right position in case that there is some unused data
    SDL_RWseek(src, pos + (num_data_words * 2), RW_SEEK_SET);

    room.num_portals = read_bitu16(src);
    room.portals = (tr_room_portal_t*)malloc(room.num_portals * sizeof(tr_room_portal_t));
    for (i = 0; i < room.num_portals; i++)
        read_tr_room_portal(src, room.portals[i]);

    room.num_zsectors = read_bitu16(src);
    room.num_xsectors = read_bitu16(src);
    room.sector_list = (tr_room_sector_t*)malloc(room.num_zsectors * room.num_xsectors * sizeof(tr_room_sector_t));
    for (i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(src, room.sector_list[i]);

    // read and make consistent
    room.intensity1 = (8191 - read_bit16(src)) << 2;
    // only in TR2-TR4
    room.intensity2 = room.intensity1;
    // only in TR2
    room.light_mode = 0;

    room.num_lights = read_bitu16(src);
    room.lights = (tr5_room_light_t*)malloc(room.num_lights * sizeof(tr5_room_light_t));
    for (i = 0; i < room.num_lights; i++)
        read_tr_room_light(src, room.lights[i]);

    room.num_static_meshes = read_bitu16(src);
    if(room.num_static_meshes > 0)
        room.static_meshes = (tr2_room_staticmesh_t*)malloc(room.num_static_meshes * sizeof(tr2_room_staticmesh_t));
    for (i = 0; i < room.num_static_meshes; i++)
        read_tr_room_staticmesh(src, room.static_meshes[i]);

    room.alternate_room  = read_bit16(src);
    room.alternate_group = 0;   // Doesn't exist in TR1-3

    room.flags = read_bitu16(src);
        room.reverb_info = 2;

    room.light_colour.r = room.intensity1 / 32767.0f;
    room.light_colour.g = room.intensity1 / 32767.0f;
    room.light_colour.b = room.intensity1 / 32767.0f;
    room.light_colour.a = 1.0f;
}

/// \brief reads object texture vertex definition.
void TR_Level::read_tr_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert)
{
    vert.xcoordinate = read_bit8(src);
    vert.xpixel = read_bitu8(src);
    vert.ycoordinate = read_bit8(src);
    vert.ypixel = read_bitu8(src);
}

/** \brief reads object texture definition.
  *
  * some sanity checks get done and if they fail an exception gets thrown.
  * all values introduced in TR4 get set appropiatly.
  */
void TR_Level::read_tr_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture)
{
    object_texture.transparency_flags = read_bitu16(src);
    object_texture.tile_and_flag = read_bitu16(src);
    if (object_texture.tile_and_flag > 64)
        Sys_extWarn("object_texture.tile_and_flags > 64");

    if ((object_texture.tile_and_flag & (1 << 15)) != 0)
        Sys_extWarn("object_texture.tile_and_flags has top bit set!");

    // only in TR4
    object_texture.flags = 0;
    read_tr_object_texture_vert(src, object_texture.vertices[0]);
    read_tr_object_texture_vert(src, object_texture.vertices[1]);
    read_tr_object_texture_vert(src, object_texture.vertices[2]);
    read_tr_object_texture_vert(src, object_texture.vertices[3]);
    // only in TR4
    object_texture.unknown1 = 0;
    object_texture.unknown2 = 0;
    object_texture.x_size = 0;
    object_texture.y_size = 0;
}

/** \brief reads sprite texture definition.
  *
  * some sanity checks get done and if they fail an exception gets thrown.
  */
void TR_Level::read_tr_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture)
{
    int tx, ty, tw, th, tleft, tright, ttop, tbottom;
    float w, h;

    sprite_texture.tile = read_bitu16(src);
    if (sprite_texture.tile > 64)
        Sys_extWarn("sprite_texture.tile > 64");

    tx = read_bitu8(src);
    ty = read_bitu8(src);
    tw = read_bitu16(src);
    th = read_bitu16(src);
    tleft = read_bit16(src);
    ttop = read_bit16(src);
    tright = read_bit16(src);
    tbottom = read_bit16(src);

    w = tw / 256.0;
    h = th / 256.0;
    sprite_texture.x0 = tx;
    sprite_texture.y0 = ty;
    sprite_texture.x1 = sprite_texture.x0 + w;
    sprite_texture.y1 = sprite_texture.y0 + h;

    sprite_texture.left_side = tleft;
    sprite_texture.right_side = tright;
    sprite_texture.top_side =-tbottom;
    sprite_texture.bottom_side =-ttop;
}

/** \brief reads sprite sequence definition.
  *
  * length is negative when read and thus gets negated.
  */
void TR_Level::read_tr_sprite_sequence(SDL_RWops * const src, tr_sprite_sequence_t & sprite_sequence)
{
    sprite_sequence.object_id = read_bit32(src);
    sprite_sequence.length = -read_bit16(src);
    sprite_sequence.offset = read_bit16(src);
}

/** \brief reads mesh definition.
  *
  * The read num_normals value is positive when normals are available and negative when light
  * values are available. The values get set appropiatly.
  */
void TR_Level::read_tr_mesh(SDL_RWops * const src, tr4_mesh_t & mesh)
{
    int i;

    read_tr_vertex16(src, mesh.centre);
    mesh.collision_size = read_bit32(src);

    mesh.num_vertices = read_bit16(src);
    mesh.vertices = (tr5_vertex_t*)malloc(mesh.num_vertices * sizeof(tr5_vertex_t));
    for (i = 0; i < mesh.num_vertices; i++)
        read_tr_vertex16(src, mesh.vertices[i]);

    mesh.num_normals = read_bit16(src);
    if (mesh.num_normals >= 0) {
        mesh.num_lights = 0;
        mesh.normals = (tr5_vertex_t*)malloc(mesh.num_normals * sizeof(tr5_vertex_t));
        for (i = 0; i < mesh.num_normals; i++)
            read_tr_vertex16(src, mesh.normals[i]);
    } else {
        mesh.num_lights = -mesh.num_normals;
        mesh.num_normals = 0;
        mesh.lights = (int16_t*)malloc(mesh.num_lights * sizeof(int16_t));
        for (i = 0; i < mesh.num_lights; i++)
            mesh.lights[i] = read_bit16(src);
    }

    mesh.num_textured_rectangles = read_bit16(src);
    if(mesh.num_textured_rectangles > 0)
        mesh.textured_rectangles = (tr4_face4_t*)malloc(mesh.num_textured_rectangles * sizeof(tr4_face4_t));
    for (i = 0; i < mesh.num_textured_rectangles; i++)
        read_tr_face4(src, mesh.textured_rectangles[i]);

    mesh.num_textured_triangles = read_bit16(src);
    if(mesh.num_textured_triangles > 0)
        mesh.textured_triangles = (tr4_face3_t*)malloc(mesh.num_textured_triangles * sizeof(tr4_face3_t));
    for (i = 0; i < mesh.num_textured_triangles; i++)
        read_tr_face3(src, mesh.textured_triangles[i]);

    mesh.num_coloured_rectangles = read_bit16(src);
    if(mesh.num_coloured_rectangles > 0)
        mesh.coloured_rectangles = (tr4_face4_t*)malloc(mesh.num_coloured_rectangles * sizeof(tr4_face4_t));
    for (i = 0; i < mesh.num_coloured_rectangles; i++)
        read_tr_face4(src, mesh.coloured_rectangles[i]);

    mesh.num_coloured_triangles = read_bit16(src);
    if(mesh.num_coloured_triangles > 0)
        mesh.coloured_triangles = (tr4_face3_t*)malloc(mesh.num_coloured_triangles * sizeof(tr4_face3_t));
    for (i = 0; i < mesh.num_coloured_triangles; i++)
        read_tr_face3(src, mesh.coloured_triangles[i]);
}

/// \brief reads an animation state change.
void TR_Level::read_tr_state_changes(SDL_RWops * const src, tr_state_change_t & state_change)
{
    state_change.state_id = read_bitu16(src);
    state_change.num_anim_dispatches = read_bitu16(src);
    state_change.anim_dispatch = read_bitu16(src);
}

/// \brief reads an animation dispatch.
void TR_Level::read_tr_anim_dispatches(SDL_RWops * const src, tr_anim_dispatch_t & anim_dispatch)
{
    anim_dispatch.low = read_bit16(src);
    anim_dispatch.high = read_bit16(src);
    anim_dispatch.next_animation = read_bit16(src);
    anim_dispatch.next_frame = read_bit16(src);
}

/// \brief reads an animation definition.
void TR_Level::read_tr_animation(SDL_RWops * const src, tr_animation_t & animation)
{
    animation.frame_offset = read_bitu32(src);
    animation.frame_rate = read_bitu8(src);
    animation.frame_size = read_bitu8(src);
    animation.state_id = read_bitu16(src);

    animation.speed = read_mixfloat(src);
    animation.accel = read_mixfloat(src);

    animation.frame_start = read_bitu16(src);
    animation.frame_end = read_bitu16(src);
    animation.next_animation = read_bitu16(src);
    animation.next_frame = read_bitu16(src);

    animation.num_state_changes = read_bitu16(src);
    animation.state_change_offset = read_bitu16(src);
    animation.num_anim_commands = read_bitu16(src);
    animation.anim_command = read_bitu16(src);
}

/** \brief reads a moveable definition.
  *
  * some sanity checks get done which throw a exception on failure.
  * frame_offset needs to be corrected later in TR_Level::read_tr_level.
  */
void TR_Level::read_tr_moveable(SDL_RWops * const src, tr_moveable_t & moveable)
{
    moveable.object_id = read_bitu32(src);
    moveable.num_meshes = read_bitu16(src);
    moveable.starting_mesh = read_bitu16(src);
    moveable.mesh_tree_index = read_bitu32(src);
    moveable.frame_offset = read_bitu32(src);
    moveable.animation_index = read_bitu16(src);

    // Disable unused skybox polygons.
    if((this->game_version == TR_III) && (moveable.object_id == 355))
    {
        this->meshes[(this->mesh_indices[moveable.starting_mesh])].num_coloured_triangles = 16;
    }
}

/// \brief reads an item definition.
void TR_Level::read_tr_item(SDL_RWops * const src, tr2_item_t & item)
{
    item.object_id = read_bit16(src);
    item.room = read_bit16(src);
    read_tr_vertex32(src, item.pos);
    item.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    item.intensity1 = read_bitu16(src);
    if (item.intensity1 >= 0)
        item.intensity1 = (8191 - item.intensity1) << 2;
    item.intensity2 = item.intensity1;
    item.ocb = 0;   // Not present in TR1!
    item.flags = read_bitu16(src);
}

/// \brief reads a cinematic frame
void TR_Level::read_tr_cinematic_frame(SDL_RWops * const src, tr_cinematic_frame_t & cf)
{
    cf.roty = read_bit16(src);         // rotation about Y axis, +/- 32767 == +/- 180 degrees
    cf.rotz = read_bit16(src);         // rotation about Z axis, +/- 32767 == +/- 180 degrees
    cf.rotz2 = read_bit16(src);        // seems to work a lot like rotZ;  I haven't yet been able to
    // differentiate them
    cf.posz = read_bit16(src);         // camera position relative to something (target? Lara? room
    // origin?).  pos* are _not_ in world coordinates.
    cf.posy = read_bit16(src);         // camera position relative to something (see posZ)
    cf.posx = read_bit16(src);         // camera position relative to something (see posZ)
    cf.unknown = read_bit16(src);      // changing this can cause a runtime error
    cf.rotx = read_bit16(src);         // rotation about X axis, +/- 32767 == +/- 180 degrees
}

/// \brief reads a static mesh definition.
void TR_Level::read_tr_staticmesh(SDL_RWops * const src, tr_staticmesh_t & mesh)
{
    mesh.object_id = read_bitu32(src);
    mesh.mesh = read_bitu16(src);

    mesh.visibility_box[0].x = (float)read_bit16(src);
    mesh.visibility_box[1].x = (float)read_bit16(src);
    mesh.visibility_box[0].y = (float)-read_bit16(src);
    mesh.visibility_box[1].y = (float)-read_bit16(src);
    mesh.visibility_box[0].z = (float)-read_bit16(src);
    mesh.visibility_box[1].z = (float)-read_bit16(src);

    mesh.collision_box[0].x = (float)read_bit16(src);
    mesh.collision_box[1].x = (float)read_bit16(src);
    mesh.collision_box[0].y = (float)-read_bit16(src);
    mesh.collision_box[1].y = (float)-read_bit16(src);
    mesh.collision_box[0].z = (float)-read_bit16(src);
    mesh.collision_box[1].z = (float)-read_bit16(src);

    mesh.flags = read_bitu16(src);
}

void TR_Level::read_tr_level(SDL_RWops * const src, bool demo_or_ub)
{
    uint32_t i;

    // Version
    uint32_t file_version = read_bitu32(src);

    if (file_version != 0x00000020)
        Sys_extError("Wrong level version");

    this->num_textiles = 0;
    this->num_room_textiles = 0;
    this->num_obj_textiles = 0;
    this->num_bump_textiles = 0;
    this->num_misc_textiles = 0;
    this->read_32bit_textiles = false;

    this->textile8_count = this->num_textiles = read_bitu32(src);
    this->textile8 = (tr_textile8_t*)malloc(this->textile8_count * sizeof(tr_textile8_t));
    for (i = 0; i < this->textile8_count; i++)
        read_tr_textile8(src, this->textile8[i]);

    // Unused
    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    this->rooms_count = read_bitu16(src);
    this->rooms = (tr5_room_t*)calloc(this->rooms_count, sizeof(tr5_room_t));
    for (i = 0; i < this->rooms_count; i++)
        read_tr_room(src, this->rooms[i]);

    this->floor_data_size = read_bitu32(src);
    this->floor_data = (uint16_t*)malloc(this->floor_data_size * sizeof(uint16_t));
    for(i = 0; i < this->floor_data_size; i++)
        this->floor_data[i] = read_bitu16(src);

    read_mesh_data(src);

    this->animations_count = read_bitu32(src);
    this->animations = (tr_animation_t*)malloc(this->animations_count * sizeof(tr_animation_t));
    for (i = 0; i < this->animations_count; i++)
        read_tr_animation(src, this->animations[i]);

    this->state_changes_count = read_bitu32(src);
    this->state_changes = (tr_state_change_t*)malloc(this->state_changes_count * sizeof(tr_state_change_t));
    for (i = 0; i < this->state_changes_count; i++)
        read_tr_state_changes(src, this->state_changes[i]);

    this->anim_dispatches_count = read_bitu32(src);
    this->anim_dispatches = (tr_anim_dispatch_t*)malloc(this->anim_dispatches_count * sizeof(tr_anim_dispatch_t));
    for (i = 0; i < this->anim_dispatches_count; i++)
        read_tr_anim_dispatches(src, this->anim_dispatches[i]);

    this->anim_commands_count = read_bitu32(src);
    this->anim_commands = (int16_t*)malloc(this->anim_commands_count * sizeof(int16_t));
    for (i = 0; i < this->anim_commands_count; i++)
        this->anim_commands[i] = read_bit16(src);

    this->mesh_tree_data_size = read_bitu32(src);
    this->mesh_tree_data = (uint32_t*)malloc(this->mesh_tree_data_size * sizeof(uint32_t));
    for (i = 0; i < this->mesh_tree_data_size; i++)
        this->mesh_tree_data[i] = read_bitu32(src);                     // 4 bytes

    read_frame_moveable_data(src);

    // try to fix ugly stick
    for (i = 0; i < this->animations_count; i++)
    {
        uint32_t frame_offset = this->animations[i].frame_offset / 2;
        this->animations[i].frame_size = this->frame_data[frame_offset + 9] * 2 + 10;
    }

    this->static_meshes_count = read_bitu32(src);
    this->static_meshes = (tr_staticmesh_t*)malloc(this->static_meshes_count * sizeof(tr_staticmesh_t));
    for (i = 0; i < this->static_meshes_count; i++)
        read_tr_staticmesh(src, this->static_meshes[i]);

    this->object_textures.resize( read_bitu32(src) );
    for (i = 0; i < this->object_textures.size(); i++)
        read_tr_object_texture(src, this->object_textures[i]);

    this->sprite_textures.resize( read_bitu32(src) );
    for (i = 0; i < this->sprite_textures.size(); i++)
        read_tr_sprite_texture(src, this->sprite_textures[i]);

    this->sprite_sequences_count = read_bitu32(src);
    this->sprite_sequences = (tr_sprite_sequence_t*)malloc(this->sprite_sequences_count * sizeof(tr_sprite_sequence_t));
    for (i = 0; i < this->sprite_sequences_count; i++)
        read_tr_sprite_sequence(src, this->sprite_sequences[i]);

    if (demo_or_ub)
        read_tr_palette(src, this->palette);

    this->cameras_count = read_bitu32(src);
    this->cameras = (tr_camera_t*)malloc(this->cameras_count * sizeof(tr_camera_t));
    for (i = 0; i < this->cameras_count; i++)
    {
        this->cameras[i].x = read_bit32(src);
        this->cameras[i].y = read_bit32(src);
        this->cameras[i].z = read_bit32(src);

        this->cameras[i].room = read_bit16(src);
        this->cameras[i].unknown1 = read_bitu16(src);
    }

    this->sound_sources_count = read_bitu32(src);
    if(this->sound_sources_count > 0)
        this->sound_sources = (tr_sound_source_t*)malloc(this->sound_sources_count * sizeof(tr_sound_source_t));
    for(i = 0; i < this->sound_sources_count; i++)
    {
        this->sound_sources[i].x = read_bit32(src);
        this->sound_sources[i].y = read_bit32(src);
        this->sound_sources[i].z = read_bit32(src);

        this->sound_sources[i].sound_id = read_bitu16(src);
        this->sound_sources[i].flags = read_bitu16(src);
    }

    this->boxes_count = read_bitu32(src);
    this->boxes = (tr_box_t*)malloc(this->boxes_count * sizeof(tr_box_t));
    for (i = 0; i < this->boxes_count; i++)
        read_tr_box(src, this->boxes[i]);

    this->overlaps_count = read_bitu32(src);
    this->overlaps = (uint16_t*)malloc(this->overlaps_count * sizeof(uint16_t));
    for (i = 0; i < this->overlaps_count; i++)
        this->overlaps[i] = read_bitu16(src);

    // Zones
    SDL_RWseek(src, this->boxes_count * 12, RW_SEEK_CUR);

    this->animated_textures_count = read_bitu32(src);
    this->animated_textures_uv_count = 0; // No UVRotate in TR1
    this->animated_textures = (uint16_t*)malloc(this->animated_textures_count * sizeof(uint16_t));
    for (i = 0; i < this->animated_textures_count; i++)
    {
        this->animated_textures[i] = read_bitu16(src);
    }

    this->items_count = read_bitu32(src);
    this->items = (tr2_item_t*)malloc(this->items_count * sizeof(tr2_item_t));
    for (i = 0; i < this->items_count; i++)
        read_tr_item(src, this->items[i]);

    read_tr_lightmap(src, this->lightmap);

    if (!demo_or_ub)
        read_tr_palette(src, this->palette);

    this->cinematic_frames_count = read_bitu16(src);
    if(this->cinematic_frames_count > 0)
        this->cinematic_frames = (tr_cinematic_frame_t*)malloc(this->cinematic_frames_count * sizeof(tr_cinematic_frame_t));
    for (i = 0; i < this->cinematic_frames_count; i++)
    {
        read_tr_cinematic_frame(src, this->cinematic_frames[i]);
    }

    this->demo_data_count = read_bitu16(src);
    if(this->demo_data_count > 0)
        this->demo_data = (uint8_t*)malloc(this->demo_data_count * sizeof(uint8_t));
    for(i=0; i < this->demo_data_count; i++)
        this->demo_data[i] = read_bitu8(src);

    // Soundmap
    this->soundmap = (int16_t*)malloc(TR_AUDIO_MAP_SIZE_TR1 * sizeof(int16_t));
    for(i=0; i < TR_AUDIO_MAP_SIZE_TR1; i++)
        this->soundmap[i] = read_bit16(src);

    this->sound_details_count = read_bitu32(src);
    this->sound_details = (tr_sound_details_t*)malloc(this->sound_details_count * sizeof(tr_sound_details_t));

    for(i = 0; i < this->sound_details_count; i++)
    {
        this->sound_details[i].sample = read_bitu16(src);
        this->sound_details[i].volume = read_bitu16(src);
        this->sound_details[i].chance = read_bitu16(src);
        this->sound_details[i].num_samples_and_flags_1 = read_bitu8(src);
        this->sound_details[i].flags_2 = read_bitu8(src);
        this->sound_details[i].sound_range = TR_AUDIO_DEFAULT_RANGE;
        this->sound_details[i].pitch = (int16_t)TR_AUDIO_DEFAULT_PITCH;
    }

    // LOAD SAMPLES

    // In TR1, samples are embedded into level file as solid block, preceded by
    // block size in bytes. Sample block is followed by sample indices array.

    this->samples_count = 0;
    this->samples_data.resize( read_bitu32(src) );
    for(i=0; i < this->samples_data.size(); i++)
    {
        this->samples_data[i] = read_bitu8(src);
        if((i >= 4) && (*((uint32_t*)(this->samples_data.data()+i-4)) == 0x46464952))   /// RIFF
        {
            this->samples_count++;
        }
    }

    this->sample_indices_count = read_bitu32(src);
    this->sample_indices = (uint32_t*)malloc(this->sample_indices_count * sizeof(uint32_t));
    for(i=0; i < this->sample_indices_count; i++)
        this->sample_indices[i] = read_bitu32(src);
}
