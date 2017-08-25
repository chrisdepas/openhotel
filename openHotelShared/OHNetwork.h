#include <SFML\Network.hpp>

/*
	Christopher De Pasquale
	18 December 2016
*/
#include "OHPacket.h"

namespace OHNetwork {
	static int VERSION = 2;
	/*
		Base Packet class 
	*/
	class CBasePacket {
	protected:
		bool m_bIsValid;
	public:
		bool IsValid() { return m_bIsValid; }
	};

	/*
		Client Info Update
		Client->Server

		Contains client information 
	*/
	class CClientInfoUpdate : public CBasePacket {
		int m_iNetworkVersion;
		std::string m_szUsername;
		std::string m_szPassword;
	public:
		CClientInfoUpdate(OHPacket& packet) {
			m_bIsValid = false;
			int id = packet.GetID();
			if (id != OHPacket::ID_CLIENT_INFO_UPDATE)
				return;

			packet >> m_iNetworkVersion;
			packet >> m_szUsername;
			packet >> m_szPassword;
			m_bIsValid = (m_szUsername.length() != 0);
		}
		std::string GetUsername() { return m_szUsername; }
		std::string GetPassword() { return m_szPassword; }

		/* Create a ClientInfoUpdate packet */
		static OHPacket Create(char* username, char* password) {
			OHPacket packet;

			packet << (sf::Int32)(OHPacket::ID_CLIENT_INFO_UPDATE);
			packet << OHNetwork::VERSION;
			packet << username;
			packet << password;
			 
			return packet;
		}
	};

	/*
	Server Info Response
	Server->Client

	Contains response to client info (auth) request, & client UID.
	*/
	class CServerInfoResponse : public CBasePacket {
		bool m_bAuthenticationAccepted;
		std::string m_sAuthFailReason;
		int m_iLocalUID;
	public:
		CServerInfoResponse(OHPacket& packet) {
			m_bIsValid = false;
			int id = packet.GetID();
			if (id != OHPacket::ID_SERVER_INFO_RESPONSE)
				return;

			packet >> m_bAuthenticationAccepted;
			if (!m_bAuthenticationAccepted)
				packet >> m_sAuthFailReason;
			else 
				packet >> m_iLocalUID;

			m_bIsValid = true;
		}
		bool GetAuthAccepted() { return m_bAuthenticationAccepted; }
		std::string GetAuthFailReason() { return m_sAuthFailReason; }
		int GetLocalUID() { return m_iLocalUID; }

		/* Create a CServerInfoResponse reject client packet */
		static OHPacket CreateRejectPacket(std::string authFailReason = "Unknown.") {
			OHPacket packet;

			packet << (sf::Int32)(OHPacket::ID_SERVER_INFO_RESPONSE);
			packet << false; /* Reject */
			packet << authFailReason;

			return packet;
		}
		/* Create a CServerInfoResponse accept client packet */
		static OHPacket CreateAcceptPacket(int clientUID) {
			OHPacket packet;

			packet << (sf::Int32)(OHPacket::ID_SERVER_INFO_RESPONSE);
			packet << true; /* Accept */
			packet << clientUID;

			return packet;
		}
	};

	/*
	Client Join Room
	Client->Server

	Client requests to join a hotel room
	*/
	class CClientJoinRoomRequest : public CBasePacket {
	public:
		CClientJoinRoomRequest(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_CLIENT_ENTER_ROOM;
		}

