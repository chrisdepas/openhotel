/*
	Christopher De Pasquale
	18 December 2016
*/
#include "stdafx.h"
#include "CHotelServer.h"
#include "CDebugLogger.h"
#include "OHPacket.h"
#include "OHNetwork.h"
#include "ClientModes.h"
#include "rapidxml-1.13\rapidxml.hpp"
#include <sstream>

CHotelServer::CHotelServer(int iMaxConnections, sf::Uint32 listenPort, std::string sHotelFile, 
	std::string sRegisteredClientsFile) {
	m_sRegisteredClientsFile = sRegisteredClientsFile;
	m_sHotelFile = sHotelFile;
	m_iMaxConnections = iMaxConnections;
	m_ListenPort = listenPort;
}

CHotelServer::~CHotelServer() {

}

/* INTERNAL */
int CHotelServer::DecryptRegisteredClientsFile(char*& buffer, int len) {
	CDebugLogger::LogWarning("Warning - CHotelServer::DecryptRegisteredClientsFile isn't implemented yet. File is read as plaintext.\n");
	return len;
}
/* INTERNAL */
int CHotelServer::EncryptRegisteredClientsFile(char*& buffer, int len) {
	CDebugLogger::LogWarning("Warning - CHotelServer::EncryptRegisteredClientsFile isn't implemented yet. File will be written in plaintext\n");
	return len;
}

/* INTERNAL */
bool CHotelServer::LoadHotelFromFile() {
	return m_Hotel.LoadHotelFromFile(m_sHotelFile);
}

/* INTERNAL */
bool CHotelServer::SaveHotelToFile() {
	return m_Hotel.SaveHotelToFile(m_sHotelFile);
}

