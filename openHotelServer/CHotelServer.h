#ifndef __CHOTELSERVER_H__
#define __CHOTELSERVER_H__

#include <string>
#include "Hotel.h"
#include "SFML\Network.hpp"
#include "OHPacket.h"

/*
	Christopher De Pasquale
		18 December 2016
*/



class CHotelServer {

	/************		SAuthenticatedClient     ******************/
	struct SAuthenticatedClient {
		std::string sUsername;
		std::string sPasswordHash;
		int eAuthFlags;

		enum EAuthenticatedClientFlags {
			FL_OPERATOR		= 1 << 0, /* Server operator, can do anything */
			FL_MODERATOR	= 1 << 1, /* Server mod, kick/ban/mute */

			FL_ALLFLAGS		= FL_OPERATOR | FL_MODERATOR, /* All valid bit flags set, used to validate authflag field*/
		};

		bool IsOperator()				{ return (eAuthFlags & FL_OPERATOR) != 0; }
		bool IsModerator()				{ return (eAuthFlags & FL_MODERATOR) != 0; }

		void SetOperatorFlag()			{ eAuthFlags |= FL_OPERATOR; }
		void SetModeratorFlag()			{ eAuthFlags |= FL_MODERATOR; }

		void RemoveOperatorFlag()		{ eAuthFlags &= ~FL_OPERATOR; }
		void RemoveModeratorFlag()		{ eAuthFlags &= ~FL_MODERATOR; }
	};
	/**************************************************************/


	/************		SConnectedClient     **********************/
	struct SConnectedClient {
	private:
		enum EConnectedClientFlags {
			/* Flags related to permissions */
			FL_OPERATOR			= 1 << 0,	/* Server operator, can do everything */
			FL_AUTHENTICATED	= 1 << 1,	/* Authenticated, set if client can join/listen */
			FL_MODERATOR		= 1 << 2,	/* Server moderator, can kick/ban/mute */

			/* Flags related to client state */
			FL_OP_EDITMODE		= 1 << 3,	/* Player is is operator edit mode, allowed to modify tiles, place furniture  
											in non-resident rooms, for free */
			FL_WALKMODE			= 1 << 4,

		};

	public:
		/* connection socket to client */
		sf::TcpSocket clientSocket;

		/* username */
		std::string sUsername;

		/* client flags */
		int eClientFlags = 0;

		/* Unique client ID - known by server & clients */
		int iClientUID; 

		/* Client Room ID */
		bool bInRoom = false;
		int iRoomID;

		SConnectedClient(int _iClientUID, std::string& _username) {
			iClientUID = _iClientUID;
			sUsername = _username;
		}
		SConnectedClient() {}

		/* Check client permissions */
		bool IsOperator()				{ return (eClientFlags & FL_OPERATOR) != 0; }
		bool IsAuthenticated()			{ return (eClientFlags & FL_AUTHENTICATED) != 0; }
		bool IsModerator()				{ return (eClientFlags & FL_MODERATOR) != 0; }
		/* Set client permissions */
		void SetOperatorFlag()			{ eClientFlags |= FL_OPERATOR; }
		void SetAuthenticatedFlag()		{ eClientFlags |= FL_AUTHENTICATED; }
		void SetModeratorFlag()			{ eClientFlags |= FL_MODERATOR; }
		/* Remove client permissions */
		void RemoveOperatorFlag()		{ eClientFlags &= ~FL_OPERATOR; }
		void RemoveAuthenticatedFlag()	{ eClientFlags &= ~FL_AUTHENTICATED; }
		void RemoveModeratorFlag()		{ eClientFlags &= ~FL_MODERATOR; }

		/* Check client state */
		bool IsInOperatorEditMode()		{ return (eClientFlags & FL_OP_EDITMODE) != 0; }
		/* Set client state */
		void SetOperatorEditMode()		{ RemoveOperatorWalkMode(); eClientFlags |= FL_OP_EDITMODE; }
		void SetWalkMode()				{ RemoveOperatorEditMode(); eClientFlags |= FL_WALKMODE; }
		/* Remove client state */
		void RemoveOperatorEditMode()	{ eClientFlags &= ~FL_OP_EDITMODE; }
		void RemoveOperatorWalkMode()	{ eClientFlags &= ~FL_WALKMODE; }

		/* Room */
		bool IsInRoom() { return bInRoom; }
		int GetRoomID() { return iRoomID; }
		void SetRoomID(int roomID) { bInRoom = true; iRoomID = roomID; }
	
		/* Client Unique ID */
		int GetUID() { return iClientUID; }
		void SetUID(int iUID) { iClientUID = iUID; }

