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

#include <SDL2/SDL_endian.h>
#include <zlib.h>
#include "l_main.h"
#include "../system.h"

#define RCSID "$Id: l_tr4.cpp,v 1.14 2002/09/20 15:59:02 crow Exp $"

void TR_Level::read_tr4_vertex_float(SDL_RWops * const src, tr5_vertex_t & vertex)
{
	vertex.x = read_float(src);
	vertex.y = -read_float(src);
	vertex.z = -read_float(src);
}

void TR_Level::read_tr4_textile32(SDL_RWops * const src, tr4_textile32_t & textile)
{
	for (int i = 0; i < 256; i++) {
		if (SDL_RWread(src, textile.pixels[i], 4, 256) < 256)
			Sys_extError("read_tr4_textile32");
                        //throw TR_ReadError ("read_tr4_textile32", __FILE__, __LINE__, RCSID);

		for (int j = 0; j < 256; j++)
			textile.pixels[i][j] = SDL_SwapLE32(textile.pixels[i][j] & 0xff00ff00 | ((textile.pixels[i][j] & 0x00ff0000) >> 16) | ((textile.pixels[i][j] & 0x000000ff) << 16));
	}
}

void TR_Level::read_tr4_face3(SDL_RWops * const src, tr4_face3_t & meshface)
{
	meshface.vertices[0] = read_bitu16(src);
	meshface.vertices[1] = read_bitu16(src);
	meshface.vertices[2] = read_bitu16(src);
	meshface.texture = read_bitu16(src);
	meshface.lighting = read_bitu16(src);
}

void TR_Level::read_tr4_face4(SDL_RWops * const src, tr4_face4_t & meshface)
{
	meshface.vertices[0] = read_bitu16(src);
	meshface.vertices[1] = read_bitu16(src);
	meshface.vertices[2] = read_bitu16(src);
	meshface.vertices[3] = read_bitu16(src);
	meshface.texture = read_bitu16(src);
	meshface.lighting = read_bitu16(src);
}

void TR_Level::read_tr4_room_light(SDL_RWops * const src, tr5_room_light_t & light)
{
	read_tr_vertex32(src, light.pos);
	read_tr_colour(src, light.color);
	light.light_type = read_bitu8(src);
	light.unknown = read_bitu8(src);
	light.intensity1 = read_bitu8(src);
	light.r_inner = read_float(src);
	light.r_outer = read_float(src);
	light.length = read_float(src);
	light.cutoff = read_float(src);
	read_tr4_vertex_float(src, light.dir);
}

void TR_Level::read_tr4_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex)
{
	read_tr_vertex16(src, room_vertex.vertex);
	// read and make consistent
	room_vertex.lighting1 = read_bit16(src);
	room_vertex.attributes = read_bitu16(src);
	room_vertex.lighting2 = read_bit16(src);
	// only in TR5
	room_vertex.normal.x = 0;
	room_vertex.normal.y = 0;
	room_vertex.normal.z = 0;
	room_vertex.colour.r = room_vertex.lighting1 / 16383.0f;
	room_vertex.colour.g = room_vertex.lighting1 / 16383.0f;
	room_vertex.colour.b = room_vertex.lighting1 / 16383.0f;
	room_vertex.colour.a = 255;
}