/* INTERNAL */
bool CHotelServer::LoadRegisteredClientsFromFile() {
	if (m_bRegisteredClientsFileRead) {
		CDebugLogger::LogWarning("Unable to read registered clients file '%s', registered clients file has already been read\n", 
			m_sRegisteredClientsFile.c_str());
		return false;
	}
	m_bRegisteredClientsFileRead = true;

	/* Open registered clients file for binary reading */
	FILE* fileRegistered = 0;
	if (fopen_s(&fileRegistered, m_sRegisteredClientsFile.c_str(), "rb") != 0 || !fileRegistered) {
		CDebugLogger::LogWarning("Unable to open registered clients file '%s'\n", m_sRegisteredClientsFile.c_str());
		return false;
	}

	/* Get filesize */
	fseek(fileRegistered, 0, SEEK_END);
	int userFileSize = ftell(fileRegistered);
	rewind(fileRegistered);

	/* Load entire file into memory & close*/
	char* userFileBuf = (char*)malloc(userFileSize + 1);
	userFileBuf[userFileSize] = '\0';
	fread_s(userFileBuf, userFileSize + 1, userFileSize, 1, fileRegistered);
	fclose(fileRegistered);

	/* Decrypt file */
	int fileLen = DecryptRegisteredClientsFile(userFileBuf, userFileSize + 1);

	/* Parse as binary file

		_File structure_
		MagicNo				- x Chars			- string REGISTEREDCLIENTS_MAGICNUMBER
		
		RecordCount			- 4 chars			- int
		
		RecordCount instances of
		[ UsernameLen		- 4 chars			- int
		  Username			- UsernameLen chars - string
		  PasswordHashLen	- 4 chars			- int
		  Password			- PassHashLen chars	- string
		  authFlags			- 4 chars			- int flags ]
	*/

	/* Current file pointer */
	char* pCur = userFileBuf;
	/* Pointer to file end */
	char* pEndBuf = userFileBuf + fileLen;

	/* Validate magic number */
	int magicNoLen = strlen(REGISTEREDCLIENTS_MAGICNUMBER);
	if (fileLen < magicNoLen) {
		/* Validation fail - Can't contain magic number */
		CDebugLogger::LogWarning("Malformed Registered Clients File '%s' - Doesn't contain magic number\n", m_sRegisteredClientsFile.c_str());
		free(userFileBuf);
		return false;
	}
	if (strncmp(pCur, REGISTEREDCLIENTS_MAGICNUMBER, magicNoLen) != 0) {
		/* Validation fail - Invalid magic number */
		CDebugLogger::LogWarning("Malformed Registered Clients File '%s' - Contains incorrect magic number\n", m_sRegisteredClientsFile.c_str());
		free(userFileBuf);
		return false;
	}
	pCur += magicNoLen;

	/* Read RecordCount */
	int recordCount;
	if ((pEndBuf - pCur) < sizeof(int)) {
		/* Validation fail - no recordcount */
		CDebugLogger::LogWarning("Malformed Registered Clients File '%s' - Doesn't contain RecordCount\n", m_sRegisteredClientsFile.c_str());
		free(userFileBuf);
		return false;
	}
	recordCount = *(int*)pCur;
	pCur += sizeof(int);
	if (recordCount < REGISTEREDCLIENTS_RECORDS_MIN || recordCount > REGISTEREDCLIENTS_RECORDS_MAX) {
		/* validation fail - recordcount out of range */
		CDebugLogger::LogWarning("Malformed Registered Clients File '%s' - Record count '%i' out of range [%i, %i]\n", m_sRegisteredClientsFile.c_str(), 
			recordCount, REGISTEREDCLIENTS_RECORDS_MIN, REGISTEREDCLIENTS_RECORDS_MAX);
		free(userFileBuf);
		return false;
	}

	/* Temp list of new clients */
	std::vector<SAuthenticatedClient> registeredClients;

	/* Parse each registered user record */
	bool bReadFail = false;
	char* szFailReason = NULL;
	int iRecordNumber = 0;
	while (pCur < pEndBuf && iRecordNumber < recordCount) {
		SAuthenticatedClient curClient;

		/* Read username len */
		if ((pEndBuf - pCur) < sizeof(int)) {
			bReadFail = true; szFailReason = "EOF Reached before username length";break;
		}
		int usernameLen = *(int*)pCur;
		pCur += sizeof(int);

		/* Validate username len */
		if (usernameLen > CLIENT_USERNAME_MAX_LEN) {
			bReadFail = true; szFailReason = "Username length supplied longer than maximum length"; break;
		}
		
		/* Read username */
		if ((pEndBuf - pCur) < usernameLen) {
			bReadFail = true; szFailReason = "EOF Reached before username string"; break;
		}
		char* usernameBuffer = (char*)malloc(usernameLen + 1);
		usernameBuffer[usernameLen] = '\0';
		memcpy(usernameBuffer, pCur, usernameLen);
		pCur += usernameLen;
		curClient.sUsername = usernameBuffer;
		free(usernameBuffer);
		

		/* Read password hash len */
		if ((pEndBuf - pCur) < sizeof(int)) {
			bReadFail = true; szFailReason = "EOF Reached before password hash length"; break;
		}
		int hashLen = *(int*)pCur;
		pCur += sizeof(int);

		/* Validate password hash len */
		if (hashLen > PASSWORD_HASH_MAX_LEN || hashLen < PASSWORD_HASH_MIN_LEN) {
			bReadFail = true; szFailReason = "Hash length not in valid range"; break;
		}

		/* Read password hash */
		if ((pEndBuf - pCur) < hashLen) {
			bReadFail = true; szFailReason = "EOF Reached before password hash string"; break;
		}
		char* hashBuffer = (char*)malloc(hashLen + 1);
		hashBuffer[hashLen] = '\0';
		memcpy(hashBuffer, pCur, hashLen);
		curClient.sPasswordHash = hashBuffer;
		free(hashBuffer);

		/* Read auth flags */
		if ((pEndBuf - pCur) < sizeof(int)) {
			bReadFail = true; szFailReason = "EOF Reached before client auth flags"; break;
		}
		int authFlags = *(int*)pCur;
		pCur += sizeof(int);

		/* Validate auth flags */
		if ((authFlags & ~SAuthenticatedClient::FL_ALLFLAGS) != 0) {
			bReadFail = true; szFailReason = "Authflags contains invalid set bits"; break;
		}
		curClient.eAuthFlags = authFlags;

		/* Add registered client to temp list */
		registeredClients.push_back(curClient);

		iRecordNumber++;
	}

	/* Free resources */
	free(userFileBuf);

	/* Check if a record read failed */
	if (bReadFail) {
		if (szFailReason) {
			CDebugLogger::LogWarning("Malformed Registered Clients File '%s' - %s\n", m_sRegisteredClientsFile.c_str(), szFailReason);
		}
		return false;
	}
	
	/* Check if correct number of records was read */
	if (recordCount != iRecordNumber) {
		/* validation fail - recordcount out of range */
		CDebugLogger::LogWarning("Malformed Registered Clients File '%s' - Records number (%i) doesn't match records in file (%i)\n", m_sRegisteredClientsFile.c_str(),
			recordCount, iRecordNumber);
		return false;
	}

	/* Add all registered clients */
	for (unsigned int i = 0; i < registeredClients.size(); i++) {
		m_vRegisteredClients.push_back(registeredClients[i]);
	}

	return true;
}
bool CHotelServer::SaveRegisteredClientsToFile() {
	/*
	_File structure_
	MagicNo				- x Chars			- string REGISTEREDCLIENTS_MAGICNUMBER

	RecordCount			- 4 chars			- int

	RecordCount instances of
	[ UsernameLen		- 4 chars			- int
	Username			- UsernameLen chars - string
	PasswordHashLen	- 4 chars			- int
	Password			- PassHashLen chars	- string
	authFlags			- 4 chars			- int flags ]
	*/

	/* Calculate total file size */
	int totalFileSize = strlen(REGISTEREDCLIENTS_MAGICNUMBER) + sizeof(int); /* File MagicNo + RecordCount */
	for (unsigned int i = 0; i < m_vRegisteredClients.size(); i++) {
		/* Usernamelen + username string */
		totalFileSize += sizeof(int) + m_vRegisteredClients[i].sUsername.length();
		/* Password hash len, password hash string, auth flags */
		totalFileSize += sizeof(int) + m_vRegisteredClients[i].sPasswordHash.size() + sizeof(int);
	}

	/* Allocate intermediate buffer for file */
	char* fileBuffer = (char*)malloc(totalFileSize + 1);
	fileBuffer[totalFileSize] = '\0';

	char* pWritePtr = fileBuffer;
	
	/* Write magic number */
	memcpy(pWritePtr, REGISTEREDCLIENTS_MAGICNUMBER, strlen(REGISTEREDCLIENTS_MAGICNUMBER));
	pWritePtr += strlen(REGISTEREDCLIENTS_MAGICNUMBER);

	/* Write record count */
	int registeredClientsCount = (int)m_vRegisteredClients.size();
	memcpy(pWritePtr, &registeredClientsCount, sizeof(int));
	pWritePtr += sizeof(int);
	
	/* Write each record */
	for (unsigned int i = 0; i < m_vRegisteredClients.size(); i++) {
		/* Write username len */
		int usernameLen = (int)m_vRegisteredClients[i].sUsername.size();
		memcpy(pWritePtr, &usernameLen, sizeof(int));
		pWritePtr += sizeof(int);
	
		/* Write username */
		memcpy(pWritePtr, m_vRegisteredClients[i].sUsername.c_str(), usernameLen);
		pWritePtr += usernameLen;

		/* Write password hash len */
		int passwordHashLen = (int)m_vRegisteredClients[i].sPasswordHash.size();
		memcpy(pWritePtr, &passwordHashLen, sizeof(int));
		pWritePtr += sizeof(int);

		/* Write password hash */
		memcpy(pWritePtr, m_vRegisteredClients[i].sPasswordHash.c_str(), passwordHashLen);
		pWritePtr += passwordHashLen;

		/* Write auth flags */
		int authFlags = (int)m_vRegisteredClients[i].eAuthFlags;
		memcpy(pWritePtr, &authFlags, sizeof(int));
		pWritePtr += sizeof(int);
	}

	printf("Bytes written to buffer %i\n", pWritePtr - fileBuffer);

	/* Encrypt file buffer*/
	int fileLen = EncryptRegisteredClientsFile(fileBuffer, totalFileSize+1);

	/* Open registered clients file for binary writing */
	FILE* fileRegistered = 0;
	if (fopen_s(&fileRegistered, m_sRegisteredClientsFile.c_str(), "wb") != 0 || !fileRegistered) {
		CDebugLogger::LogWarning("SaveRegisteredClientsToFile() - Unable to open registered clients file '%s'\n", m_sRegisteredClientsFile.c_str());
		free(fileBuffer);
		return false;
	}

	/* Write encrypted data to file & close */
	int writtenBytes = fwrite(fileBuffer, totalFileSize, 1, fileRegistered);
	printf("Bytes written to file %i\n", writtenBytes);
	fclose(fileRegistered);

	/* Free resources */
	free(fileBuffer);

	return true;
}

