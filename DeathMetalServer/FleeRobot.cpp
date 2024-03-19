#include "FleeRobot.h"
#include <random>

std::default_random_engine dre;
std::uniform_int_distribution uid(0, 2);

FleeRobot::FleeRobot(short id) : NPC(id)
{
	hp = 10;
	flee_point = {};
	home_location = vector3(-275, 2930, 2495);
	cur_spot_index = 2;
}

void FleeRobot::DoFlee()
{
	vector3 spot[3] = {
		{-2522, -2906, 2484},
		{1956, -2990, 2484},
		{-275, 2930, 2495}
	};

	short spot_index = uid(dre);
	while (true) {
		if (cur_spot_index != spot_index) break;
		spot_index = uid(dre);
	}
	
	flee_point = spot[spot_index];
}

void FleeRobot::Respawn()
{
	hp = 10;
	cur_spot_index = 2;
}
