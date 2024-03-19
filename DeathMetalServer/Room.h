#pragma once
#include "stdafx.h"
#include "Player.h"

class Room {
public:
	std::atomic_bool is_start;
	short players_num;
	short id;
	std::vector<Player*> players;
	std::vector<NPC*> monsters;
	std::vector<Object*> objects;
	std::chrono::steady_clock::time_point start_time;

	class Timer* timer_mgr;
	class ItemManager* item_mgr;
	class MonsterManager* monster_mgr;

	Room(short r_id, Timer* mgr);
	~Room();

	void ReadyGame();
	void StartGame();
	void GameOver();
};