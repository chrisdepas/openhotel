/*
	Christopher De Pasquale
	18 December 2016
*/

#include "stdafx.h"
#include "CHotelClient.h"
#include "CDebugLogger.h"
#include "SFML\System.hpp"
#include "OHPacket.h"
#include "OHNetwork.h"
#include "SFML\Window.hpp"

std::vector<CHotelClient::SClientServerlistItem>& CHotelClient::GetServerList() {
	sf::TcpSocket listSocket;
	if (listSocket.connect(SERVERLIST_IP, SERVERLIST_PORT) != sf::Socket::Done) {
		CDebugLogger::LogError("Error - Unable to connect to serverlist\n");
		return m_Serverlist;
	}

	/* Connection succeeded, recieve serverlist */
	while (true) {
		sf::Packet recvPacket;
		if (listSocket.receive(recvPacket) != sf::Socket::Done) {
			break;
		}

		/* Recieve data */
		CHotelClient::SClientServerlistItem newServerItem; 
		sf::Uint32 uiAddr;
		recvPacket >> newServerItem.szServerName;
		recvPacket >> uiAddr;
		recvPacket >> newServerItem.uiPort;
		newServerItem.addr = sf::IpAddress((sf::Uint32)uiAddr);
		m_Serverlist.push_back(newServerItem);
	}

	return m_Serverlist;
}	

bool CHotelClient::CanEditCurrentRoom() {
	if (m_HotelRoom.GetType() == CHotelRoom::ROOMTYPE_LOBBY ||
		m_HotelRoom.GetType() == CHotelRoom::ROOMTYPE_HALL) {
		return GetCurrentMode() == EClientMode::MODE_OPERATOREDIT;
	}
	else if (m_HotelRoom.GetType() == CHotelRoom::ROOMTYPE_RESIDENT) {
		static int err = 0;
		if (err == 0) 
			CDebugLogger::LogWarning("Warning - CanEditCurrentRoom not implemented for resident rooms\n");
		err = 1;

		return false;
	}

	return false;
}

bool CHotelClient::Connect(sf::IpAddress addr, sf::Uint32 port) {
	m_ClientSocket.setBlocking(false);
	sf::Time time;

	sf::Clock clock;
	clock.restart();

	/* Connect until failure/success */
	sf::Socket::Status status = m_ClientSocket.connect(addr, port);
	while (status == sf::Socket::NotReady) {
		status = m_ClientSocket.connect(addr, port);

		if (clock.getElapsedTime().asSeconds() > CLIENT_CONNECT_TIMEOUT_SECONDS) {
			CDebugLogger::LogInfo("Unable to connect - Connection timed out.\n");
			return false;
		}
	}

	if (status != sf::Socket::Done) {
		CDebugLogger::LogWarning("Unable to reach hotel at %s:%u\n", addr.toString().c_str(), port);
		return false;
	}

	return true;
}

bool CHotelClient::IsConnected() {
	return m_ClientSocket.getRemoteAddress() != sf::IpAddress::None;
}

void CHotelClient::Update(float fElapsedTime) {
	/* Handle packets from server */
	OHPacket recvPacket;
	sf::Socket::Status status;
	while ((status = m_ClientSocket.receive(recvPacket)) == sf::Socket::Done) {

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

		case OHPacket::ID_SERVER_INFO_RESPONSE:
			OnServerInfoResponse(recvPacket);
			break;

		case OHPacket::ID_SERVER_ROOM_REQUEST_RESPONSE:
			OnServerRoomRequestResponse(recvPacket);
			break;

		case OHPacket::ID_SERVER_ROOM_DATA:
			OnServerRoomData(recvPacket);
			break;

		case OHPacket::ID_SERVER_BALANCE_UPDATE:
			OnServerBalanceUpdate(recvPacket);
			break;

		case OHPacket::ID_SERVER_CHAT_UPDATE:
			OnServerChatUpdate(recvPacket);
			break;

		case OHPacket::ID_SERVER_MODE_RESPONSE:
			OnServerModeResponse(recvPacket);
			break;

		case OHPacket::ID_SERVER_TILE_UPDATE:
			OnServerTileUpdate(recvPacket);
			break;
		}
	}

	/* Handle movement */
	sf::Vector2f moveDelta(0.0f, 0.0f);
	bool bMove = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		moveDelta.x -= 1.0f;
		moveDelta.y -= 1.0f; 
		bMove = true;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		moveDelta.x += 1.0f;
		moveDelta.y += 1.0f;
		bMove = true;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		moveDelta.x += 1.0f;
		moveDelta.y -= 1.0f;
		bMove = true;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		moveDelta.x -= 1.0f;
		moveDelta.y += 1.0f;
		bMove = true;
	}
	if (bMove && (moveDelta.x*moveDelta.x >= 1.0f || moveDelta.y*moveDelta.y >= 1.0f)) {
		/* Unit vector - Get direction */
		float len = (float)sqrt(moveDelta.x * moveDelta.x + moveDelta.y * moveDelta.y);
		moveDelta /= len;
		if (moveDelta.x*moveDelta.x < 0.1f) {
	//		moveDelta.y *= 2.0f;
		}
		/* Calculate frame movement */
		moveDelta *= fElapsedTime * WALK_SPEED;
		m_vPosition += moveDelta;

		printf("Move delta %f %f\n", moveDelta.x, moveDelta.y);
	}
}
bool CHotelClient::SendPacket(OHPacket& packet) {
	return m_ClientSocket.send(packet) == sf::Socket::Done;
}
void CHotelClient::OnServerInfoResponse(OHPacket& packet) {
	OHNetwork::CServerInfoResponse in(packet);
	if (!in.IsValid())
		return;

	if (!in.GetAuthAccepted()) {
		CDebugLogger::LogWarning("Authentication failed. Reason: %s\n", in.GetAuthFailReason().c_str());

		/* TODO: Fail condition here which disconnects or something */
	}
	else {
		CDebugLogger::LogInfo("Authentication accepted, requesting to join lobby.\n");

		/* Send a request to join hotel lobby */
		OHPacket lobbyRequest = OHNetwork::CClientLobbyRequest::Create();
		SendPacket(lobbyRequest);
	}
}

