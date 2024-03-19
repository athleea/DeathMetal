#include "Timer.h"
#include "GameServer.h"

using namespace std;

Timer::Timer(class GameServer* server)
{
	game_server = server;
}

void Timer::TimerThread()
{
	priority_queue<TimerEvent> temp_queue;
	while (true) {
		TimerEvent value;
		auto current_time = chrono::steady_clock::now();

		while (not temp_queue.empty() and temp_queue.top().exec_time <= current_time) {
			value = temp_queue.top();
			temp_queue.pop();
			ProcessEvent(value);
			std::cout << std::format("exec : {}\n", std::chrono::duration<double>(temp_queue.top().exec_time - current_time));
		}

		if (true == event_queue.try_pop(value)) {
			if (value.exec_time > current_time) {
				event_queue.push(value);
				this_thread::sleep_for(1ms);
				continue;
			}

			ProcessEvent(value);
			continue;
		}
	}
}

void Timer::PushEvent(const TimerEvent value)
{
	event_queue.push(value);
}

void Timer::PushEvent(const short obj_id, const OP_TYPE type, const short duration)
{
	TimerEvent ev;
	ev.obj_id = obj_id;
	ev.type = type;
	ev.exec_time = std::chrono::steady_clock::now() + std::chrono::seconds(duration);

	event_queue.push(ev);
}

void Timer::ProcessEvent(const TimerEvent& value)
{
	switch (value.type) {
		case OP_TYPE::GAME_TIME:
		case OP_TYPE::ITEM_SPAWN:
		case OP_TYPE::SPAWN: {
			OVER_EXP* ex_over = new OVER_EXP{};
			ex_over->comp_type = value.type;
			game_server->PQCS(ex_over, value.obj_id);
			break;
		}
	}
}
