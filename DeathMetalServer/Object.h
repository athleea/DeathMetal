#pragma once
#include "stdafx.h"

class Object {
public:
	int id;
	vector3 location;
	vector3 rotation;
	int cur_room_id;

	Object();
	Object(short id);
};