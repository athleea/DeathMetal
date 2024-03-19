#pragma once
#include "stdafx.h"


struct TRANSFORM {
	vector3 location;
	vector3 rotation;
};

constexpr short FLEE_ROBOT_HP = 20;
constexpr short ATTACK_ROBOT_HP = 30;

const TRANSFORM SPAWN_SPOT[4] = {
	{ { 80.0f, -5390.0f, 108.f }, { 0.f, 0.f, 90.f } },
	{ { 2230.0f, 230.0f, 108.f }, { 0.f, 0.f, 180.f } },
	{ { 5830.0f, 380.0f, 108.f }, { 0.f, 0.f, 180.f } },
	{ { 6350.f, -6770.f, 190.f }, { 0.f, 0.f, 140.0f } }
};

struct BaseRobotMonster {
	std::mutex	mLock;
	short			mID;
	char			mState;
	char			mType;
	std::atomic<int> mHP;
	vector3		mLocation;
	vector3		mRotation;
	vector3		mVelocity;
	
	void Init(int id, char type, int hp, vector3 location, vector3 rotation);
	void ApplyDamage(int damage);
	void Respawn();

	void SetRobotID(int id);
	int GetRobotID() const;

	void SetRobotType(char type);
	char GetRobotType() const;

	void SetHP(int hp);
	int GetHP() const;

	vector3 GetLocation() const;
};