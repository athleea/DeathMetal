#include "GameServer.h"

GameServer gServer{};

int main()
{
	setlocale(LC_ALL, "korean");
	std::wcout.imbue(std::locale("korean"));

	gServer.Initialize();
	gServer.StartServer();
}

