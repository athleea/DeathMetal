#include <random>
#include "Player.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 7);

Player::Player(const u_short key, const SOCKET s) : NPC()
{
	recv_buf.reserve(MAX_BUF_SIZE);
	io_key = key;
	socket = s;
	ready_flag = false;

	kill_count = 0;
	death_count = 0;
	shield_flag = false;
	cur_weapon = EItem::DEFAULT;
}

Player::~Player()
{
	closesocket(socket);
}

void Player::DoSend(void* p)
{
	OVER_EXP* send_data = new OVER_EXP{ reinterpret_cast<char*>(p) }; //생성자에서 패킷복사
	WSASend(socket, &send_data->wsabuf, 1, 0, 0, &send_data->overlapped, 0);
}

void Player::DoRecv()
{
	ZeroMemory(&ex_over.overlapped, sizeof ex_over.overlapped );
	DWORD flags = 0;
	ex_over.wsabuf.buf = ex_over.io_buf;
	ex_over.wsabuf.len = MAX_BUF_SIZE - recv_buf.size();
	WSARecv(socket, &ex_over.wsabuf, 1, NULL, &flags, &ex_over.overlapped, NULL);
}

void Player::SendLoginOkPacket(const char* name)
{
	SC_PACKET_LOGIN_OK p;
	p.type = S2C_LOGIN_OK;
	strcpy_s(p.name, name);
	p.size = sizeof p.type + strlen(name);

	DoSend(&p);
}

void Player::SendLoginFailPacket()
{
	SC_PACKET_FAIL_LOGIN p;
	p.size = sizeof(p);
	p.type = S2C_LOGIN_FAIL;

	DoSend(&p);
}

void Player::SendLeavePacket(const int other_id)
{
	SC_PACKET_LEAVE p;
	p.id = other_id;
	p.size = sizeof(p);
	p.type = S2C_LEAVE;

	DoSend(&p);
}

void Player::SendGameStartPacket()
{
	SC_PACKET_LEAVE p;
	p.size = sizeof(p);
	p.type = S2C_GAME_START;

	DoSend(&p);

}


void Player::SendGameReadyPacket(const std::array<std::string, MAX_ROOM_PLAYER>& other_names)
{
	SC_PACKET_GAME_START p;
	p.SIZE = sizeof(p);
	p.TYPE = S2C_GAME_READY;
	p.ID = id;
	for (int i = 0; i < MAX_ROOM_PLAYER; ++i)
		strcpy_s(p.NAMES[i], other_names[i].c_str());

	DoSend(&p);
	
}

void Player::SendGameOverPacket(const short kills[])
{
	SC_PACKET_GAME_OVER p;

	p.SIZE = sizeof(p);
	p.TYPE = S2C_GAME_RESULT;
	CopyMemory(p.KILL_RESULT, kills, sizeof(short) * MAX_ROOM_PLAYER);

	DoSend(&p);
}

void Player::SendTimePacket(float game_time)
{
	SC_PACKET_TIMER p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_TIMER;
	p.TIME = game_time;

	DoSend(&p);
}

void Player::SendMovePacket(const NPC* obj)
{
	SC_PACKET_MOVE p;
	p.SIZE = sizeof(p);
	p.TYPE = S2C_MOVE;

	p.ID = obj->id;
	p.LOCATION = obj->location;
	p.ROTATION = obj->rotation;
	p.VELOCITY = obj->velocity;

	DoSend(&p);
}

void Player::SendAnimPacket(const short o_id, const char anim_type)
{
	SC_PACKET_ANIM p;
	p.SIZE = sizeof(p);
	p.TYPE = S2C_ANIM;
	p.ID = o_id;
	p.ANIM_TYPE = anim_type;

	DoSend(&p);
}

void Player::SendHitPacket(const short attacker_id, const short damaged_id, const short hp, const char attack_type, const int bone_index)
{
	SC_PACKET_HIT p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_HIT;
	p.ATTKER_ID = attacker_id;
	p.HP = hp;
	p.DAMAGED_ID = damaged_id;
	p.ATTACK_TYPE = attack_type;
	p.BONE_INDEX = bone_index;

	DoSend(&p);
}

void Player::SendSpawnObjectPacket(const Object* obj)
{
	SC_PACKET_SPAWN p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_SPAWN;
	p.OBJ_ID = obj->id;
	p.LOCATION = obj->location;
	p.ROTATION = obj->rotation;

	DoSend(&p);
}

void Player::SendSpawnObjectPacket(const Item* obj)
{
	SC_PACKET_SPAWN p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_SPAWN;
	p.OBJ_ID = obj->id;
	p.LOCATION = obj->location;
	p.ROTATION = obj->rotation;
	p.ITEM_TYPE = obj->type;

	DoSend(&p);
}

void Player::SendChangeStatAndDestroyItem(const Player* pl, const short item_id)
{
	SC_PACKET_CHANGE_STAT_REMOVE_ITEM p;
	p.TYPE = S2C_CHANGE_STAT_REMOVE_ITEM;
	p.SIZE = sizeof p;
	p.ITEM_ID = item_id;
	p.PLAYER_ID = pl->id;
	p.HP = pl->hp;
	p.WEAPON = pl->cur_weapon;

	DoSend(&p);
}

void Player::SendKillMsgPacket(const short attacker, const short victim)
{
	SC_PACKET_KILL p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_KILL;
	p.KILLER = attacker;
	p.VICTIM = victim;

	DoSend(&p);
}

void Player::SendRobotFleePacket(const short robot_id, const vector3 flee_location)
{
	SC_PACKET_ROBOT_FLEE p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_ROBOT_FLEE;
	p.ROBOT_ID = robot_id;
	p.TARGET_POS = flee_location;

	DoSend(&p);
}

void Player::SendRobotSetTargetPacket(const short robot_id, const short target_id)
{
	SC_PACKET_SET_TARGET p;
	p.SIZE = sizeof p;
	p.TYPE = S2C_SET_TARGET;
	p.ROBOT_ID = robot_id;
	p.TARGET_ID = target_id;

	DoSend(&p);
}

void Player::InitializeStat()
{
	hp = 100;
	kill_count = 0;
	death_count = 0;
	shield_flag = false;
	cur_weapon = EItem::DEFAULT;
	cur_anim = ANIM::Idle;

	attack_power = 5;
}

void Player::Respawn()
{
	NPC::Respawn();

	hp = 100;
	shield_flag = false;
	cur_anim = ANIM::Idle;
	cur_weapon = EItem::DEFAULT;
	SetWeaponStat();
}

void Player::SetWeaponStat()
{
	switch (cur_weapon) {
		case EItem::DEFAULT: {
			attack_power = 5;
			break;
		}
		case EItem::KNUCKLES: {
			attack_power = 10;
			break;
		}
	}
}
