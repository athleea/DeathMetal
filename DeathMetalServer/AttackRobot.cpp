#include "AttackRobot.h"

AttackRobot::AttackRobot(short id) : NPC(id)
{
	target_id = -1;
	attack_power = 10;
	hp = 30;
}

void AttackRobot::Respawn()
{
	target_id = -1;
	hp = 30;
}