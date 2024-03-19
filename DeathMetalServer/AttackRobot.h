#pragma once
#include "stdafx.h"
#include "NPC.h"

class AttackRobot : public NPC {
public:
	AttackRobot(short id);
	void Respawn();
public:
	short target_id;
};