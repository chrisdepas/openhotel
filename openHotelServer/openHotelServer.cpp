/*
	Christopher De Pasquale
	18 December 2016
*/

#include "stdafx.h"
#include "getopt.h"
#include "SFML\Network.hpp"
#include "CHotelServer.h"
#include "CDebugLogger.h"
#include <mutex>
#include <iostream>

/* Version of OpenHotelServer */
#define OPENHOTELSERVER_VERSION 0.0

/* Default max clients connected to hotel */
#define OPENHOTELSERVER_DEFAULT_MAXCLIENTS 32
/* Range of accepted maxclient numbers */
#define OPENHOTELSERVER_MIN_MAXCLIENTS 1		
#define OPENHOTELSERVER_MAX_MAXCLIENTS 1000
/* Default port used by OpenHotelServer */
#define OPENHOTELSERVER_DEFAULT_PORT 6843
/* Default file used by OpenHotelServer to store hotel */
#define OPENHOTELSERVER_DEFAULT_FILENAME "OpenHotel.htl"
/* Default file used by OpenHotelServer as log file*/
#define OPENHOTELSERVER_LOGFILE "HotelServer.log"
/* Default file used by OpenHotelServer to store registered users */
#define OPENHOTELSERVER_REGISTEREDUSERFILE "HotelUser.xrf"
/* Help string given when -h / -? arg is supplied */
#define OPENHOTELSERVER_HELP_STRING "OpenHotelServer \
	USAGE: openhotelserver [-p server_port] [-h] [-?] [-f] [ \
	\
	OPTIONS: \
	-h Show help info \
	-? Show help info \
	-p PORT - Port for server to listen on (Default 6843)\
	-f FILE - Path to hotel file to be loaded (Default is OpenHotel.htl) \
	-l FILE - Path to log file used by server (Default is HotelServer.log) \
	-u FILE - Path to registered users file used by server (Default is HotelUser.xrf)"
/* Size of command buffer */
#define CMD_BUFFER_SIZE 1024
/* Number of recognised commands */
#define CMD_COUNT 6
/* Indices for commands */
#define CMD_HELP_INDEX 0
#define CMD_EXIT_INDEX 1
#define CMD_QUIT_INDEX 2
#define CMD_LIST_INDEX 3
#define CMD_KICK_INDEX 4
#define CMD_REGISTER_INDEX 5
/* Help string for commands */
#define CMD_HELP_STRING "OpenHotelServer Commands \n\
	help - Show this message \n\
	\n\
	exit / quit - Safely exit server\n\
	listplayers - show all players with IDs on server\n\
	kickplayer - kick a player by ID\n\
	registerplayer - Register a player by ID"

/* Globals for command buffer */
std::mutex g_CmdMutex;
char g_LastCmd[CMD_BUFFER_SIZE];
bool g_bParseCmd = false;
/* List of recognised commands */
const char* g_CmdList[CMD_COUNT] = {
	"help",			/* show list of commands */
	"exit",			/* stop server running */
	"quit",			/* same as 'exit'*/
	"listplayers",	/* Show a list of all players, next to their ID */
	"kickplayer",	/* Kick a player from the server by ID */
	"registerplayer"/* Register a player by ID */
};

/* Forward declaration of command reader thread*/
int cmd_thread();

int main(int argc, char** argv)
{
	printf("OpenHotelServer v%2.1f\n", OPENHOTELSERVER_VERSION);

	/* Parse command-line options */
	int serverListenPort = OPENHOTELSERVER_DEFAULT_PORT;
	int serverMaxClients = OPENHOTELSERVER_DEFAULT_MAXCLIENTS;
	std::string serverHotelFile = OPENHOTELSERVER_DEFAULT_FILENAME;
	std::string sLogFile = OPENHOTELSERVER_LOGFILE;
	std::string sRegisteredClientsFile = OPENHOTELSERVER_REGISTEREDUSERFILE;
	int opt = -1;
	while ((opt = getopt(argc, argv, "h?p:f:l:u:")) != -1) {
		switch (opt) {

		case 'h':
		case '?':
			/* Print help */
			puts(OPENHOTELSERVER_HELP_STRING);
			exit(EXIT_SUCCESS); 

		case 'p':
			/* Change listen port used by server */
			serverListenPort = atoi(optarg);
			printf("Listen port changed to %i\n", serverListenPort);
			break;

		case 'l':
			/* Change log file used by server */
			sLogFile = optarg;
			printf("Log file set to '%s'\n", optarg);
			break;

		case 'u':
			/* Change user file used by server */
			sRegisteredClientsFile = optarg;
			printf("Registered user file set to '%s'\n", optarg);
			break;

		case 'c':
			/* Change max client number on server */
			serverMaxClients = atoi(optarg);
			if (serverMaxClients < OPENHOTELSERVER_MIN_MAXCLIENTS || serverMaxClients > OPENHOTELSERVER_MAX_MAXCLIENTS) {
				printf("MaxClients specified '%i' is out of range [%i, %i], ignoring.\n", serverMaxClients, OPENHOTELSERVER_MIN_MAXCLIENTS, OPENHOTELSERVER_MAX_MAXCLIENTS);
				serverMaxClients = OPENHOTELSERVER_DEFAULT_MAXCLIENTS;
			} else {
				printf("MaxClients changed to %i\n");
			}
			break;

		case 'f':
			/* Change hotel file used by server */
			serverHotelFile = optarg;
			printf("Hotel file changed to '%s'\n", serverHotelFile.c_str());
			break;

		default:
			printf("Unhandled argument character '%c'\n", opt);
		}
	}

	/* Init debug logger */
	CDebugLogger::Instance()->SetLogFile((char*)sLogFile.c_str());

	/* Init hotel server */
	CHotelServer hotel(serverMaxClients, serverListenPort, serverHotelFile, sRegisteredClientsFile);

	/* Load hotel */
	if (!hotel.Initialise()) {
		CDebugLogger::LogFatal("Unable to run server - Hotel initialisation failed.\n");
		exit(EXIT_FAILURE);
	}

	/* Start accepting new connections */
	hotel.StartListen();

	/* Command input thread */
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)cmd_thread, 0, 0, 0);
	 
	/* Main loop */
	bool bRunning = true;
	while (bRunning) {
		/* Update hotel networking */
		hotel.Update();

		/* Parse command */
		if (g_CmdMutex.try_lock()) {

			if (g_bParseCmd) {
				/* Find command index */
				int iCmdIndex = -1; 
				for (int i = 0; i < CMD_COUNT; i++) {
					if (strncmp(g_LastCmd, g_CmdList[i], strlen(g_CmdList[i])) == 0) {
						iCmdIndex = i;
						break;
					}
				}
				
				/* Do command actions */
				switch (iCmdIndex) {
				case CMD_QUIT_INDEX:
				case CMD_EXIT_INDEX:
					bRunning = false;
					break;

				case CMD_LIST_INDEX:
					hotel.ListPlayers();
					break;
				case CMD_REGISTER_INDEX:
					break;

				case CMD_KICK_INDEX:
					{
					/* Find Arg */
					char* pSpacePos = strchr(g_LastCmd, ' ');
					if (pSpacePos) { 

						//hotel.KickPlayer();
					} else {
						CDebugLogger::LogWarning("kickplayer command supplied with no arg\n");
					}
					break;
					}

				case CMD_HELP_INDEX:
				default:
					CDebugLogger::LogWarning("Warning - Ignored invalid command '%s' which was sent to main thread\n", g_LastCmd);
				}
				g_bParseCmd = false;
			}
			g_CmdMutex.unlock();

		}
	}

	hotel.Shutdown();

	return 0;
}