/* Internal - authenticate client details */
bool CHotelServer::AuthenticateClient(std::string username, std::string password, int iClientIndex) {
	/* Hash provided password */
	std::string passwordHash;
	HashPassword(password, passwordHash);

	/* Check if username matches a registered user */
	for (unsigned int i = 0; i < m_vRegisteredClients.size(); i++) {

		if (_stricmp(username.c_str(), m_vRegisteredClients[i].sUsername.c_str()) == 0) {

			/* Check password hash */
			if (strcmp(passwordHash.c_str(), m_vRegisteredClients[i].sPasswordHash.c_str()) == 0) {
				/* Password matches, copy permission flags */
				if (m_vRegisteredClients[i].IsModerator())
					m_vClients[i]->SetModeratorFlag();
				if (m_vRegisteredClients[i].IsOperator())
					m_vClients[i]->SetOperatorFlag();

				return true;
			} 
			else {
				/* Wrong password, reject */
				return false;
			}
		}
	}

	/* Check if username matches an already connected user */
	for (unsigned int i = 0; i < m_vClients.size(); i++) {
		/* Skip local client */
		if (i == iClientIndex)
			continue;

		if (!m_vClients[i])
			continue;

		if (_stricmp(username.c_str(), m_vClients[i]->GetUsername().c_str()) == 0) {
			/* Username taken, reject auth */
			return false;
		}
	}

	/* User isn't registered, and name isn't taken, allow auth */
	return true;
}

