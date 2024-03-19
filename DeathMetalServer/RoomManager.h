#pragma once
#include "Room.h"

class RoomManager {
public:
	RoomManager();

	void AddPlayer(Player* player);
	bool MakeRoom(class Timer* mgr);
	bool CheckPlayersReadyState(short room_id);

	std::atomic_short room_index;
	concurrency::concurrent_unordered_map<u_short, Room*> rooms;
	concurrency::concurrent_vector<Player*> waiting_players;
};