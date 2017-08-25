/*
	Christopher De Pasquale
	November 2016
*/

#include "stdafx.h"
#include "getopt.h"
#include "SFML\Network.hpp"
#include <vector>
#define OPENHOTELSERVERLIST_VERSION 0.0f
#define OPENHOTELSERVERLIST_DEFAULT_PORT 6343
#define OPENHOTELSERVERLIST_HELP_STRING "todo"

struct SServerlistServer {
	char* szName;
	sf::IpAddress addr;
	int iPort;
};

bool SendServerlistServer(SServerlistServer* pServer, sf::TcpSocket* clientSock) {
	sf::Packet serverPacket;
	serverPacket << pServer->szName;
	serverPacket << pServer->addr.toInteger();
	serverPacket << sf::Uint32(pServer->iPort);
	return clientSock->send(serverPacket) == sf::Socket::Done;
}

int main(int argc, char* argv[])
{
	printf("OpenHotelServerList v%2.1f\n", OPENHOTELSERVERLIST_VERSION);

	/* Add dummy localhost server */
	std::vector<SServerlistServer> serverList;
	SServerlistServer localhost;
	localhost.iPort = 6843;
	localhost.addr = sf::IpAddress(127, 0, 0, 1);
	localhost.szName = "Localhost Server";
	serverList.push_back(localhost);

	/* Parse options */
	int opt = -1;
	int serverListenPort = OPENHOTELSERVERLIST_DEFAULT_PORT;
	while ((opt = getopt(argc, argv, "h?p:")) != -1) {
		switch (opt) {

		case 'h':
		case '?':
			puts(OPENHOTELSERVERLIST_HELP_STRING);
			exit(EXIT_SUCCESS);

		case 'p':
			serverListenPort = atoi(optarg);
			printf("Listen port changed to %i\n", serverListenPort);
			break;
		}
	}

	/* Listen */
	sf::TcpListener sockListen;
	sf::Socket::Status status = sockListen.listen(serverListenPort);
	if (status != sf::Socket::Done) {
		printf("ERROR - Unable to listen on port %i\n", serverListenPort);
		exit(EXIT_FAILURE);
	}
	else {
		printf("Listening on port %i\n", serverListenPort);
	}

	/* Dump list of servers and disconnect clients */
	sf::TcpSocket newClient;
	bool bRunning = true; 
	while (bRunning) {
		sockListen.accept(newClient);
		for (unsigned int i = 0; i < serverList.size(); i++) {
			SendServerlistServer(&serverList[i], &newClient);
		}
		printf("Sent a client the serverlist.\n");
		newClient.disconnect();
	}

	printf("Shutting down\n");

	return 0;
}