void CHotelServer::HashPassword(std::string password, std::string& __out_hash) {
	CDebugLogger::LogWarning("Warning - CHotelServer::HashPassword isn't implemented yet. Hash is plaintext password.\n");
}
bool CHotelServer::Initialise() {

	/* Load hotel from saved file */
	if (!LoadHotelFromFile()) {
		CDebugLogger::LogWarning("Hotel not loaded from file. Hotel will be loaded as default\n");

		/* Initialise to default hotel */
		m_Hotel.InitAsDefault();
	}
	else {
		CDebugLogger::LogInfo("Successfully loaded hotel data from file '%s'\n", m_sHotelFile.c_str());
	}

	/* Load list of registered clients */
	if (!LoadRegisteredClientsFromFile()) {
		CDebugLogger::LogWarning("Warning - Registered clients not loaded from file. \n");
	}
	else {
		CDebugLogger::LogInfo("Successfully loaded registered clients from file '%s'\n", m_sRegisteredClientsFile.c_str());
	}

	return true;
}

int CHotelServer::GetLobbyID() {
	return m_Hotel.GetLobbyIndex();
}

void CHotelServer::OnClientInfoUpdate(OHPacket& packet, int clientIndex) {
	OHNetwork::CClientInfoUpdate in(packet);
	if (!in.IsValid())
		return;

	CDebugLogger::LogInfo("Client with index %i has authenticated using %s:%s [TODO: AUTHENTICATE PROPERlY!]\n", clientIndex, in.GetUsername().c_str(), in.GetPassword().c_str());

	/* Run authenticator */
	if (AuthenticateClient(in.GetUsername(), in.GetPassword(), clientIndex)) {
		/* Client is now authenticated */
		m_vClients[clientIndex]->SetAuthenticatedFlag();
		CDebugLogger::LogInfo("Client '%s' (Index %i) Passed authentication. \n", in.GetUsername().c_str(), clientIndex);
	}
	else {
		CDebugLogger::LogInfo("Client with index %i failed authentication Credentials: '%s:%s'\n", clientIndex, in.GetUsername().c_str(), in.GetPassword().c_str());
	}

	/* Save new client data */
	m_vClients[clientIndex]->sUsername = in.GetUsername();
	m_vClients[clientIndex]->iClientUID = GetNextClientUID();

	/* Allocate client UID */
	m_vClients[clientIndex]->iClientUID = GetNextClientUID();

	/* Send info response */
	OHPacket response = OHNetwork::CServerInfoResponse::CreateAcceptPacket(m_vClients[clientIndex]->iClientUID);
	if (!SendPacket(response, clientIndex)) 
		CDebugLogger::LogError("Failed to send InfoResponse packet to client\n");
}
void CHotelServer::OnClientEnterLobbyRequest(OHPacket& packet, int clientIndex) {
	OHNetwork::CClientLobbyRequest in(packet);
	if (!in.IsValid())
		return;

	CDebugLogger::LogInfo("Client with index %i requests to join lobby\n", clientIndex);
	
	if (m_vClients[clientIndex]->IsAuthenticated()) {
		/* Move client into lobby */
		m_vClients[clientIndex]->SetRoomID(GetLobbyID());

		/* Client is authenticated, send accept + room info */
		OHPacket response = OHNetwork::CServerRoomResponse::Create(true);
		SendPacket(response, clientIndex);
	} else {
		/* Client not authenticated, send reject response */
		OHPacket response = OHNetwork::CServerRoomResponse::Create(false, "Not authenticated");
		SendPacket(response, clientIndex);
	}
}
void CHotelServer::OnClientEnterRoomRequest(OHPacket& packet, int clientIndex) {
	CDebugLogger::LogInfo("Server - OnClientEnterRoomRequest not implemented!!!!!!\n", clientIndex);
}
void CHotelServer::OnClientChatMessage(OHPacket& packet, int clientIndex) {
	CDebugLogger::LogInfo("Server - OnClientChatMessage not implemented!!!!!!\n", clientIndex);
}
void CHotelServer::OnClientRequestRoomData(OHPacket& packet, int clientIndex) {
	OHNetwork::CClientRequestRoomData in(packet);
	if (!in.IsValid())
		return;

	OHPacket roomData;
	if (m_vClients[clientIndex]->IsAuthenticated()) {
		/* Find room */
		CHotelRoom clientRoom = m_Hotel.GetHotelRoomByIndex(m_vClients[clientIndex]->iRoomID);

		/* Create info packet */
		roomData = OHNetwork::CServerRoomData::CreateAcceptPacket(clientRoom);

		/* Send info to client */
		SendPacket(roomData, clientIndex);
	}
	else {
		roomData = OHNetwork::CServerRoomData::CreateDenyPacket(std::string("Not authenticated\n"));
		SendPacket(roomData, clientIndex);
	}
	 
}

