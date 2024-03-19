#pragma once
#include "stdafx.h"
#include "Object.h"

class Item : public Object {
public:
	Item(const short , const vector3 );
public:
	EItem type;
};