		/* Create a CClientJoinRoomRequest packet */
		static OHPacket Create() {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_CLIENT_ENTER_ROOM);
			return packet;
		}
	};

	/*
	Client Join Lobby
	Client->Server

	Client requests to join hotel lobby
	*/
	class CClientLobbyRequest : public CBasePacket {
	public:
		CClientLobbyRequest(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_CLIENT_ENTER_LOBBY;
		}

		/* Create a CClientLobbyRequest packet */
		static OHPacket Create() {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_CLIENT_ENTER_LOBBY);
			return packet;
		}
	};

	/*
	Server Room Request Response
	Server->Client

	Contains response to client request to join a room
	*/
	class CServerRoomResponse : public CBasePacket {
		bool m_bRequestAccepted;
		std::string m_sFailReason; /* Used if request isn't accepted */
	public:
		CServerRoomResponse(OHPacket& packet) {
			m_bIsValid = false;
			int id = packet.GetID();
			if (id != OHPacket::ID_SERVER_ROOM_REQUEST_RESPONSE)
				return;

			packet >> m_bRequestAccepted;
			if (!m_bRequestAccepted)
				packet >> m_sFailReason;

			m_bIsValid = true;
		}
		bool GetRequestAccepted() { return m_bRequestAccepted; }
		std::string GetFailReason() { return m_sFailReason; }

		/* Create a CServerRoomResponse packet */
		static OHPacket Create(bool requestAccepted, std::string failReason = "") {
			OHPacket packet;
			 
			packet << (sf::Int32)(OHPacket::ID_SERVER_ROOM_REQUEST_RESPONSE);
			packet << requestAccepted;

			if (!requestAccepted)
				packet << failReason;

			return packet;
		}
	};

	/*
	Client Join Room
	Client->Server

	Client requests all data for room they are in
	*/
	class CClientRequestRoomData : public CBasePacket {
	public:
		CClientRequestRoomData(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_CLIENT_REQUEST_ROOM_DATA;
		}

		/* Create a CClientRequestRoomData packet */
		static OHPacket Create() {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_CLIENT_REQUEST_ROOM_DATA);
			return packet;
		}
	};

	/*
	Server Room Data Respones
	Server->Client

	Contains response to client with all data on room
	*/
	class CServerRoomData : public CBasePacket {
		bool m_bRequestAccepted;
		std::string m_sRequestFailReason;/* Used if request isn't accepted */
		bool m_bRoomDataExtracted = false;
		sf::Vector2f m_vEnterPosition;
	public:
		CServerRoomData(OHPacket& packet) {
			m_bIsValid = false;
			int id = packet.GetID();
			if (id != OHPacket::ID_SERVER_ROOM_DATA)
				return;

			packet >> m_bRequestAccepted;
			if (!m_bRequestAccepted) {
				/* Room data request failed, read failreason */
				packet >> m_sRequestFailReason;
			}
			m_bIsValid = true;
		}

		bool GetRequestAccepted() { return m_bRequestAccepted; }
		std::string GetFailReason() { return m_sRequestFailReason; }
		sf::Vector2f GetEnterPosition() { return m_vEnterPosition; }

		bool ExtractRoomData(OHPacket& roomDataPacket, CHotelRoom& __out_room) {
			if (!m_bRequestAccepted)
				return false;

			/* Check if room data already taken */
			if (m_bRoomDataExtracted)
				return false;

			/* Decode data from packet */
			m_bRoomDataExtracted = true;
			__out_room.DecodeFromPacket(roomDataPacket);

			return true;
		}

		/* Create a CServerRoomData deny packet */
		static OHPacket CreateDenyPacket(std::string& sFailReason) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_ROOM_DATA);
			packet << false; /* Deny request for room data */
			packet << sFailReason;

			return packet;
		}
		/* Create a CServerRoomData accept packet */
		static OHPacket CreateAcceptPacket(CHotelRoom& room) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_ROOM_DATA);
			packet << true; /* Accept room data request */
			room.EncodeToPacket(packet);

			return packet;
		}
	};


	/*
	Client Chat Message
	Client->Server

	Client sends a chat message (May not be accepted!)
	*/
	class CClientChatMessage : public CBasePacket {
		std::string m_sChatMessage;
	public:
		CClientChatMessage(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_CLIENT_CHAT_MESSAGE;
			packet >> m_sChatMessage;
		}

		std::string GetChatMessage() {
			return m_sChatMessage;
		}

		/* Create a CClientChatMessage packet */
		static OHPacket Create(std::string sChatMessage) {
			OHPacket packet;
			
			packet << (sf::Int32)(OHPacket::ID_CLIENT_CHAT_MESSAGE);
			packet << sChatMessage;

			return packet;
		}
	};

	/*
	Server Chat Update
	Server->Client

	Contains a client chat update
	*/
	class CServerChatUpdate : public CBasePacket {
		int m_iClientID;
		std::string m_sChatMessage; 
		bool m_bIsPrivateMessage; 
	public:
		CServerChatUpdate(OHPacket& packet) {
			m_bIsValid = false;
			int id = packet.GetID();
			if (id != OHPacket::ID_SERVER_CHAT_UPDATE)
				return;

			packet >> m_iClientID;
			packet >> m_sChatMessage;
			packet >> m_bIsPrivateMessage;

			m_bIsValid = true;
		}
		int GetClientID() { return m_iClientID; }
		bool GetIsPrivate() { return m_bIsPrivateMessage; }
		std::string GetChatMessage() { return m_sChatMessage; }

		/* Create a CServerChatUpdate packet */
		static OHPacket Create(std::string sChatMessage, int iSayClientID, bool bIsPrivate = false) {
			OHPacket packet;
			 
			packet << (sf::Int32)(OHPacket::ID_SERVER_CHAT_UPDATE);
			packet << iSayClientID;
			packet << sChatMessage;
			packet << bIsPrivate;

			return packet;
		}
	};

	/*
	Server Balance Update
	Server->Client

	Contains new $ balance for client
	*/
	class CServerBalanceUpdate : public CBasePacket {
		int m_iNewBalance;
	public:
		CServerBalanceUpdate(OHPacket& packet) {
			if (!(m_bIsValid = (packet.GetID() == OHPacket::ID_SERVER_BALANCE_UPDATE)))
				return;
			packet >> m_iNewBalance;
		}

		int GetBalance() { return m_iNewBalance; }

		/* Create a CServerBalanceUpdate packet */
		static OHPacket Create(int iBalance) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_BALANCE_UPDATE);
			packet << iBalance;
			return packet;
		}
	};

	/*
	Client Set Mode Request
	Client->Server

	Client requests a change to a specific mode
	*/
	class CClientSetMode : public CBasePacket {
		int m_iClientMode;  /* ID of mode to switch to */
	public:
		CClientSetMode(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_CLIENT_SET_MODE;
			packet >> m_iClientMode;
		}

		int GetClientMode()			{ return m_iClientMode; }

		/* Create a CClientSetMode packet */
		static OHPacket Create(int iClientMode) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_CLIENT_SET_MODE);
			packet << iClientMode;
			return packet;
		}
	};

	/*
	Server Mode Request Response
	Server->Client

	Contains a response to a request to change client mode 
	*/
	class CServerModeResponse : public CBasePacket {
		int m_iClientMode;			/* Mode */
		bool m_bSuccess;			/* Server allowed/disallowed request */
		std::string m_sFailReason;	/* Reason request wasnt allowed (not present if m_bSuccess==true)*/
	public:
		CServerModeResponse(OHPacket& packet) {
			m_bIsValid = false;
			int id = packet.GetID();
			if (id != OHPacket::ID_SERVER_MODE_RESPONSE)
				return;

			packet >> m_bSuccess;
			if (!m_bSuccess)
				packet >> m_sFailReason;
			packet >> m_iClientMode;

			m_bIsValid = true;
		}

		int GetClientMode()			{ return m_iClientMode; }
		bool GetRequestAccepted()	{ return m_bSuccess; }
		std::string GetFailReason() { return m_sFailReason; }

		/* Create a CServerModeResponse accept */
		static OHPacket CreateAcceptPacket(int iClientMode) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_MODE_RESPONSE);
			packet << true;
			packet << iClientMode;

			return packet;
		}

		/* Create a CServerModeResponse deny */
		static OHPacket CreateDenyPacket(int iClientMode, std::string sDenyReason) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_MODE_RESPONSE);
			packet << false;
			packet << sDenyReason;
			packet << iClientMode;

			return packet;
		}
	};

	/*
	Client Set Tile request 
	Client->Server

	Client requests tile change
	*/
	class CClientSetTile : public CBasePacket {
		int m_iTileX;
		int m_iTileY;
		bool m_bDeleteTile; /* Tile should be deleted */
		int m_iTileID;  /* ID of mode to switch to [IF NOT DELETING TILE] */
	public:
		CClientSetTile(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_CLIENT_SET_TILE;
			packet >> m_iTileX;
			packet >> m_iTileY; 
			packet >> m_bDeleteTile;
			if (!m_bDeleteTile)
				packet >> m_iTileID;
		}

		int GetTileX()			{ return m_iTileX; }
		int GetTileY()			{ return m_iTileY; }
		bool DeleteTile()		{ return m_bDeleteTile; }
		int GetTileID()			{ return m_iTileID; }

		/* Create a CClientSetTile packet */
		static OHPacket CreateDeleteTile(int x, int y) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_CLIENT_SET_TILE);
			packet << x;
			packet << y;
			packet << true;
			return packet;
		}
		/* Create a CClientSetTile packet */
		static OHPacket CreateSetTile(int x, int y, int iTileID) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_CLIENT_SET_TILE);
			packet << x;
			packet << y;
			packet << false;
			packet << iTileID;
			return packet;
		}
	};

	/*
	Server tile update
	Server->Client

	Contains an update to a map tile
	*/
	class CServerTileUpdate : public CBasePacket {
		int m_iTileX;
		int m_iTileY;
		bool m_bDeleteTile; /* Tile should be deleted */
		int m_iTileID;  /* ID of mode to switch to [IF NOT DELETING TILE] */
	public:
		CServerTileUpdate(OHPacket& packet) {
			m_bIsValid = packet.GetID() == OHPacket::ID_SERVER_TILE_UPDATE;
			if (!m_bIsValid)
				return;
			
			packet >> m_iTileX;
			packet >> m_iTileY;
			packet >> m_bDeleteTile;
			if (!m_bDeleteTile)
				packet >> m_iTileID;

			printf("CLIENT READ %i %i\n", m_iTileX, m_iTileY);
		}

		int GetTileX()			{ return m_iTileX; }
		int GetTileY()			{ return m_iTileY; }
		bool DeleteTile()		{ return m_bDeleteTile; }
		int GetTileID()			{ return m_iTileID; }

		/* Create a CServerTileUpdate delete tile */
		static OHPacket CreateDeleteTile(int x, int y) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_TILE_UPDATE);
			packet << x;
			packet << y;
			packet << true;
			printf("SEVER %i %i\n", x, y);
			return packet;
		}
		/* Create a CServerTileUpdate set tile */
		static OHPacket CreateSetTile(int x, int y, int iTileID) {
			OHPacket packet;
			packet << (sf::Int32)(OHPacket::ID_SERVER_TILE_UPDATE);
			packet << x;
			packet << y;
			packet << false;
			packet << iTileID;
			printf("SERVER %i %i\n", x, y);
			return packet;
		}
	};
};