void CHotelServer::OnClientSetModeRequest(OHPacket& packet, int clientIndex) {
	OHNetwork::CClientSetMode in(packet);
	if (!in.IsValid())
		return;

	OHPacket roomData;
	if (m_vClients[clientIndex]->IsAuthenticated()) {
		CDebugLogger::LogWarning("OnClientSetModeRequest - Proper authentication not implemented!!\n");

		/* Change client mode */
		switch (in.GetClientMode()) {
		case EClientMode::MODE_FROZEN:
			break;
		case EClientMode::MODE_OPERATOREDIT:
			m_vClients[clientIndex]->SetOperatorEditMode();
			break;
		case EClientMode::MODE_WALK:
			m_vClients[clientIndex]->SetWalkMode();
			break;
		}
		
		/* Accept */
		OHPacket accept = OHNetwork::CServerModeResponse::CreateAcceptPacket(in.GetClientMode());
		SendPacket(accept, clientIndex);
	}
	else {
		/* Send deny packet */
		OHPacket deny = OHNetwork::CServerModeResponse::CreateDenyPacket(in.GetClientMode(), "Not authenticated!");
		SendPacket(deny, clientIndex);
	}

}
void CHotelServer::OnClientSetTile(OHPacket& packet, int clientIndex) {
	OHNetwork::CClientSetTile in(packet);
	if (!in.IsValid())
		return;

	if (m_vClients[clientIndex]->IsAuthenticated()) {
		if (m_vClients[clientIndex]->IsInRoom()) {
			CHotelRoom& room = m_Hotel.GetHotelRoomByIndex(m_vClients[clientIndex]->GetRoomID());

			/* Get packet data */
			int x = in.GetTileX();
			int y = in.GetTileY();
			bool bDelete = in.DeleteTile();
			int tileID = !bDelete ? in.GetTileID() : -1;
			
			/* Validate packet data */
			if (!room.TilePositionInBounds(x, y)) {
				CDebugLogger::LogWarning("Recieved ClientSetTile packet with invalid position %i,%i\n", x, y);
				return;
			}
			if (!bDelete) {
				if (tileID <= TileID::TILE_BLANK || tileID >= TileID::TILE_LAST) {
					CDebugLogger::LogWarning("Recieved ClientSetTile packet with invalid tileID '%i'\n", tileID);
					return;
				}
			}

			/* Update map and inform clients */
			if (in.DeleteTile()) {
				room.DeleteTile(x, y);
				printf("Sending tile delete for  %i, %i\n", x,y);
				OHPacket accept = OHNetwork::CServerTileUpdate::CreateDeleteTile(x, y);
				BroadcastPacket(accept);
			}
			else {
				room.SetTile(x, y, (TileID::ETileID)tileID);
				printf("Sending tile set for  %i, %i, %i\n", x, y, tileID);
				OHPacket accept = OHNetwork::CServerTileUpdate::CreateSetTile(x, y, tileID);
				BroadcastPacket(accept);
			}
			
		}
		else {
			CDebugLogger::LogWarning("Recieved ClientSetTile packet from client %i who isn't in a room, ignoring\n", clientIndex);
			return;
		}
	}

}


