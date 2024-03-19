#pragma once
#include "NPC.h"

class FleeRobot : public NPC {

public:
	FleeRobot(short id);
	
	void DoFlee();
	void Respawn();

	vector3 flee_point;
	std::atomic_short cur_spot_index;

};