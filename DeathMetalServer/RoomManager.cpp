#include "RoomManager.h"
#include "Timer.h"

RoomManager::RoomManager()
{
	room_index = 0;
	waiting_players.reserve(MAX_CLIENT);
}

void RoomManager::AddPlayer(Player* player)
{
	waiting_players.push_back(player);
}

bool RoomManager::MakeRoom(Timer* mgr)
{
	int count = 0;
	std::vector<Player*> temp_players{};

	for (int i = 0; i < waiting_players.size(); ++i) {
		if (waiting_players[i] == nullptr) continue;
		if (waiting_players[i]->net_state != NET_STATE::MATCHING) continue;

		waiting_players[i]->net_state = NET_STATE::INGAME;
		temp_players.push_back(waiting_players[i]);
		waiting_players[i] = nullptr;
		count++;
		if (count == MAX_ROOM_PLAYER) break;
	}

	if (count < MAX_ROOM_PLAYER) {
		for (auto cl : temp_players) {
			cl->net_state = NET_STATE::MATCHING;
			waiting_players.push_back(cl);
		}
		return false;
	}

	Room* temp_room = new Room{ room_index, mgr };
	temp_room->players = temp_players;
	rooms[room_index] = temp_room;
	temp_room->ReadyGame();

	++room_index;

	return true;
}

bool RoomManager::CheckPlayersReadyState(short room_id)
{
	for (auto player : rooms[room_id]->players) {
		if (player == nullptr) continue;
		if (player->ready_flag == false) {
			return false;
		}
	}
	return true;
}