void CHotelServer::PollClients() {
	for (unsigned int i = 0; i < m_vClients.size(); i++) {
		if (m_vClients[i]->clientSocket.getRemoteAddress() == sf::IpAddress::None) {
			/* Client disconnected, remove */
			m_vClients.erase(m_vClients.begin() + i);

			CDebugLogger::LogInfo("Client with index %i disconnected.\n", i);

			/* 'Rewind' i */
			i--;
		}
	}
}
bool CHotelServer::StartListen() {
	m_bIsListening = true;

	/* Set next client sock to non-blocking */
	if (m_NextClient == NULL) {
		m_NextClient = new SConnectedClient();
	}
	m_NextClient->clientSocket.setBlocking(false);

	/* Set listen socket to non-blocking*/
	m_ListenSocket.setBlocking(false);

	/* Listen for connections */
	sf::Socket::Status status = m_ListenSocket.listen(m_ListenPort);
	while (status == sf::Socket::NotReady) {
		sf::Socket::Status status = m_ListenSocket.listen(m_ListenPort);
	}

	if (status != sf::Socket::Done) {
		printf("ERROR - Unable to listen on port %i\n", m_ListenPort);
		return false;
	}

	return true;
}
bool CHotelServer::StopListen() {
	m_bIsListening = false;
	m_ListenSocket.close();
	return true;
}

