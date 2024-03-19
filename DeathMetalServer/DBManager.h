#pragma once
#include "stdafx.h"
#include <sqlext.h>
#include "ProtocolThread.h"


constexpr int MAX_QUERY_SIZE = 256;

class GameServer;

struct DB_BUF {
	char buf[MAX_QUERY_SIZE];
};

class DBManager {
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	concurrency::concurrent_queue<DB_BUF> query_queue;
	GameServer* game_server;

public:
	DBManager(GameServer* server);
	~DBManager();

	void ConnectDB(const std::wstring& odbc_name);

	void PushEvent(DB_BUF&);

	void ExecLogin(const short s_id, const char* login_id, const char* login_pw);
	void ExecRegister(const short s_id, const char* login_id, const char* login_pw, const char* name);
	void ExecLogout(const char* login_id, const vector3 loc, const vector3 rot, const int hp);

	void DatabaseThread();
};