void TR_Level::read_tr4_room(SDL_RWops * const src, tr5_room_t & room)
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

	pos = SDL_RWseek(src, 0, SEEK_CUR);

	room.num_layers = 0;

	room.num_vertices = read_bitu16(src);
	room.vertices = (tr5_room_vertex_t*)calloc(room.num_vertices, sizeof(tr5_room_vertex_t));
	for (i = 0; i < room.num_vertices; i++)
		read_tr4_room_vertex(src, room.vertices[i]);

	room.num_rectangles = read_bitu16(src);
	room.rectangles = (tr4_face4_t*)malloc(room.num_rectangles * sizeof(tr4_face4_t));
	for (i = 0; i < room.num_rectangles; i++)
		read_tr_face4(src, room.rectangles[i]);

	room.num_triangles = read_bitu16(src);
	room.triangles = (tr4_face3_t*)malloc(room.num_triangles * sizeof(tr4_face3_t));
	for (i = 0; i < room.num_triangles; i++)
		read_tr_face3(src, room.triangles[i]);

	room.num_sprites = read_bitu16(src);
	room.sprites = (tr_room_sprite_t*)malloc(room.num_sprites * sizeof(tr_room_sprite_t));
	for (i = 0; i < room.num_sprites; i++)
		read_tr_room_sprite(src, room.sprites[i]);

	// set to the right position in case that there is some unused data
	SDL_RWseek(src, pos + (num_data_words * 2), SEEK_SET);

	room.num_portals = read_bitu16(src);
	room.portals = (tr_room_portal_t*)malloc(room.num_portals * sizeof(tr_room_portal_t));
	for (i = 0; i < room.num_portals; i++)
		read_tr_room_portal(src, room.portals[i]);

	room.num_zsectors = read_bitu16(src);
	room.num_xsectors = read_bitu16(src);
	room.sector_list = (tr_room_sector_t*)malloc(room.num_zsectors * room.num_xsectors * sizeof(tr_room_sector_t));
	for (i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
		read_tr_room_sector(src, room.sector_list[i]);

	room.intensity1 = read_bit16(src);
	room.intensity2 = read_bit16(src);

	// only in TR2
	room.light_mode = 0;

	room.num_lights = read_bitu16(src);
	room.lights = (tr5_room_light_t*)malloc(room.num_lights * sizeof(tr5_room_light_t));
	for (i = 0; i < room.num_lights; i++)
		read_tr4_room_light(src, room.lights[i]);

	room.num_static_meshes = read_bitu16(src);
	room.static_meshes = (tr2_room_staticmesh_t*)malloc(room.num_static_meshes * sizeof(tr2_room_staticmesh_t));
	for (i = 0; i < room.num_static_meshes; i++)
		read_tr3_room_staticmesh(src, room.static_meshes[i]);

	room.alternate_room = read_bit16(src);
	room.flags = read_bitu16(src);

	// only in TR3-TR4
	read_tr_colour(src, room.fog_colour);
	room.light_colour.r = room.intensity1 / 32767.0f;
	room.light_colour.g = room.intensity1 / 32767.0f;
	room.light_colour.b = room.intensity1 / 32767.0f;
	room.light_colour.a = 1.0f;
}

void TR_Level::read_tr4_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert)
{
	vert.xcoordinate = read_bit8(src);
	vert.xpixel = read_bitu8(src);
	vert.ycoordinate = read_bit8(src);
	vert.ypixel = read_bitu8(src);
	if (vert.xcoordinate == 0)
		vert.xcoordinate = 1;
	if (vert.ycoordinate == 0)
		vert.ycoordinate = 1;
}

void TR_Level::read_tr4_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture)
{
	object_texture.transparency_flags = read_bitu16(src);
	object_texture.tile_and_flag = read_bitu16(src);
	if ((object_texture.tile_and_flag & 0x7FFF) > 128)
                Sys_extWarn("object_texture.tile > 128");

	object_texture.flags = read_bitu16(src);
	read_tr4_object_texture_vert(src, object_texture.vertices[0]);
	read_tr4_object_texture_vert(src, object_texture.vertices[1]);
	read_tr4_object_texture_vert(src, object_texture.vertices[2]);
	read_tr4_object_texture_vert(src, object_texture.vertices[3]);
	object_texture.unknown1 = read_bitu32(src);
	object_texture.unknown2 = read_bitu32(src);
	object_texture.x_size = read_bitu32(src);
	object_texture.y_size = read_bitu32(src);
}

 /*
  * tr4 + sprite loading
  */
