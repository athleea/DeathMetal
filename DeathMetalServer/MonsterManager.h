#pragma once
#include "stdafx.h"
#include "AttackRobot.h"
#include "FleeRobot.h"

class MonsterManager {
public:
	MonsterManager(class Timer* mgr);
	~MonsterManager();
	NPC* SpawnNewMonster(const bool);
	void SetDeadMonster(const short id);
	void RespawnMonster(const short id);
public:
	std::atomic_short monster_index;
	class Timer* timer_mgr;
	concurrency::concurrent_unordered_map<short, NPC*> monster_map;
	std::vector<std::pair<vector3, vector3>> robot_positions;
	std::vector<vector3> flee_robot_locations;
};