#pragma once
#include "stdafx.h"
#include "ProtocolThread.h"

class TimerEvent {
public:
	short obj_id;
	OP_TYPE type;
	std::chrono::steady_clock::time_point exec_time;

	constexpr bool operator<(const TimerEvent& value) const {
		return exec_time > value.exec_time;
	}
};

class Timer {
	concurrency::concurrent_priority_queue<TimerEvent> event_queue;
	class GameServer* game_server;
public:
	Timer(class GameServer* server);

	void TimerThread();
	void PushEvent(const TimerEvent value);
	void PushEvent(const short obj_id, const OP_TYPE type, const short duration);
	void ProcessEvent(const TimerEvent& value);
};
