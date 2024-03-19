#include "GameServer.h"
#include "FleeRobot.h"
#include "AttackRobot.h"
#include "Timer.h"
#include "DBManager.h"
#include "ItemManager.h"
#include "MonsterManager.h"

GameServer::GameServer()
{
	accept_code = 0;
	room_mgr = new RoomManager{};
}

GameServer::~GameServer()
{
	WSACleanup();

	for (auto [key, cl] : clients) {
		delete cl;
	}

	delete room_mgr;
	delete timer_mgr;
	delete db_mgr;
}
void GameServer::Initialize()
{
	WSADATA WSAData;
	int retval = WSAStartup(MAKEWORD(2, 2), &WSAData);

	listen_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	::bind(listen_socket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));

	::listen(listen_socket, SOMAXCONN);

	handle_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_socket), handle_iocp, 999, 0);
}
void GameServer::StartServer()
{
	client_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&accept_over, sizeof(accept_over.overlapped));
	accept_over.comp_type = OP_TYPE::ACCEPT;
	::AcceptEx(listen_socket, client_socket, accept_over.io_buf, NULL, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &accept_over.overlapped);

	timer_mgr = new Timer{ this };
	db_mgr = new DBManager{ this };

	CreateWorkerThread();
}
void GameServer::CreateWorkerThread()
{
	int num_threads = std::thread::hardware_concurrency();
	num_threads = 4;
	for (int i = 0; i < num_threads; ++i)
		game_threads.emplace_back(std::thread(&GameServer::WorkerThread, this));

	game_threads.emplace_back([this]() { db_mgr->DatabaseThread(); });
	game_threads.emplace_back([this]() { timer_mgr->TimerThread(); });

	for (auto& th : game_threads)
		th.join();
}
void GameServer::PacketConstruct(const short c_id)
{
	Player* cur_client = clients[c_id];
	size_t rest_byte = cur_client->recv_buf.size();

	while (rest_byte > 0) {
		int packet_size = cur_client->recv_buf[0];
		if (packet_size <= cur_client->recv_buf.size()) {
			char* buf = cur_client->recv_buf.data();
			ProcessPacket(c_id, buf);
			cur_client->recv_buf.erase(cur_client->recv_buf.begin(), cur_client->recv_buf.begin() + packet_size);
			rest_byte -= packet_size;
		}
		else {
			break;
		}
	}
}

