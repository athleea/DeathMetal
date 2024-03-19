#include "MonsterManager.h"
#include "Timer.h"


MonsterManager::MonsterManager(Timer* mgr)
{
	timer_mgr = mgr;
	monster_index = 0;

	for (short i = 0; i < MAX_ATTACK_ROBOT; ++i) {
		short id = MAX_ROOM_PLAYER + monster_index;
		monster_map[id] = new AttackRobot{id};
		std::cout << std::format("{} monster spawn\n", id);

		++monster_index;
	}
	
	for (short i = 0; i < MAX_FLEE_ROBOT; ++i) {
		short id = MAX_ROOM_PLAYER+ monster_index;
		monster_map[id] = new FleeRobot{ id };
		
		std::cout << std::format("{} monster spawn\n", id);
		++monster_index;
	}
}

MonsterManager::~MonsterManager()
{
	for (auto [key, value] : monster_map) {
		delete value;
	}
}

NPC* MonsterManager::SpawnNewMonster(const bool is_flee_robot)
{
	if (is_flee_robot == true) {
		short id = MAX_ROOM_PLAYER + monster_index++;
		monster_map[id] = new AttackRobot{ id };

		return monster_map[id];
	}
	else {
		short id = MAX_ROOM_PLAYER + MAX_ATTACK_ROBOT + monster_index++;
		monster_map[id] = new FleeRobot{ id };

		return monster_map[id];
	}
}

void MonsterManager::SetDeadMonster(const short id)
{
	monster_map[id]->cur_anim = ANIM::Dead;
	monster_map[id]->hp = 0;
	monster_map[id]->velocity = vector3{ 0, 0, 0 };
}

void MonsterManager::RespawnMonster(const short id)
{
	monster_map[id]->cur_anim = ANIM::Idle;
	monster_map[id]->hp = 100;
}


