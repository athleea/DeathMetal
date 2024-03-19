#pragma once
#include "stdafx.h"
#include "ProtocolThread.h"
#include "NPC.h"
#include "Item.h"

class Player : public NPC {
public:
	u_short io_key;
	SOCKET socket;
	OVER_EXP ex_over;
	std::vector<char> recv_buf;
	std::atomic<NET_STATE> net_state;

	std::atomic_bool ready_flag;
	char name[USER_NAME_SIZE];
	std::atomic_int kill_count;
	std::atomic_int death_count;

	std::atomic<EItem> cur_weapon;
	std::atomic_bool shield_flag;
	std::atomic_bool is_lazy;

	Player(const u_short key, const SOCKET s);
	~Player();

	void InitializeStat();
	void Respawn() override;
	void SetWeaponStat();

	void DoSend(void* p);
	void DoRecv();

	void SendLoginOkPacket(const char* name);
	void SendLoginFailPacket();
	void SendLeavePacket(const int other_id);
	void SendGameReadyPacket(const std::array<std::string, MAX_ROOM_PLAYER>& names);
	void SendGameStartPacket();
	void SendGameOverPacket(const short kills[]);
	void SendTimePacket(float game_time);
	
	void SendMovePacket(const NPC* obj);
	void SendAnimPacket(const short o_id, const char anim_type);
	void SendHitPacket(const short attacker_id, const short damaged_id, const short damage, const char attack_type, const int bone_index);

	void SendSpawnObjectPacket(const Object* obj);
	void SendSpawnObjectPacket(const Item* obj);

	void SendChangeStatAndDestroyItem(const Player* pl, const short item_id);
	void SendKillMsgPacket(const short attacker, const short victim);

	void SendRobotFleePacket(const short robot_id, const vector3 flee_location);
	void SendRobotSetTargetPacket(const short robot_id, const short target_id);
};