void GameServer::WorkerThread()
{
	while (true) {
		DWORD io_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* overlapped;
		BOOL ret = GetQueuedCompletionStatus(handle_iocp, &io_bytes, &key, &overlapped, INFINITE);

		OVER_EXP* comp_exover = reinterpret_cast<OVER_EXP*>(overlapped);

		short comp_id = static_cast<short>(key);
		if (FALSE == ret) {
			if (comp_exover->comp_type == OP_TYPE::ACCEPT)
				std::cout << "[GQCS Error] : Accept\n";
			else {
				DisconnectClient(comp_id);
				if (comp_exover->comp_type == OP_TYPE::SEND)
					delete comp_exover;
				continue;
			}
		}

		if (0 == io_bytes && (comp_exover->comp_type == OP_TYPE::RECV || comp_exover->comp_type == OP_TYPE::SEND)) {
			DisconnectClient(comp_id);
			if (comp_exover->comp_type == OP_TYPE::SEND)
				delete comp_exover;
			continue;
		}

		switch (comp_exover->comp_type) {
			case OP_TYPE::RECV: {
				Player* player = clients[comp_id];
				for (u_int i = 0; i < io_bytes; ++i)
					player->recv_buf.push_back(comp_exover->io_buf[i]);
				PacketConstruct(comp_id);
				player->DoRecv();
				break;
			}
			case OP_TYPE::SEND: {
				delete comp_exover;
				break;
			}
			case OP_TYPE::ACCEPT: {
				const u_short new_key = GetNewClientID();
				if (MAX_CLIENT == new_key) {
					// send login fail
					closesocket(client_socket);
				}
				else {
					Player* new_player = new Player{ new_key, client_socket };
					clients[new_key] = new_player;

					CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_socket), handle_iocp, new_key, 0);

					new_player->DoRecv();
					client_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

					std::cout << std::format("{} : Accept Player\n", new_key);
				}

				ZeroMemory(&accept_over.overlapped, sizeof accept_over.overlapped);
				int addr_size = sizeof(sockaddr_in) + 16;
				::AcceptEx(listen_socket, client_socket, accept_over.io_buf, NULL, addr_size, addr_size, NULL, &accept_over.overlapped);
				std::cout << "Accept\n";
				break;
			}
			case OP_TYPE::GAME_TIME: {
				Room* cur_room = room_mgr->rooms[comp_id];
				if (cur_room == nullptr) {
					delete comp_exover;
					break;
				}

				auto game_time = std::chrono::steady_clock::now();
				float cur_time = std::chrono::duration<float>(game_time - cur_room->start_time).count();

				// 게임 시간 오버
				if (cur_time >= ROUND_TIME) {
					room_mgr->rooms[comp_id]->GameOver();
					room_mgr->rooms.unsafe_erase(comp_id);
				}
				else {
					for (Player* player : cur_room->players) {
						if (player->cur_room_id != comp_id) {
							continue;
						}
						player->SendTimePacket(ROUND_TIME - cur_time);
					}
					TimerEvent t_event{ };
					t_event.obj_id = comp_id;
					t_event.type = OP_TYPE::GAME_TIME;
					t_event.exec_time = game_time + std::chrono::seconds(1);
					timer_mgr->PushEvent(t_event);
				}

				delete comp_exover;
				break;
			}
			case OP_TYPE::SPAWN: {
				Object* obj = clients[comp_id];
				if (obj == nullptr) {
					delete comp_exover;
					break;
				}

				Room* cur_room = room_mgr->rooms[obj->cur_room_id];
				if (cur_room == nullptr) {
					delete comp_exover;
					break;
				}

				for (Player* pl : cur_room->players) {
					pl->SendSpawnObjectPacket(obj);
				}

 				delete comp_exover;
				break;
			}
			case OP_TYPE::ITEM_SPAWN: {
				Room* cur_room = room_mgr->rooms[comp_id];
				if (cur_room != nullptr) {
					int count = 0;
					for (const vector3& loc : cur_room->item_mgr->item_spawn_locations) {
						Item* spawn_item = cur_room->item_mgr->SpawnNewItem(false);
						if (spawn_item != nullptr) {
							//spawn_item->location = vector3(739, -4410, 50);
							//spawn_item->type = EItem::KNUCKLES;
							for (Player* pl : cur_room->players) {
								pl->SendSpawnObjectPacket(spawn_item);
							}
						}
						count++;
						if (count > 30) break;
					}
				}

				delete comp_exover;
				break;
			}
			case OP_TYPE::RESULT_LOGIN: {
				DS_LOGIN_INFO* p = reinterpret_cast<DS_LOGIN_INFO*>(comp_exover->io_buf);

				Player* player = clients[comp_id];
				if (p->state == NET_STATE::INGAME) // 재접속 처리
					return;

				player->SendLoginOkPacket(p->name);
				strcpy_s(player->name, p->name);
				player->net_state = NET_STATE::LOBBY;

				std::cout << "Login Player\n";
				delete comp_exover;
			}
				break;
			default: {
				break;
			}
		}
	}
}

void GameServer::PQCS(OVER_EXP* exover, const short c_id)
{
	PostQueuedCompletionStatus(handle_iocp, 1, c_id, &exover->overlapped);
}


void GameServer::DisconnectClient(const short c_id)
{
	Player* client = clients[c_id];
	if (client == nullptr) return;

	NET_STATE temp_state = client->net_state;
	client->net_state = NET_STATE::FREE;
	clients[c_id] = nullptr;
	clients.unsafe_erase(c_id);
	std::cout << std::format("disconnect client {}\n", c_id);

	Room* cur_room = room_mgr->rooms[client->cur_room_id];
	if (cur_room == nullptr) return;
	--cur_room->players_num;

	if (temp_state == NET_STATE::INGAME) {
		if (cur_room->players_num <= 1) {
			room_mgr->rooms[client->cur_room_id]->GameOver();
			room_mgr->rooms[client->cur_room_id] = nullptr;
			room_mgr->rooms.unsafe_erase(client->cur_room_id);
		}
		else {
			for (auto cl : room_mgr->rooms[client->cur_room_id]->players) {
				if (cl->id == c_id) continue;
				if (cl->net_state == NET_STATE::FREE) continue;
				cl->SendLeavePacket(c_id);
			}
		}
	}

	closesocket(client->socket);
	client = nullptr;
}

