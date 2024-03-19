#include "Room.h"
#include "Timer.h"
#include "ItemManager.h"
#include "MonsterManager.h"

Room::Room(short r_id, Timer* mgr)
{
	is_start = false;
	players_num = 0;
	id = r_id;
	players.reserve(MAX_ROOM_PLAYER);
	monsters.reserve(MAX_ROOM_PLAYER);
	objects.reserve(15);

	timer_mgr = mgr;
	item_mgr = new ItemManager{ timer_mgr };
	monster_mgr = new MonsterManager{ timer_mgr };
}

Room::~Room()
{
	delete item_mgr;
	delete monster_mgr;
}
void Room::ReadyGame()
{
	std::array<std::string, MAX_ROOM_PLAYER> names;

	for (int i = 0; i < MAX_ROOM_PLAYER; ++i) {
		players[i]->id = i;
		players[i]->cur_room_id = id;
		names[i] = players[i]->name;
	}


	for (auto pl : players) {
		if (pl == nullptr) continue;
		pl->InitializeStat();
		pl->SendGameReadyPacket(names);
		++players_num;
	}
	std::cout << std::format("room is ready\n");
}

void Room::StartGame()
{
	bool start_value = false;
	if (std::atomic_compare_exchange_strong(&is_start, &start_value, true) == true) {
		for (auto pl : players) {
			if (pl == nullptr) continue;
			pl->SendGameStartPacket();
		}

		start_time = std::chrono::steady_clock::now() + std::chrono::seconds(3);

		TimerEvent t_event{ };
		t_event.obj_id = id;
		t_event.type = OP_TYPE::GAME_TIME;
		t_event.exec_time = start_time;
		timer_mgr->PushEvent(t_event);

		t_event.type = OP_TYPE::ITEM_SPAWN;
		t_event.exec_time += std::chrono::seconds(5);
		timer_mgr->PushEvent(t_event);
	}
	
}

void Room::GameOver()
{
	short kills[MAX_ROOM_PLAYER]{};
	short deaths[MAX_ROOM_PLAYER]{};

	for (Player* player : players) {
		kills[player->id] = player->kill_count;
		deaths[player->id] = player->death_count;
	}

	for (Player* player : players) {
		player->SendGameOverPacket(kills);
		player->net_state = NET_STATE::LOBBY;
	}
}
