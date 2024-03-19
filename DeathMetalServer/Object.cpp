#include "Object.h"

Object::Object()
{
	id = -1;
	location = { 0.0f,0.0f,0.0f };
	rotation = { 0.0f,0.0f,0.0f };
	cur_room_id = -1;
}

Object::Object(short new_id)
{
	id = new_id;
	location = { 0.0f,0.0f,0.0f };
	rotation = { 0.0f,0.0f,0.0f };
	cur_room_id = -1;
}