void CHotelClient::OnServerRoomRequestResponse(OHPacket& packet) {
	OHNetwork::CServerRoomResponse in(packet);
	if (!in.IsValid())
		return;

	if (in.GetRequestAccepted()) {
		printf("Room join accepted, requesting room data.\n");
		/* Send RequestRoomData packet */
		OHPacket dataRequest = OHNetwork::CClientRequestRoomData::Create();
		SendPacket(dataRequest);
	}
	else {
		printf("Room join declined. Reason: %s\n", in.GetFailReason().c_str());
	}
}

void CHotelClient::OnServerRoomData(OHPacket& packet) {
	OHNetwork::CServerRoomData in(packet);
	if (!in.IsValid())
		return;

	/* Check if room data request was denied */
	if (!in.GetRequestAccepted()) {
		CDebugLogger::LogWarning("Request for room data failed. Reason: %s\n", in.GetFailReason().c_str());
		return;
	}

	/* Free resources */
	if (m_bRoomDataLoaded) {
		m_HotelRoom.Cleanup();
	}

	/* Load room from packet */
	puts("loading room data from server");
	m_bRoomDataLoaded = in.ExtractRoomData(packet, m_HotelRoom);

	/* Init position from packet */
	m_vPosition = in.GetEnterPosition();
	
	if (!m_bRoomDataLoaded) {
		CDebugLogger::LogError("ERROR - Failed to initialise current room from network\n"); 
	}
	else {
		printf("loaded room from server, name is %s, dimensions (%i, %i)\n", m_HotelRoom.GetName().c_str(), m_HotelRoom.GetWidth(), m_HotelRoom.GetHeight());
	}
}
void CHotelClient::OnServerBalanceUpdate(OHPacket& packet) {
	OHNetwork::CServerBalanceUpdate in(packet);
	if (!in.IsValid())
		return;
	CDebugLogger::LogInfo("Unhandled packet CServerBalanceUpdate\n");
}
void CHotelClient::OnServerChatUpdate(OHPacket& packet) {
	OHNetwork::CServerChatUpdate in(packet);
	if (!in.IsValid())
		return;
	CDebugLogger::LogInfo("Unhandled packet CServerChatUpdate\n");
}
void CHotelClient::OnServerModeResponse(OHPacket& packet) {
	OHNetwork::CServerModeResponse in(packet);
	if (!in.IsValid())
		return;
	
	if (in.GetRequestAccepted()) {
		/* Validate clientMode */
		int clientMode = in.GetClientMode();
		if (clientMode <= MODE_FIRST || clientMode >= MODE_LAST) {
			CDebugLogger::LogError("Invalid clientmode value '%i' received on ServerModeResponse, ignoring.\n", clientMode); 
			return;
		}

		/* Set mode locally */
		m_eCurrentMode = (EClientMode)clientMode;
	}
	else {
		CDebugLogger::LogInfo("Client mode change request denied. Reason: %s\n", in.GetFailReason().c_str());
	}
}

void CHotelClient::OnServerTileUpdate(OHPacket& packet) {
	OHNetwork::CServerTileUpdate in(packet);
	if (!in.IsValid())
		return;

	/* Get packet data */
	int x = in.GetTileX();
	int y = in.GetTileY();
	bool bDelete = in.DeleteTile();
	int tileID = !bDelete ? in.GetTileID() : -1;
	printf("ServerTileUpdate %i, %i, delete? %s, id %i\n", x, y, bDelete ? "ye" : "na", tileID);

	/* Validate packet data */
	if (!m_HotelRoom.TilePositionInBounds(x, y)) {
		CDebugLogger::LogWarning("Recieved CServerTileUpdate packet with invalid position %i,%i. Client may be out of sync.\n", x, y);
		return; 
	}
	if (!bDelete) {
		if (tileID <= TileID::TILE_BLANK || tileID >= TileID::TILE_LAST) {
			CDebugLogger::LogWarning("Recieved CServerTileUpdate packet with invalid tileID '%i'. Client may be out of sync.\n", tileID);
			return;
		}
	}

	/* Update room map */
	if (in.DeleteTile()) {
		m_HotelRoom.DeleteTile(x, y);
	}
	else {
		m_HotelRoom.SetTile(x, y, (TileID::ETileID)tileID);
	}
}
void CHotelClient::Shutdown() {
	/* Disconnect socket */
	if (IsConnected())
		m_ClientSocket.disconnect();
}