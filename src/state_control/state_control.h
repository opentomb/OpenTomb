
#ifndef STATE_CONTROL_H
#define STATE_CONTROL_H

struct ss_animation_s;
struct entity_s;

int StateControl_Lara(struct entity_s *ent, struct ss_animation_s *ss_anim);
void StateControl_LaraSetIdleAnim(struct entity_s *ent, int anim_type, int move_type);

#endif

