#include "BaseRobotMonster.h"

void BaseRobotMonster::Init(int id, char type, int hp, vector3 location, vector3 rotation)
{
	mID = id;
	mType = type;
	mHP = hp;
	mState = STATE_IDLE;
	mLocation = location;
	mRotation = rotation;
	mVelocity = {};
}

void BaseRobotMonster::ApplyDamage(int damage)
{
	mHP -= damage;
}

void BaseRobotMonster::Respawn()
{
	mLocation = SPAWN_SPOT[mID].location;
	mRotation = SPAWN_SPOT[mID].rotation;

	if (mType == TYPE_FLEE_ROBOT) {
		mHP = FLEE_ROBOT_HP;
	}
	else {
		mHP = ATTACK_ROBOT_HP;
	}
}

void BaseRobotMonster::SetRobotID(int id)
{
	mID = id;
}

int BaseRobotMonster::GetRobotID() const
{
	return mID;
}

void BaseRobotMonster::SetRobotType(char type)
{
	mType = type;
}

char BaseRobotMonster::GetRobotType() const
{
	return mType;
}

void BaseRobotMonster::SetHP(int hp)
{
	mHP = hp;
}

int BaseRobotMonster::GetHP() const
{
	return mHP;
}

vector3 BaseRobotMonster::GetLocation() const
{
	return mLocation;
}