void TR_Level::read_tr4_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture)
{
        int tx, ty, tw, th, tleft, tright, ttop, tbottom;
        
	sprite_texture.tile = read_bitu16(src);
	if (sprite_texture.tile > 128)
                Sys_extWarn("sprite_texture.tile > 128");

	tx = read_bitu8(src);
	ty = read_bitu8(src);
	tw = read_bitu16(src);
	th = read_bitu16(src);
	tleft = read_bit16(src);
	ttop = read_bit16(src);
	tright = read_bit16(src);
	tbottom = read_bit16(src);
        
        sprite_texture.x0 = tleft;
        sprite_texture.x1 = tright;
        sprite_texture.y0 = tbottom;
        sprite_texture.y1 = ttop;

        sprite_texture.left_side = tx;
        sprite_texture.right_side = tx + tw / (256);
        sprite_texture.bottom_side = ty;
        sprite_texture.top_side = ty + th / (256);
}

void TR_Level::read_tr4_mesh(SDL_RWops * const src, tr4_mesh_t & mesh)
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
	mesh.textured_rectangles = (tr4_face4_t*)malloc(mesh.num_textured_rectangles * sizeof(tr4_face4_t));
	for (i = 0; i < mesh.num_textured_rectangles; i++)
		read_tr4_face4(src, mesh.textured_rectangles[i]);

	mesh.num_textured_triangles = read_bit16(src);
	mesh.textured_triangles = (tr4_face3_t*)malloc(mesh.num_textured_triangles * sizeof(tr4_face3_t));
	for (i = 0; i < mesh.num_textured_triangles; i++)
		read_tr4_face3(src, mesh.textured_triangles[i]);

	mesh.num_coloured_rectangles = 0;
	mesh.num_coloured_triangles = 0;
}

/// \brief reads an animation definition.
void TR_Level::read_tr4_animation(SDL_RWops * const src, tr_animation_t & animation)
{
	animation.frame_offset = read_bitu32(src);
	animation.frame_rate = read_bitu8(src);
	animation.frame_size = read_bitu8(src);
	animation.state_id = read_bitu16(src);

	animation.unknown = read_bit16(src);
	animation.speed = read_bit16(src);
	animation.accel_lo = read_bit16(src);
	animation.accel_hi = read_bit16(src);

	animation.unknown2 = read_bit16(src);
	animation.speed2 = read_bit16(src);
	animation.accel_lo2 = read_bit16(src);
	animation.accel_hi2 = read_bit16(src);

	animation.frame_start = read_bitu16(src);
	animation.frame_end = read_bitu16(src);
	animation.next_animation = read_bitu16(src);
	animation.next_frame = read_bitu16(src);

	animation.num_state_changes = read_bitu16(src);
	animation.state_change_offset = read_bitu16(src);
	animation.num_anim_commands = read_bitu16(src);
	animation.anim_command = read_bitu16(src);
}

