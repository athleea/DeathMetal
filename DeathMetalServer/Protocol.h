#pragma once

constexpr int SERVER_PORT = 8000;

constexpr float LEVEL_WIDTH = 12000.0f;
constexpr float LEVEL_HEIGHT = 12000.0f;
constexpr float ROUND_TIME = 600.0f;
constexpr short ID_SIZE = 20;
constexpr short USER_NAME_SIZE = 20;
constexpr short PW_SIZE = 30;

constexpr int MAX_CLIENT = 10000;
constexpr int MAX_ROOM_PLAYER = 3;

constexpr short ITEM_COUNT = 3;
enum class EItem : unsigned char
{
	KNUCKLES,
	AXE,
	HEALPACK,
	DEFAULT,
};

struct vector2 {
	float x = 0.f;
	float y = 0.f;
};
struct vector3 {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	bool operator == (const vector3& other) const {
		if (x == other.x && y == other.y && z == other.z)
			return true;
		return false;
	}
};

constexpr unsigned int MAX_WORLD_WIDTH = 7500;
constexpr unsigned int MAX_WORLD_HEIGHT = 7500;

constexpr unsigned char STATE_IDLE = 0;
constexpr unsigned char STATE_RUN = 1;
constexpr unsigned char STATE_ATTACK = 2;
constexpr unsigned char STATE_DEAD = 2;

constexpr char TYPE_ATTACK_ROBOT = 0;
constexpr char TYPE_FLEE_ROBOT = 1;

constexpr auto  LOGOUT = -1;

constexpr unsigned char C2S_LOGIN						= 1;
constexpr unsigned char C2S_MATCHING				= 2;
constexpr unsigned char C2S_MOVE						= 3;
constexpr unsigned char C2S_LEAVE						= 4;
constexpr unsigned char C2S_ANIM						= 5;
constexpr unsigned char C2S_HIT							= 6;
constexpr unsigned char C2S_DEAD						= 7;
constexpr unsigned char C2S_GAMEOVER				= 8;
constexpr unsigned char C2S_HIT_MONSTER			= 9;
constexpr unsigned char C2S_COLLECT_ITEM			= 10;
constexpr unsigned char C2S_ATTACKED				= 11;
constexpr unsigned char C2S_CREATE					= 12;
constexpr unsigned char C2S_READY_OK				= 13;

constexpr unsigned char S2C_GAME_START						= 1;
constexpr unsigned char S2C_LOGIN_OK							= 2;
constexpr unsigned char S2C_LOGIN_FAIL							= 3;
constexpr unsigned char S2C_SIGNUP_FAIL							= 4;
constexpr unsigned char S2C_MOVE									= 5;
constexpr unsigned char S2C_ENTER									= 6;
constexpr unsigned char S2C_LEAVE									= 7;
constexpr unsigned char S2C_TIMER									= 8;
constexpr unsigned char S2C_GAME_RESULT						= 9;
constexpr unsigned char S2C_ANIM									= 10;
constexpr unsigned char S2C_HIT										= 11;
constexpr unsigned char S2C_KILL									= 12;
constexpr unsigned char S2C_SPAWN								= 13;
constexpr unsigned char S2C_CHANGE_STAT_REMOVE_ITEM	= 14;
constexpr unsigned char S2C_ROBOT_FLEE							= 15;
constexpr unsigned char S2C_ROBOT_RESPAWN					= 16;
constexpr unsigned char S2C_SET_TARGET							= 17;
constexpr unsigned char S2C_GAME_READY						= 18;

constexpr short ROBOT_DAMAGE		= 5;

constexpr char ROBOT_ATTACK		= 0;
constexpr char LEFT_HOOK				= 1;
constexpr char RIGHT_PUNCH			= 2;
constexpr char LEFT_PUNCH			= 3;
constexpr char RIGHT_UPPERCUT		= 4;
constexpr char ROTATE_ATTACK		= 5;
constexpr char DASH_ATTACK			= 6;

constexpr char THROW_AXE			= 7;
constexpr char DASH						= 8;
constexpr char SHILED_ON				= 9;
constexpr char SHILED_OFF				= 10;
constexpr char DEAD						= 11;
constexpr char STURN					= 12;


constexpr int	PACKET_SIZE				= 255;
constexpr auto MAX_BUF_SIZE				= 4096;
constexpr auto MAX_FLEE_ROBOT			= 1;
constexpr auto MAX_ATTACK_ROBOT	= 2;

#pragma pack(push ,1)

struct PLAYER {
	short ID;
	vector3 Location;
	vector3 Rotator;
	vector3 Velocity;