void CHotelServer::Update() {
	/* If listening, accept new connections */
	if (m_bIsListening) {
		if (m_ListenSocket.accept(m_NextClient->clientSocket) == sf::Socket::Done) {
			CDebugLogger::LogInfo("Accepted connection from %s\n", m_NextClient->clientSocket.getRemoteAddress().toString().c_str());

			/* Move to conected clients */
			m_vClients.push_back(m_NextClient);

			/* Next socket */
			if ((int)m_vClients.size() > m_iMaxConnections) {
				CDebugLogger::LogWarning("Unable to continue listening - No free sockets left (Max connections reached)\n");
				m_bIsListening = false;
				m_NextClient = NULL;
				return;
			} else {
				m_NextClient = new SConnectedClient();
				m_NextClient->clientSocket.setBlocking(false);
			}
		}
	}	

	/* Read data from connected clients */
	OHPacket recvPacket;
	for (unsigned int i = 0; i < m_vClients.size(); i++) {
		if (m_vClients[i]->clientSocket.receive(recvPacket) == sf::Socket::Done) {

			/* Manually extract packet ID */
			sf::Int32 packetID;
			recvPacket >> packetID;
			recvPacket.SetID(packetID);

			/* Send to packet handler */
			switch (recvPacket.GetID()) {

			case OHPacket::ID_NONE:
			case OHPacket::ID_NOTSET:
				/* Invalid ID */
				break;

			case OHPacket::ID_CLIENT_INFO_UPDATE:
				OnClientInfoUpdate(recvPacket, i);
				break;

			case OHPacket::ID_CLIENT_ENTER_LOBBY:
				OnClientEnterLobbyRequest(recvPacket, i);
				break;

			case OHPacket::ID_CLIENT_ENTER_ROOM:
				OnClientEnterRoomRequest(recvPacket, i);
				break;

			case OHPacket::ID_CLIENT_CHAT_MESSAGE:
				OnClientChatMessage(recvPacket, i);
				break;

			case OHPacket::ID_CLIENT_REQUEST_ROOM_DATA:
				OnClientRequestRoomData(recvPacket, i); 

			case OHPacket::ID_CLIENT_SET_MODE:
				OnClientSetModeRequest(recvPacket, i);
				break;

			case OHPacket::ID_CLIENT_SET_TILE:
				OnClientSetTile(recvPacket, i);
				break;
			}
		}
	}

	/* Remove disconnected clients */
	PollClients();// Think this is broken
}

bool CHotelServer::SendPacket(OHPacket& packet, int iClientIndex) {
	return m_vClients[iClientIndex]->clientSocket.send(packet) == sf::Socket::Done;
}
bool CHotelServer::BroadcastPacket(OHPacket& packet, int iSkipIndex) {
	for (unsigned int i = 0; i < m_vClients.size(); i++) {
		if ((int)i == iSkipIndex)
			continue;
		if (m_vClients[i]->clientSocket.send(packet) != sf::Socket::Done) {
			CDebugLogger::LogError("Failed to send packet to client with ID %i\n", i);
		}
	}
	return true;
}

void CHotelServer::Shutdown() {
	/* Save hotel to file */
	CDebugLogger::LogError("Saving hotel to file.\n");
	if (!SaveHotelToFile()) {
		CDebugLogger::LogError("Error - Couldn't save hotel to file\n");
	}

	/* Save registered users to file */
	CDebugLogger::LogError("Hotel saved. Saving registered clients\n");
	if (!SaveRegisteredClientsToFile()) {
		CDebugLogger::LogError("Error - Couldn't save registered clients to file\n");
	}
	CDebugLogger::LogError("Registered clients saved. Disconnecting sockets.\n");

	/* Close all sockets & free resources */
	for (unsigned int i = 0; i < m_vClients.size(); i++) {
		m_vClients[i]->clientSocket.disconnect();
		delete m_vClients[i];
	}
	if (m_NextClient != NULL)
		delete m_NextClient;	
	CDebugLogger::LogError("Shutdown complete.\n");
}


void CHotelServer::ListPlayers() {
	printf("ID\tNAME\tROOM ID\tFLAGS\n");
	for (unsigned int i = 0; i < m_vClients.size(); i++) {
		/* Print client info */
		printf("%i\t%s\t%i\t%c%c%c\n", m_vClients[i]->GetUID(), m_vClients[i]->GetUsername().c_str(), m_vClients[i]->GetRoomID(),
			m_vClients[i]->IsAuthenticated() ? 'A' : '-', m_vClients[i]->IsModerator() ? 'M' : '-',
			m_vClients[i]->IsOperator() ? 'O' : '-');
	}
	printf("-----------------------------------\n");
}

void CHotelServer::KickPlayer(int iID) {
	printf("KicKPlayer not implemented - Trying to kick ID %i\n", iID);
}