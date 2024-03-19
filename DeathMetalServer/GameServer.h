#pragma once
#include "stdafx.h"
#include "ProtocolThread.h"
#include "RoomManager.h"

class DBManager;

class GameServer {
public:
	GameServer();
	~GameServer();

	void Initialize();
	void StartServer();
	void CreateWorkerThread();
	void WorkerThread();
	void PQCS(OVER_EXP* exover, const short c_id);

	void PacketConstruct(const short);
	void ProcessPacket(const short, char*);
	void SendQuery(void* p, const int size);

	void DisconnectClient(const short);
	
	const u_short GetNewClientID();

public:
	HANDLE handle_iocp;
	concurrency::concurrent_unordered_map<u_short, Player*> clients;
	RoomManager* room_mgr;
	DBManager* db_mgr;
	class Timer* timer_mgr;
private:
	OVER_EXP accept_over;
	SOCKET listen_socket;
	SOCKET client_socket;
	std::vector<std::thread> game_threads;
	std::atomic_ushort accept_code;
};