	bool IsAttack;
	char AttackCombo;
	short KillCount;
	short DeathCount;
	EItem Weapon;
	float Speed;
	char Name[USER_NAME_SIZE];

	void Reset()
	{
		ID = LOGOUT;
		Location = { 0.f, 0.f, 100.f };
		Rotator = { 0.f, 0.f, 0.f };
		Velocity = { 0.f, 0.f, 0.f };

		IsAttack = false;
		AttackCombo = 0;
		KillCount = 0;
		Weapon = EItem::DEFAULT;
	}
};

struct ROBOT_MONSTER {
	vector3 Location{ 0.f, 0.f, 100.f };
	vector3 Rotator{ 0.f, 0.f, 0.f };
	vector2 Target{ 0.f, 0.f };
	unsigned char State = STATE_IDLE;
};

struct SC_PACKET_LOGIN_OK {
	char size;
	char type;
	char name[USER_NAME_SIZE];
};

struct SC_PACKET_FAIL_LOGIN {
	char size;
	char type;
};

struct SC_PACKET_GAME_START {
	char SIZE;
	char TYPE;
	short ID;
	char NAMES[MAX_ROOM_PLAYER][USER_NAME_SIZE];
};

struct SC_PACKET_MOVE {
	char SIZE;
	char TYPE;
	short ID;
	vector3 LOCATION;
	vector3 ROTATION;
	vector3 VELOCITY;
};

struct SC_PACKET_LEAVE {
	char size;
	char type;
	short id;
};

struct SC_PACKET_ANIM {
	char SIZE;
	char TYPE;
	short ID;
	char ANIM_TYPE;
};

struct SC_PACKET_KILL {
	char SIZE;
	char TYPE;
	short KILLER;
	short VICTIM;
};

struct SC_PACKET_SPAWN {
	char SIZE;
	char TYPE;
	short OBJ_ID;
	vector3 LOCATION;
	vector3 ROTATION;
	EItem ITEM_TYPE;
};

struct SC_PACKET_CHANGE_STAT_REMOVE_ITEM {
	char SIZE;
	char TYPE;
	short ITEM_ID;
	short PLAYER_ID;
	int HP;
	EItem WEAPON;
};

struct SC_PACKET_RESPAWN {
	char SIZE;
	char TYPE;
	int ROBOT_ID;
};

struct SC_PACKET_GAME_OVER {
	char SIZE;
	char TYPE;
	short KILL_RESULT[MAX_ROOM_PLAYER];
};

struct SC_PACKET_ROBOT_FLEE {
	char SIZE;
	char TYPE;

	short ROBOT_ID;
	vector3 TARGET_POS;
};

struct SC_PACKET_TIMER {
	char SIZE;
	char TYPE;
	float TIME;
};

struct SC_PACKET_HIT {
	char SIZE;
	char TYPE;
	short ATTKER_ID;
	short DAMAGED_ID;
	short HP;
	char ATTACK_TYPE;
	int BONE_INDEX;
};

struct SC_PACKET_SET_TARGET {
	char SIZE;
	char TYPE;
	short ROBOT_ID;
	short TARGET_ID;
};

struct CS_PACKET_LOGIN_PACKET {
	char	size;
	char	type;

	char id[ID_SIZE];
	char pw[PW_SIZE];
};

struct CS_PACKET_CREATE_PACKET {
	char	size;
	char	type;

	char id[ID_SIZE];
	char pw[PW_SIZE];
	char name[USER_NAME_SIZE];
};

struct CS_PACKET_MATCHING
{
	char SIZE;
	char TYPE;
};

struct CS_PACKET_LEAVE
{
	char	size;
	char	type;
};

struct CS_PACKET_MOVE {
	char	SIZE;
	char	TYPE;
	vector3 LOCATION;
	vector3 ROTATION;
	vector3 VELOCITY;
};

struct CS_PACKET_ANIM {
	char SIZE;
	char TYPE;

	char ANIM_TYPE;
};

struct CS_PACKET_HIT {
	char	SIZE;
	char	TYPE;
	short DAMAGED_ID;
	char ATTACK_TYPE;
	int BONE_INDEX;
};

struct cs_packet_gameover {
	char	size;
	char	type;
	int		kill_count;
};

struct CS_PACKET_HIT_MONSTER {
	char	size;
	char	type;

	int	    damage;
	int		robot_id;
	char	robot_type;
	vector3 location;
};
struct CS_PACKET_COLLECT_ITEM {
	char	size;
	char	type;

	short	item_id;
	EItem item_type;
};

struct CS_PACKET_ATTACKED {
	char SIZE;
	char TYPE;
	int DAMAGE;
};

struct CS_PACKET_READY_OK {
	char SIZE;
	char TYPE;
};
#pragma pack (pop)