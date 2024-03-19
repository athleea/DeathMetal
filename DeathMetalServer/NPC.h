#pragma once
#include "Object.h"

enum class ANIM : u_char {
	Idle,
	Walk,
	Run,
	Dead,
	Attack,
	Defense
};

class NPC : public Object {
public:
	int hp;
	int attack_power;
	vector3 velocity;
	std::atomic<ANIM> cur_anim;
	vector3 home_location;

	NPC();
	NPC(int id);

	virtual void Respawn();
};