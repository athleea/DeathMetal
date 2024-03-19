#pragma once
#include "stdafx.h"

enum class NET_STATE {
	FREE,
	INGAME,
	LOBBY,
	MATCHING
};

enum class DB_QUERY {
	LOGIN,
	CREATE,
	LOGOUT
};

enum class OP_TYPE {
	ACCEPT,
	RECV,
	SEND,
	MOVE,
	GAME_TIME,
	SPAWN,
	RESPAWN,
	RESULT_LOGIN,
	RESULT_CREATE,
	ITEM_SPAWN
};

class OVER_EXP {
public:
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	char io_buf[MAX_BUF_SIZE]{};
	OP_TYPE comp_type;

	OVER_EXP() {
		wsabuf.len = MAX_BUF_SIZE;
		wsabuf.buf = io_buf;
		comp_type = OP_TYPE::RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

	OVER_EXP(char* packet) {
		wsabuf.len = packet[0];
		wsabuf.buf = io_buf;
		ZeroMemory(&overlapped, sizeof(overlapped));
		comp_type = OP_TYPE::SEND;
		memcpy(io_buf, packet, packet[0]);
	}
};

struct SD_LOGIN_INFO {
	DB_QUERY type;
	int client_id;
	char id[ID_SIZE];
	char pw[PW_SIZE];
};

struct SD_REGISTER {
	DB_QUERY type;
	int client_id;
	char id[ID_SIZE];
	char pw[PW_SIZE];
	char name[USER_NAME_SIZE];
};

struct SD_LOGOUT {
	char type;
	char name[USER_NAME_SIZE];
	int x;
	int y;
};

struct DS_LOGIN_INFO {
	int id;
	char name[USER_NAME_SIZE];
	NET_STATE state;
	int rating;
};