int cmd_thread() {
	bool bExitCmd = false;
	std::string line;

	while (!bExitCmd) {
		printf(">");

		/* Read command */
		line = "";
		std::getline(std::cin, line);

		/* Ignore enter key etc */
		if (line.length() <= 2)
			continue;

		/* Convert to lowercase */
		int len = line.length();
		for (int i = 0; i < len; i++)
			line[i] = (char)tolower((int)line[i]);

		g_CmdMutex.lock();

		/* Check if command matches a valid cmd */
		bool bValid = false;	/* Command is a valid cmd */
		bool bIsHelp = false;	/* Command is a help cmd */
		bool bShouldExit = false;/* Command is exit/quit */
		for (int i = 0; i < CMD_COUNT; i++) {
			if (strncmp(line.c_str(), g_CmdList[i], strlen(g_CmdList[i])) == 0) {
				bValid = true;
				bIsHelp = (i == CMD_HELP_INDEX);
				bShouldExit = (i == CMD_EXIT_INDEX || i == CMD_QUIT_INDEX);
				break;
			}
		}

		/* Copy command to command buffer */
		if (bValid && !bIsHelp) {
			memset(g_LastCmd, 0, CMD_BUFFER_SIZE);
			memcpy(g_LastCmd, line.c_str(), len);
			g_bParseCmd = true;
		}

		g_CmdMutex.unlock();

		/* If help command, print help */
		if (bIsHelp) {
			printf(CMD_HELP_STRING);
		}
		else if (bShouldExit) {
			bExitCmd = true;
		} 
		else if (!bValid) {
			/* Prune string to root command */
			for (int i = 0; i < len; i++) {
				if (line[i] == ' ' || line[i] == '\n') {
					line[i] = '\0';
					break;
				}
				if (line[i] == '\0')
					break; 
			}
			/* Print error */
			printf("Unrecognised command '%s', enter 'help' for list of commands.\n", line.c_str());
		}
	}
	return 0;
}
 