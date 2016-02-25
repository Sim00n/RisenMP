#include "Server.h"

ServerConfig sc;
Server *server = nullptr;

int main()
{
	INFORM(SYSTEM, "Server application started.");

	bool running = true;

	//  This will be replaced by config reading
	sc.host = "127.0.0.1";
	sc.port = 6666;
	sc.max_clients = 10;
	sc.outgoing_limit = 400000;

	server = new Server(sc);
	server->Start();

	while (running)
	{
		if (server->isShuttingDown()) {
			running = false;
		}
		
		server->Pulse();

		Sleep(17); // freq = ~60Hz
	}

	if (server)
	{
		delete server;
		server = nullptr;
	}

	INFORM(SYSTEM, "Server is shutting down ...");
	system("PAUSE");
	return 0;
}