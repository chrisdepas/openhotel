#ifndef __CHOTELCLIENT_H__
#define __CHOTELCLIENT_H__

/*
	Christopher De Pasquale
	18 December 2016
*/


#include <vector>
#include "SFML\Network.hpp"
#include "OHPacket.h"
#include "Hotel.h"
#include "ClientModes.h"
#define SERVERLIST_IP "127.0.0.1"
#define SERVERLIST_PORT 6343

#define CLIENT_CONNECT_TIMEOUT_SECONDS 6.0f

class CHotelClient {
public:
	struct SClientServerlistItem;

private:
	/* Clock */
	sf::Clock m_Clock;

	/* List of servers loaded from OpenHotelServerList */
	std::vector<SClientServerlistItem> m_Serverlist;

	/* Connection to server */
	sf::TcpSocket m_ClientSocket;

	/* Current loaded room */
	CHotelRoom m_HotelRoom;
	bool m_bRoomDataLoaded = false;

	/* Current client mode */
	EClientMode m_eCurrentMode = EClientMode::MODE_WALK;

	/* Current position */
	sf::Vector2f m_vPosition;

	/* walking speed in tiles/second */
	const float WALK_SPEED = 4.0f;

public:
	struct SClientServerlistItem {
		std::string szServerName;
		sf::Uint32 uiPort;
		sf::IpAddress addr;
	};

	/* Get player position */
	sf::Vector2f GetLocalPosition() { return m_vPosition; }

	/* Connects to serverlist & gets hotel superlist */
	std::vector<SClientServerlistItem>& GetServerList();

	/* Room */
	bool HasRoomData() { return m_bRoomDataLoaded; }

	/* Get current mode */
	EClientMode GetCurrentMode() { return m_eCurrentMode; }

	/* Checks if 1:In edit mode 2:Has permission to edit current room */
	bool CanEditCurrentRoom();

	/* Connect to a hotel */
	bool Connect(sf::IpAddress addr, sf::Uint32 port);

	/* Check connection status */
	bool IsConnected();

	/* Update connection to server */
	void Update(float fElapsedSeconds);

	/* Send a packet to the server */
	bool SendPacket(OHPacket& packet);

	/* Resets frame time clock */
	void InitClock() { m_Clock.restart(); }
	/* Gets time elapsed since last frame in seconds */
	float GetElapsedSeconds() { return m_Clock.restart().asSeconds(); }

	/* Packet handlers */
	void OnServerInfoResponse(OHPacket& packet);
	void OnServerRoomRequestResponse(OHPacket& packet);
	void OnServerRoomData(OHPacket& packet);
	void OnServerBalanceUpdate(OHPacket& packet);
	void OnServerChatUpdate(OHPacket& packet);
	void OnServerModeResponse(OHPacket& packet);
	void OnServerTileUpdate(OHPacket& packet);

	/* Gracefully close client */
	void Shutdown();

	CHotelRoom* GetCurrentRoom() { if (HasRoomData()) return &m_HotelRoom; return NULL; }
};
#endif