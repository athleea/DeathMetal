#include "Item.h"
#include <random>

std::random_device item_rd;
std::mt19937 item_gen(item_rd());
std::uniform_int_distribution<int> item_dis(0, ITEM_COUNT-1);

Item::Item(short i, const vector3 loc) : Object(i)
{
	type = static_cast<EItem>(item_dis(item_gen));
	location = loc;
}