void GameServer::ProcessPacket(const short c_id, char* buf)
{
	switch (buf[1]) {
		case C2S_GAMEOVER: {
			Player* player = reinterpret_cast<Player*>(clients[c_id]);
			room_mgr->rooms[player->cur_room_id]->GameOver();
			break;
		}
		case C2S_LEAVE: {
			CS_PACKET_LEAVE* packet = reinterpret_cast<CS_PACKET_LEAVE*>(buf);
			DisconnectClient(c_id);
			break;
		}
		case C2S_CREATE: {
			CS_PACKET_CREATE_PACKET* packet = reinterpret_cast<CS_PACKET_CREATE_PACKET*>(buf);

			SD_REGISTER db_packet{};
			db_packet.type = DB_QUERY::CREATE;
			db_packet.client_id = c_id;
			strcpy_s(db_packet.id, packet->id);
			strcpy_s(db_packet.pw, packet->pw);
			strcpy_s(db_packet.name, packet->name);

			SendQuery(&db_packet, sizeof SD_REGISTER);
			break;
		}
		case C2S_LOGIN: {
			CS_PACKET_LOGIN_PACKET* packet = reinterpret_cast<CS_PACKET_LOGIN_PACKET*>(buf);

			SD_LOGIN_INFO db_packet{};
			db_packet.type = DB_QUERY::LOGIN;
			db_packet.client_id = c_id;
			strcpy_s(db_packet.id, packet->id);
			strcpy_s(db_packet.pw, packet->pw);

			SendQuery(&db_packet, sizeof SD_LOGIN_INFO);
			break;
		}
		case C2S_MATCHING: {
			CS_PACKET_MATCHING* packet = reinterpret_cast<CS_PACKET_MATCHING*>(buf);
			Player* player = reinterpret_cast<Player*>(clients[c_id]);
			if (player == nullptr) break;

			player->net_state = NET_STATE::MATCHING;
			std::cout << std::format("{} matching\n", player->io_key);
			room_mgr->AddPlayer(player);
			if (room_mgr->MakeRoom(timer_mgr) == true) {
				std::cout << "Success Making Room!!\n";
			}
			break;
		}
		case C2S_READY_OK: {
			Player* player = reinterpret_cast<Player*>(clients[c_id]);
			if (player == nullptr) break;
			player->ready_flag = true;

			bool is_all_ready = room_mgr->CheckPlayersReadyState(player->cur_room_id);
			if (is_all_ready == true) {
				room_mgr->rooms[player->cur_room_id]->StartGame();
			}
			std::cout << std::format("{} ready ok\n", player->id);
			break;
		}
		case C2S_MOVE: {
			CS_PACKET_MOVE* packet = reinterpret_cast<CS_PACKET_MOVE*>(buf);

			Player* player = clients[c_id];
			if (player == nullptr) break;

			player->location = packet->LOCATION;
			player->rotation = packet->ROTATION;
			player->velocity = packet->VELOCITY;

			auto cur_room = room_mgr->rooms[player->cur_room_id];
			if (cur_room == nullptr) break;

			for (auto& cl : cur_room->players) {
				if (cl->id == player->id) continue;
				if (cl->net_state != NET_STATE::INGAME) continue;
				cl->SendMovePacket(player);
			}
			break;
		}
		case C2S_ANIM: {
			CS_PACKET_ANIM* packet = reinterpret_cast<CS_PACKET_ANIM*>(buf);

			Player* player = clients[c_id];
			if (player == nullptr) break;

			switch (packet->ANIM_TYPE) {
				case THROW_AXE: {
					player->cur_weapon = EItem::DEFAULT;
					player->SetWeaponStat();
					break;
				}
				case SHILED_ON: {
					player->shield_flag = true;
					break;
				}
				case SHILED_OFF: {
					player->shield_flag = false;
					break;
				}
				default:
					break;
			}

			for (auto& cl : room_mgr->rooms[player->cur_room_id]->players) {
				if (cl->id == player->id) continue;
				if (cl->net_state != NET_STATE::INGAME) continue;
				cl->SendAnimPacket(player->id, packet->ANIM_TYPE);
			}
			break;
		}
		case C2S_HIT: {
			CS_PACKET_HIT* packet = reinterpret_cast<CS_PACKET_HIT*>(buf);

			switch (packet->ATTACK_TYPE) {
				case ROBOT_ATTACK: {
					Player* victim = clients[c_id];
					if (victim == nullptr) break;
					if (victim->hp == 0) break;
					victim->hp -= packet->DAMAGED_ID;

					if (victim->hp <= 0) {
						for (auto& cl : room_mgr->rooms[victim->cur_room_id]->players) {
							if (cl == nullptr) continue;
							if (cl->net_state != NET_STATE::INGAME) continue;
							cl->SendAnimPacket(victim->id, DEAD);
						}

						timer_mgr->PushEvent(victim->io_key, OP_TYPE::SPAWN, 3);
						victim->Respawn();
					}
					else {
						for (auto& cl : room_mgr->rooms[victim->cur_room_id]->players) {
							if (cl == nullptr) continue;
							if (cl->net_state != NET_STATE::INGAME) continue;
							cl->SendHitPacket(999, victim->id, victim->hp, ROBOT_ATTACK, -1);
						}
					}
					break;
				}
				case THROW_AXE: {
					Player* attacker = clients[c_id];
					if (attacker == nullptr) break;
					Player* victim = room_mgr->rooms[attacker->cur_room_id]->players[packet->DAMAGED_ID];
					if (victim == nullptr) break;
					if (victim->hp <= 0) break;

					for (auto& cl : room_mgr->rooms[attacker->cur_room_id]->players) {
						if (cl->net_state != NET_STATE::INGAME) continue;
						cl->SendAnimPacket(victim->id, STURN);
						if (attacker->io_key != c_id)
							cl->SendAnimPacket(attacker->id, THROW_AXE);
					}

					break;
				}
				default: {
					Player* attacker = clients[c_id];
					if (attacker == nullptr) break;
					Player* victim = room_mgr->rooms[attacker->cur_room_id]->players[packet->DAMAGED_ID];
					if (victim == nullptr) break;

					if (victim->shield_flag == true) break;
					if (victim->hp == 0) break;

					short damage = 0;
					if (attacker->hp < 40) {
						damage = attacker->attack_power + 3;
					}
					else {
						damage = attacker->attack_power;
					}
					victim->hp -= damage;
					std::cout << std::format("{} attack {} by {} dmg : {}\n", attacker->id, victim->id, damage, victim->hp);
					if (victim->hp <= 0) {
						for (auto& cl : room_mgr->rooms[attacker->cur_room_id]->players) {
							if (cl->net_state != NET_STATE::INGAME) continue;
							cl->SendAnimPacket(victim->id, DEAD);
							cl->SendKillMsgPacket(attacker->id, victim->id);
						}

						timer_mgr->PushEvent(victim->io_key, OP_TYPE::SPAWN, 5);
						victim->Respawn();
					}
					else {
						for (auto& cl : room_mgr->rooms[attacker->cur_room_id]->players) {
							if (cl->net_state != NET_STATE::INGAME) continue;
							cl->SendHitPacket(attacker->id, victim->id, victim->hp, packet->ATTACK_TYPE, packet->BONE_INDEX);
						}
					}
				}
					break;
			}

			

			break;
		}
		case C2S_COLLECT_ITEM: {
			CS_PACKET_COLLECT_ITEM* packet = reinterpret_cast<CS_PACKET_COLLECT_ITEM*>(buf);
			Player* cur_player = clients[c_id];
			if (cur_player == nullptr) break;

			Room* cur_room = room_mgr->rooms[cur_player->cur_room_id];
			if (cur_room == nullptr) break;

			Item* acquire_item = cur_room->item_mgr->item_map[packet->item_id];
			if (acquire_item == nullptr) break;
			cur_room->item_mgr->DestroyItem(acquire_item->id);


			switch (acquire_item->type) {
				case EItem::AXE:
				case EItem::KNUCKLES:
					cur_player->cur_weapon = acquire_item->type;
					cur_player->SetWeaponStat();
					break;
				case EItem::HEALPACK: {
					cur_player->hp += 50;
					break;
				}
				default: break;
			}

			for (Player* pl : cur_room->players) {
				if (pl == nullptr) continue;
				if (pl->net_state != NET_STATE::INGAME) continue;
				pl->SendChangeStatAndDestroyItem(cur_player, acquire_item->id);
			}

			

			break;
		}
		case C2S_HIT_MONSTER: {
			CS_PACKET_HIT_MONSTER* packet = reinterpret_cast<CS_PACKET_HIT_MONSTER*>(buf);
			Player* cur_player = clients[c_id];
			if (cur_player == nullptr) break;

			Room* cur_room = room_mgr->rooms[cur_player->cur_room_id];
			if (cur_room == nullptr) break;

			NPC* monster = cur_room->monster_mgr->monster_map[packet->robot_id];
			if (monster == nullptr) break;

			if (monster->hp <= 0) break;

			monster->hp -= cur_player->attack_power;
			if (monster->hp <= 0) {
				Item* item = cur_room->item_mgr->SpawnNewItem(true);
				item->location = packet->location;
				//std::cout << std::format("{}, {}, {}, {}, {}\n", item->location.x, item->location.y, item->location.z, item->type);
				for (Player* pl : cur_room->players) {
					pl->SendAnimPacket(monster->id, DEAD);
					pl->SendSpawnObjectPacket(item);
				}

				if (packet->robot_type == TYPE_FLEE_ROBOT) {
					reinterpret_cast<AttackRobot*>(monster)->Respawn();
				}
				else {
					reinterpret_cast<FleeRobot*>(monster)->Respawn();
				}

				break;
			}

			if (packet->robot_type == TYPE_FLEE_ROBOT) {
				// Do Flee
				FleeRobot* flee_robot = reinterpret_cast<FleeRobot*>(monster);
				flee_robot->DoFlee();

				for (Player* pl : cur_room->players) {
					pl->SendRobotFleePacket(flee_robot->id, flee_robot->flee_point);
				}
			}
			else if (packet->robot_type == TYPE_ATTACK_ROBOT) {
				// Set Target
				AttackRobot* attack_robot = reinterpret_cast<AttackRobot*>(monster);
				attack_robot->target_id = cur_player->id;

				for (Player* pl : cur_room->players) {
					pl->SendRobotSetTargetPacket(attack_robot->id, cur_player->id);
				}
			}
			break;
		}

		case C2S_ATTACKED: {
			CS_PACKET_ATTACKED* packet = reinterpret_cast<CS_PACKET_ATTACKED*>(buf);

			Player* victim = clients[c_id];
			if (victim == nullptr) break;
			if (victim->hp == 0) break;
			victim->hp -= packet->DAMAGE;

			if (victim->hp <= 0) {
				for (auto& cl : room_mgr->rooms[victim->cur_room_id]->players) {
					if (cl == nullptr) continue;
					if (cl->net_state != NET_STATE::INGAME) continue;
					cl->SendAnimPacket(victim->id, DEAD);
				}

				timer_mgr->PushEvent(victim->io_key, OP_TYPE::SPAWN, 3);
				victim->Respawn();
			}
			else {
				for (auto& cl : room_mgr->rooms[victim->cur_room_id]->players) {
					if (cl == nullptr) continue;
					if (cl->net_state != NET_STATE::INGAME) continue;

				}
			}
			break;
		}

		default: {
			std::cout << "unknown packet type error \n";
			DebugBreak();
		}
	}
}

void GameServer::SendQuery(void* q, const int size)
{
	DB_BUF query{};
	memcpy(query.buf, reinterpret_cast<char*>(q), size);
	db_mgr->PushEvent(query);
}

const u_short GameServer::GetNewClientID()
{
	while (accept_code < MAX_CLIENT) {
		if (clients.count(accept_code) == 0) {
			return accept_code++;
		}
		++accept_code;
	}
	return MAX_CLIENT;
}

bool is_player(const short id)
{
	if (id < MAX_ROOM_PLAYER) {
		return true;
	}
	return false;
}

bool is_monster(const short id)
{
	if (id >= MAX_ROOM_PLAYER && id < MAX_ROOM_PLAYER + MAX_FLEE_ROBOT + MAX_ATTACK_ROBOT) {
		return true;
	}
	return false;
}

bool is_item(const short id)
{
	if (id >= MAX_ROOM_PLAYER + MAX_FLEE_ROBOT + MAX_ATTACK_ROBOT) {
		return true;
	}
	return false;
}