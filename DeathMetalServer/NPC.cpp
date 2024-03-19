#include "NPC.h"

NPC::NPC() : Object()
{
	hp = 100;
	attack_power = 0;
	velocity = home_location = vector3{ 0,0,0 };
	cur_anim = ANIM::Idle;
}

NPC::NPC(int id) : Object(id)
{
	hp = 100;
	attack_power = 0;
	velocity = home_location = vector3{ 0,0,0 };
	cur_anim = ANIM::Idle;
}

void NPC::Respawn()
{
	hp = 100;
	cur_anim = ANIM::Idle;
}
