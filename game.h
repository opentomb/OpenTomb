
#ifndef GAME_H
#define GAME_H

#include <stdint.h>

class VT_Level;
struct polygon_s;
struct base_mesh_s;
struct room_s;
struct world_s;
struct camera_s;
struct entity_s;
struct room_sector_s;

extern btScalar cam_angles[3];


void Game_InitGlobals();
int Game_Load(const char* name);
int Game_Save(const char* name);


void Game_ApplyControls();
void GameFrame(btScalar time);

void Cam_FollowEntity(struct camera_s *cam, struct entity_s *ent, btScalar dx, btScalar dz);

#endif

