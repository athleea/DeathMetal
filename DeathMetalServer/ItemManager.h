#pragma once
#include "stdafx.h"
#include "Item.h"

struct MyHash {
	size_t operator()(const vector3& vec3) const {
		std::hash<float> hash_func; 

		return hash_func(vec3.x + vec3.y + vec3.z); 
	}
};


class ItemManager {
public:
	ItemManager(class Timer* mgr);
	Item* SpawnNewItem(bool);
	void DestroyItem(short item_id);
	void SpawnAllItem();
public:
	
	std::vector<vector3> item_spawn_locations;
	std::unordered_set<vector3, MyHash> cur_spawned_locations;
	concurrency::concurrent_unordered_map<short, Item*> item_map;
	std::atomic_short item_index;
	class Timer* timer_mgr;

};