void TR_Level::read_tr4_level(SDL_RWops * const _src)
{
	SDL_RWops *src = _src;
	uint32_t i;
	uint8_t *uncomp_buffer = NULL;
	uint8_t *comp_buffer = NULL;
	SDL_RWops *newsrc = NULL;

        // Version
        uint32_t file_version = read_bitu32(src);

        if (file_version != 0x00345254 /*&& file_version != 0x63345254*/)           // +TRLE
                Sys_extError("Wrong level version");

        this->num_textiles = 0;
        this->num_room_textiles = 0;
        this->num_obj_textiles = 0;
        this->num_bump_textiles = 0;
        this->num_misc_textiles = 0;
        this->read_32bit_textiles = false;

        {
                uint32_t uncomp_size;
                uint32_t comp_size;
                unsigned long size;

                this->num_room_textiles = read_bitu16(src);
                this->num_obj_textiles = read_bitu16(src);
                this->num_bump_textiles = read_bitu16(src);
                this->num_misc_textiles = 2;
                this->num_textiles = this->num_room_textiles + this->num_obj_textiles + this->num_bump_textiles + this->num_misc_textiles;

                uncomp_size = read_bitu32(src);
                if (uncomp_size == 0)
                        Sys_extError("read_tr4_level: textiles32 uncomp_size == 0");

                comp_size = read_bitu32(src);
                if (comp_size > 0) {
                        uncomp_buffer = new uint8_t[uncomp_size];
                        
                        this->textile32_count = this->num_textiles;
                        this->textile32 = (tr4_textile32_t*)malloc(this->textile32_count * sizeof(tr4_textile32_t));
                        comp_buffer = new uint8_t[comp_size];

                        if (SDL_RWread(src, comp_buffer, 1, comp_size) < (int)comp_size)
                                Sys_extError("read_tr4_level: textiles32");

                        size = uncomp_size;
                        if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                                Sys_extError("read_tr4_level: uncompress");

                        if (size != uncomp_size)
                                Sys_extError("read_tr4_level: uncompress size mismatch");
                        delete [] comp_buffer;

                        comp_buffer = NULL;
                        if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                                Sys_extError("read_tr4_level: SDL_RWFromMem");

                        for (i = 0; i < (this->num_textiles - this->num_misc_textiles); i++)
                                read_tr4_textile32(newsrc, this->textile32[i]);
                        SDL_RWclose(newsrc);
                        newsrc = NULL;
                        delete [] uncomp_buffer;

                        uncomp_buffer = NULL;
                        this->read_32bit_textiles = true;
                }

                uncomp_size = read_bitu32(src);
                if (uncomp_size == 0)
                        Sys_extError("read_tr4_level: textiles16 uncomp_size == 0");

                comp_size = read_bitu32(src);
                if (comp_size > 0) {
                        if (this->textile32_count == 0) {
                                uncomp_buffer = new uint8_t[uncomp_size];

                                this->textile16_count = this->num_textiles;
                                this->textile16 = (tr2_textile16_t*)malloc(this->textile16_count * sizeof(tr2_textile16_t));
                                comp_buffer = new uint8_t[comp_size];

                                if (SDL_RWread(src, comp_buffer, 1, comp_size) < (int)comp_size)
                                        Sys_extError("read_tr4_level: textiles16");

                                size = uncomp_size;
                                if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                                        Sys_extError("read_tr4_level: uncompress");

                                if (size != uncomp_size)
                                        Sys_extError("read_tr4_level: uncompress size mismatch");
                                delete [] comp_buffer;

                                comp_buffer = NULL;
                                if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                                        Sys_extError("read_tr4_level: SDL_RWFromMem");

                                for (i = 0; i < (this->num_textiles - this->num_misc_textiles); i++)
                                        read_tr2_textile16(newsrc, this->textile16[i]);
                                SDL_RWclose(newsrc);
                                newsrc = NULL;
                                delete [] uncomp_buffer;

                                uncomp_buffer = NULL;
                        } else {
                                SDL_RWseek(src, comp_size, SEEK_CUR);
                        }
                }

                uncomp_size = read_bitu32(src);
                if (uncomp_size == 0)
                        Sys_extError("read_tr4_level: textiles32d uncomp_size == 0");

                comp_size = read_bitu32(src);
                if (comp_size > 0) {
                        uncomp_buffer = new uint8_t[uncomp_size];

                        if ((uncomp_size / (256 * 256 * 4)) > 2)
                                Sys_extWarn("read_tr4_level: num_misc_textiles > 2");

                        if (this->textile32_count == 0)
                        {
                                this->textile32_count = this->num_textiles;
                                this->textile32 = (tr4_textile32_t*)malloc(this->textile32_count * sizeof(tr4_textile32_t));
                        }
                        comp_buffer = new uint8_t[comp_size];

                        if (SDL_RWread(src, comp_buffer, 1, comp_size) < (int)comp_size)
                                Sys_extError("read_tr4_level: misc_textiles");

                        size = uncomp_size;
                        if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                                Sys_extError("read_tr4_level: uncompress");

                        if (size != uncomp_size)
                                Sys_extError("read_tr4_level: uncompress size mismatch");
                        delete [] comp_buffer;

                        comp_buffer = NULL;
                        if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                                Sys_extError("read_tr4_level: SDL_RWFromMem");

                        for (i = (this->num_textiles - this->num_misc_textiles); i < this->num_textiles; i++)
                                read_tr4_textile32(newsrc, this->textile32[i]);
                        SDL_RWclose(newsrc);
                        newsrc = NULL;
                        delete [] uncomp_buffer;

                        uncomp_buffer = NULL;
                }

                uncomp_size = read_bitu32(src);
                comp_size = read_bitu32(src);
                if (!comp_size)
                        Sys_extError("read_tr4_level: packed geometry");

                uncomp_buffer = new uint8_t[uncomp_size];
                comp_buffer = new uint8_t[comp_size];

                if (SDL_RWread(src, comp_buffer, 1, comp_size) < (int)comp_size)
                        Sys_extError("read_tr4_level: packed geometry");

                size = uncomp_size;
                if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                        Sys_extError("read_tr4_level: uncompress");

                if (size != uncomp_size)
                        Sys_extError("read_tr4_level: uncompress size mismatch");
                delete [] comp_buffer;

                comp_buffer = NULL;
                if ((src = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                        Sys_extError("read_tr4_level: SDL_RWFromMem");
        }

        // Unused
        if (read_bitu32(src) != 0)
                Sys_extWarn("Bad value for 'unused'");

        this->rooms_count = read_bitu16(src);
	this->rooms = (tr5_room_t*)calloc(this->rooms_count, sizeof(tr5_room_t));
	for (i = 0; i < this->rooms_count; i++)
                read_tr4_room(src, this->rooms[i]);

	this->floor_data_size = read_bitu32(src); 
        this->floor_data = (uint16_t*)malloc(this->floor_data_size * sizeof(uint16_t));
        for(i = 0; i < this->floor_data_size; i++)
                this->floor_data[i] = read_bitu16(src); 

        read_mesh_data(src);

        this->animations_count = read_bitu32(src);
	this->animations = (tr_animation_t*)malloc(this->animations_count * sizeof(tr_animation_t));
	for (i = 0; i < this->animations_count; i++)
                read_tr4_animation(src, this->animations[i]);

        this->state_changes_count = read_bitu32(src);
	this->state_changes = (tr_state_change_t*)malloc(this->state_changes_count * sizeof(tr_state_change_t));
        for (i = 0; i < this->state_changes_count; i++)
                read_tr_state_changes(src, this->state_changes[i]);

        this->anim_dispatches_count = read_bitu32(src);
	this->anim_dispatches = (tr_anim_dispatch_t*)malloc(this->anim_dispatches_count * sizeof(tr_anim_dispatch_t));
        for (i = 0; i < this->anim_dispatches_count; i++)
                read_tr_anim_dispatches(src, this->anim_dispatches[i]);
	//SDL_RWseek(src, this->anim_dispatches.size() * 8, SEEK_CUR);

        this->anim_commands_count = read_bitu32(src);
	this->anim_commands = (int16_t*)malloc(this->anim_commands_count * sizeof(int16_t));
        for (i = 0; i < this->anim_commands_count; i++)
                this->anim_commands[i] = read_bit16(src);
        //SDL_RWseek(src, this->anim_commands.size() * 2, SEEK_CUR);

        this->mesh_tree_data_size = read_bitu32(src);
        this->mesh_tree_data = (uint32_t*)malloc(this->mesh_tree_data_size * sizeof(uint32_t));
        for (i = 0; i < this->mesh_tree_data_size; i++)
                this->mesh_tree_data[i] = read_bitu32(src);                     // 4 bytes
        
        read_frame_moveable_data(src);

        this->static_meshes_count = read_bitu32(src);
	this->static_meshes = (tr_staticmesh_t*)malloc(this->static_meshes_count * sizeof(tr_staticmesh_t));
	for (i = 0; i < this->static_meshes_count; i++)
                read_tr_staticmesh(src, this->static_meshes[i]);

        if (read_bit8(src) != 'S')
                Sys_extError("read_tr4_level: 'SPR' not found");

        if (read_bit8(src) != 'P')
                Sys_extError("read_tr4_level: 'SPR' not found");

        if (read_bit8(src) != 'R')
                Sys_extError("read_tr4_level: 'SPR' not found");

        this->sprite_textures_count = read_bitu32(src);
	this->sprite_textures = (tr_sprite_texture_t*)malloc(this->sprite_textures_count * sizeof(tr_sprite_texture_t));
	for (i = 0; i < this->sprite_textures_count; i++)
		read_tr4_sprite_texture(src, this->sprite_textures[i]);

        this->sprite_sequences_count = read_bitu32(src);
	this->sprite_sequences = (tr_sprite_sequence_t*)malloc(this->sprite_sequences_count * sizeof(tr_sprite_sequence_t));
	for (i = 0; i < this->sprite_sequences_count; i++)
		read_tr_sprite_sequence(src, this->sprite_sequences[i]);

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
	//SDL_RWseek(src, this->cameras.size() * 16, SEEK_CUR);

        this->flyby_cameras_count = read_bitu32(src);
	this->flyby_cameras = (tr4_flyby_camera_t*)malloc(this->flyby_cameras_count * sizeof(tr4_flyby_camera_t));
        for (i = 0; i < this->flyby_cameras_count; i++)
        {
                this->flyby_cameras[i].x1 = read_bit32(src);
                this->flyby_cameras[i].y1 = read_bit32(src);
                this->flyby_cameras[i].z1 = read_bit32(src);
                this->flyby_cameras[i].x2 = read_bit32(src);
                this->flyby_cameras[i].y2 = read_bit32(src);
                this->flyby_cameras[i].z2 = read_bit32(src);                    // 24
                
                this->flyby_cameras[i].index1 = read_bit8(src);
                this->flyby_cameras[i].index2 = read_bit8(src);                 // 26
                
                this->flyby_cameras[i].unknown[0] = read_bitu16(src);
                this->flyby_cameras[i].unknown[1] = read_bitu16(src);
                this->flyby_cameras[i].unknown[2] = read_bitu16(src);
                this->flyby_cameras[i].unknown[3] = read_bitu16(src);
                this->flyby_cameras[i].unknown[4] = read_bitu16(src);           // 36
                
                this->flyby_cameras[i].id = read_bit32(src);                    // 40
        }
        //SDL_RWseek(src, this->flyby_cameras.size() * 40, SEEK_CUR);

        this->sound_sources_count = read_bitu32(src);
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
		read_tr2_box(src, this->boxes[i]);

        this->overlaps_count = read_bitu32(src);
	this->overlaps = (uint16_t*)malloc(this->overlaps_count * sizeof(uint16_t));
        for (i = 0; i < this->overlaps_count; i++)
                this->overlaps[i] = read_bitu16(src);

        // Zones
        SDL_RWseek(src, this->boxes_count * 20, SEEK_CUR);

        this->animated_textures_count = read_bitu32(src);
	this->animated_textures = (tr_animated_textures_t*)malloc(this->animated_textures_count * sizeof(tr_animated_textures_t));
        for (i = 0; i < this->animated_textures_count; i++)
        {
                this->animated_textures[i].texture_ids = NULL;                  // FIXME: complete this section!
                this->animated_textures[i].texture_ids_count = read_bit16(src); 
        }
	//SDL_RWseek(src, this->animated_textures.size() * 2, SEEK_CUR);

        int unknown = read_bit8(src);

        if ((unknown != 0) && (unknown != 1) && (unknown != 2) && (unknown != 4) && (unknown != 12))
                Sys_extError("read_tr4_level: unknown before TEX has bad value");

        if (read_bit8(src) != 'T')
                Sys_extError("read_tr4_level: '\\0TEX' not found");

        if (read_bit8(src) != 'E')
                Sys_extError("read_tr4_level: '\\0TEX' not found");

        if (read_bit8(src) != 'X')
                Sys_extError("read_tr4_level: '\\0TEX' not found");
    
        this->object_textures_count = read_bitu32(src);
	this->object_textures = (tr4_object_texture_t*)malloc(this->object_textures_count * sizeof(tr4_object_texture_t));
	for (i = 0; i < this->object_textures_count; i++)
		read_tr4_object_texture(src, this->object_textures[i]);

        this->items_count = read_bitu32(src);
	this->items = (tr2_item_t*)malloc(this->items_count * sizeof(tr2_item_t));
	for (i = 0; i < this->items_count; i++)
                read_tr3_item(src, this->items[i]);

        this->ai_objects_count = read_bitu32(src);
	this->ai_objects = (tr4_ai_object_t*)malloc(this->ai_objects_count * sizeof(tr4_ai_object_t));
        for(i=0; i < this->ai_objects_count; i++)
        {
            this->ai_objects[i].object_id = read_bitu16(src);
            this->ai_objects[i].room = read_bitu16(src);                        // 4
            
            this->ai_objects[i].x = read_bit32(src);
            this->ai_objects[i].y = read_bit32(src);
            this->ai_objects[i].z = read_bit32(src);                            // 16
            
            this->ai_objects[i].ocb = read_bitu16(src);
            this->ai_objects[i].flags = read_bitu16(src);                       // 20
            this->ai_objects[i].angle = read_bit32(src);                        // 24
        }
	//SDL_RWseek(src, this->ai_objects.size() * 24, SEEK_CUR);

        this->demo_data_count = read_bitu16(src);
	this->demo_data = (uint8_t*)malloc(this->demo_data_count * sizeof(uint8_t));
        for(i=0; i < this->demo_data_count; i++)
                this->demo_data[i] = read_bitu8(src);
	//SDL_RWseek(src, this->demo_data.size(), SEEK_CUR);

        // Soundmap
        SDL_RWseek(src, 2 * 370, SEEK_CUR);

        this->sound_details_count = 0;
        i = read_bitu32(src);
        if(i != 0xFFFFFFFF)
        {
            this->sound_details_count = i;
            this->sound_details = (tr_sound_details_t*)malloc(this->sound_details_count * sizeof(tr_sound_details_t));
            for(i=0; i < this->sound_details_count; i++)
            {
                    this->sound_details[i].sample = read_bit16(src);
                    this->sound_details[i].volume = read_bit16(src);
                    this->sound_details[i].sound_range = read_bit16(src);
                    this->sound_details[i].flags = read_bit16(src);
            }
            //SDL_RWseek(src, this->sound_details.size() * 8, SEEK_CUR);
        }
        
        i = read_bitu32(src);
        if(i != 0xFFFFFFFF)
        {
            this->sample_indices_count = read_bitu32(src);
            this->sample_indices = (uint32_t*)malloc(this->sample_indices_count * sizeof(uint32_t));
            for(i=0; i < this->sample_indices_count; i++)
                    this->sample_indices[i] = read_bitu32(src);
            //SDL_RWseek(src, this->sample_indices.size() * 4, SEEK_CUR);
        }
        else
        {
            this->sample_indices_count = 0;
            this->sample_indices = NULL;
        }
        
        uint16_t temp;

        temp = read_bitu16(src);
        //if ((temp != 0) && (temp != 0xCDCD) && (temp != 0xFFFF) && (temp != 0x0178))
        //        Sys_extWarn("read_tr4_level: filler1 has wrong value");

        temp = read_bitu16(src);
        if ((temp != 0) && (temp != 0xCDCD) && (temp != 0xFFFF) && (temp != 0x0178))
                Sys_extWarn("read_tr4_level: filler2 has wrong value");

        temp = read_bitu16(src);
        //if ((temp != 0) && (temp != 0xCDCD) && (temp != 0xFFFF) && (temp != 0x0178))
        //        Sys_extWarn("read_tr4_level: filler3 has wrong value");

        SDL_RWclose(src);
        delete [] uncomp_buffer;

        uncomp_buffer = NULL;
}
