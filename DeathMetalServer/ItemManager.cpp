#include <random>
#include "ItemManager.h"
#include "Timer.h"

std::random_device location_rd;
std::mt19937 location_gen(location_rd());

ItemManager::ItemManager(Timer* mgr)
{
	item_index = MAX_ROOM_PLAYER + MAX_FLEE_ROBOT + MAX_ATTACK_ROBOT;
	timer_mgr = mgr;

	std::ifstream in{ "item_pos.txt" };
	if (!in) return;
	double x, y, z;

	while (in >> x) {
		in >> y;
		in >> z;
		item_spawn_locations.emplace_back(x, y, z);
	};

	

}

Item* ItemManager::SpawnNewItem(bool is_monster)
{
	//std::shuffle(item_spawn_locations.begin(), item_spawn_locations.end(), location_gen);
	if (is_monster) {
		const short item_id = item_index;
		++item_index;

		Item* item = new Item{ item_id, {} };
		item_map[item_id] = item;

		return item;
	}
	int index = item_spawn_locations.size();
	bool find_location = false;
	while (index > 0) {
		if (cur_spawned_locations.contains(item_spawn_locations[index-1])) {
			--index;
			continue;
		}
		else {
			find_location = true;
			break;
		}
	}
	
	if (find_location) {
		const short item_id = item_index;
		++item_index;

		Item* item = new Item{ item_id, item_spawn_locations[index-1]};
		item_map[item_id] = item;
		cur_spawned_locations.insert(item->location);

		return item;
	}
	else {
		std::cout << std::format("Item spawn spot is full\n");
		return nullptr;
	}
}

void ItemManager::DestroyItem(short item_id)
{
	cur_spawned_locations.erase(item_map[item_id]->location);
	item_map.unsafe_erase(item_id);
}

void ItemManager::SpawnAllItem()
{
}

