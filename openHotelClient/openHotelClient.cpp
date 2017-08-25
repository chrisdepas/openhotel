/*
	Christopher De Pasquale
	18 December 2016
*/

#include "stdafx.h"
#include "getopt.h"
#include "SFML\Network.hpp"
#include "CHotelClient.h"
#include "CDebugLogger.h"
#include "OHNetwork.h"
#include "CGameView.h"
#include "CClientSettings.h"

#define OPENHOTELCLIENT_WINDOW_TITLE "OpenHotel Client"
#define OPENHOTELCLIENT_SETTINGSFILE "HotelClient.cfg"
#define OPENHOTELCLIENT_LOGFILE "HotelClient.log"
#define OPENHOTELCLIENT_VERSION 0.1f
#define OPENHOTELCLIENT_HELP_STRING "OpenHotelClient \
	USAGE: openhotelclient [-p server_port -s server_ip] [-h] \
	\
	OPTIONS: \
	-h Show help info \
	-p PORT - Port for server to connect to \
	-s IP - IP for server to connect to \
	-l FILE - Log file to write to \
	-c FILE - Config file to read from/write to"

int main(int argc, char* argv[])
{
	printf("OpenHotelClient version %2.1f\n", OPENHOTELCLIENT_VERSION);

	/* Parse command line args */
	int opt = -1;
	bool connectPortSupplied = false;
	bool connectServerSupplied = false;
	sf::IpAddress connectAddr;
	sf::Uint32 connectPort;
	std::string sLogFile = OPENHOTELCLIENT_LOGFILE;
	std::string sSettingsFile = OPENHOTELCLIENT_SETTINGSFILE;
	while ((opt = getopt(argc, argv, "h?s:p:l:")) != -1) {
		switch (opt) {

		case 'h':
		case '?':
			puts(OPENHOTELCLIENT_HELP_STRING);
			exit(EXIT_SUCCESS);

		case 'p':
			connectPort = (sf::Uint32)atoi(optarg);
			connectPortSupplied = true;
			printf("Recieved server port '%u'\n", connectPort);
			break;

		case 's':
			connectAddr = sf::IpAddress(optarg);
			connectServerSupplied = true;
			printf("Recieved server address '%s'\n", optarg);
			break;

		case 'l':
			sLogFile = optarg;
			printf("Log file set to '%s'\n", optarg);
			break;

		case 'c':
			sSettingsFile = optarg;
			printf("Settings file set to '%s'\n", optarg);
			break;
		}
	}

	/* Init debug logger */
	//CDebugLogger::Instance()->SetLogFile((char*)sLogFile.c_str());

	/* Hotel client */
	CHotelClient client;

	/* Choose server if details aren't supplied */
	if (!connectPortSupplied || !connectServerSupplied) {
		/* Load details from serverlist */
		std::vector<CHotelClient::SClientServerlistItem>& servers = client.GetServerList();
		if (servers.size() <= 0) {
			printf("No servers found.\n");
			exit(EXIT_SUCCESS);
		}

		/* Choose server prompt */
		int connectServer = -1;
		while (connectServer < 0 || connectServer > (int)servers.size() - 1) {
			/* Print serverlist */
			for (unsigned int i = 0; i < servers.size(); i++) {
				printf("[%i]\t%s\t%s:%u\n", i, servers[i].szServerName.c_str(),
					servers[i].addr.toString().c_str(), servers[i].uiPort);
			}

			printf("Choose a server\n>");
			scanf("%i", &connectServer);
			printf("\n");
		}

		connectAddr = servers[connectServer].addr;
		connectPort = servers[connectServer].uiPort;
	}

	/* Read client settings */
	CClientSettings settings;
	if (!settings.ReadFromFile(sSettingsFile)) {
		CDebugLogger::LogWarning("Coudln't read client settings file '%s'\n", sSettingsFile.c_str());
	}

	/* Save client settings file */
	if (!settings.SaveToFile(sSettingsFile)) {
		CDebugLogger::LogWarning("Coudln't save client settings to file '%s'\n", sSettingsFile.c_str());
	}
	
	/* Initialise game view */
	CGameView gameView;
	if (!gameView.Initialise(settings, OPENHOTELCLIENT_WINDOW_TITLE, &client)) {
		CDebugLogger::LogFatal("FATAL - Failed to initialise View.\n");
		client.Shutdown();
		exit(EXIT_FAILURE);
	}

	/* Connect to server */
	CDebugLogger::LogInfo("Connecting to server at %s:%u\n", connectAddr.toString().c_str(), connectPort);
	if (!client.Connect(connectAddr, connectPort)) {
		CDebugLogger::LogFatal("OpenHotelClient failed to connect to server.\n"); 
		exit(EXIT_SUCCESS);
	}

	/* Send local player info */
	OHPacket info = OHNetwork::CClientInfoUpdate::Create("TestUsedr","IgnoredPassword");
	if (!client.SendPacket(info)) {
		CDebugLogger::LogWarning("Failed to send local info packet to server.\n");
	}

	/* Reset clock */
	client.InitClock();

	/* Main loop */
	float fElapsedSeconds;
	while (client.IsConnected()) {
		/* Time since last frame */
		fElapsedSeconds = client.GetElapsedSeconds();

		/* Update game */
		client.Update(fElapsedSeconds);
		gameView.Update(client);

		/* Draw game */
		gameView.Draw(client);
	}

	/* Graceful shutdown */
	printf("Client shutting down\n");
	gameView.Shutdown();
	client.Shutdown();

	return 0;
}