		/* Username */
		std::string& GetUsername() { return sUsername; }
	};
	/**************************************************************/

	/* Last unique client ID given */
	int m_iLastClientUID = 0;

	/* If hotel is loaded & ready to go */
	bool m_bIsInitialised = false;

	/* Filename for hotel */
	std::string m_sHotelFile;

	/* Filename for registered clients list */
	std::string m_sRegisteredClientsFile;
	/* If registered clients file has been read */
	bool m_bRegisteredClientsFileRead = false;

	/* Hotel / hotel rooms */
	CHotel m_Hotel;

	/* Max connections (Number of sockets in m_ClientSockets) */
	int m_iMaxConnections;
	/* Port to listen on */
	sf::Uint32 m_ListenPort;
	/* Listening socket */
	sf::TcpListener m_ListenSocket;

	/* Connected clients */
	std::vector<SConnectedClient*> m_vClients;
	SConnectedClient* m_NextClient = NULL;
	/* Registered clients list */
	std::vector<SAuthenticatedClient> m_vRegisteredClients;

	/* Is server currently listening */
	bool m_bIsListening = false;

	/* Packet handlers */
	void OnClientInfoUpdate(OHPacket& packet, int clientIndex);
	void OnClientEnterLobbyRequest(OHPacket& packet, int clientIndex);
	void OnClientEnterRoomRequest(OHPacket& packet, int clientIndex);
	void OnClientChatMessage(OHPacket& packet, int clientIndex);
	void OnClientRequestRoomData(OHPacket& packet, int clientIndex);
	void OnClientSetModeRequest(OHPacket& packet, int clientIndex);
	void OnClientSetTile(OHPacket& packet, int clientIndex);

	/* Internal - Decrypts registered clients file buffer */
	int DecryptRegisteredClientsFile(char*& buffer, int len);
	/* Internal - Encrypt registered clients file buffer */
	int EncryptRegisteredClientsFile(char*& buffer, int len);

	/* Internal - Loads hotel from saved filename */
	bool LoadHotelFromFile();
	/* Internal - Save hotel to saved filename */
	bool SaveHotelToFile();

	/* Internal - Loads list of registered clients from saved filename */
	bool LoadRegisteredClientsFromFile();
	/* Internal - Save list of registered users to saved filename */
	bool SaveRegisteredClientsToFile();

	/* Internal - Get next unused client UID */
	int GetNextClientUID() { return ++m_iLastClientUID; }

	/* Internal - authenticate client details, set mod/OP flags, returns success/fail bool */
	bool AuthenticateClient(std::string Username, std::string Password, int iClientIndex);
	/* Internal - hashes password */
	void HashPassword(std::string password, std::string& __out_hash);

	/* Magic number used to identify Registered Clients file */
	const char* REGISTEREDCLIENTS_MAGICNUMBER = "SN4KEB00t";

	/* Max accepted range of registered client record counts, inclusive */
	const int REGISTEREDCLIENTS_RECORDS_MIN = 0;
	const int REGISTEREDCLIENTS_RECORDS_MAX = 5000;

	/* Max accepted range for username length in Registered Clients file */
	const int CLIENT_USERNAME_MIN_LEN = 3;
	const int CLIENT_USERNAME_MAX_LEN = 20;

	/* Max accepted range for password hash length in Registered Clients file */
	const int PASSWORD_HASH_MAX_LEN = 512;
	const int PASSWORD_HASH_MIN_LEN = 1;
	
	/* Don't implement */
	CHotelServer();

public:
	CHotelServer(int iMaxConnections, sf::Uint32 listenPort, std::string sSavedHotelFilename, 
		std::string sRegisteredClientsFile);
	~CHotelServer();

	/* Checks clients for disconnects */
	void PollClients();

	/* Network update */
	void Update();

	/* Start listening for clients */
	bool StartListen();
	
	/* Stop accepting new clients (Doesn't shutdown server) */
	bool StopListen();

	/* Initialise hotel */
	bool Initialise();

	/* Get ID of lobby room */
	int GetLobbyID();
	
	/* Send a packet to client by clientIndex */
	bool SendPacket(OHPacket& packet, int iClientIndex);
	/* Broadcast packet to all clients, optionally skipping one by index*/
	bool BroadcastPacket(OHPacket& packet, int iSkipIndex = -1);

	/* Gracefully shutdown server */
	void Shutdown();


	/* Console cmd helper - list all players w/ id in console */
	void ListPlayers();
	/* Console cmd helper - kick player by ID */
	void KickPlayer(int iID);
};


#endif