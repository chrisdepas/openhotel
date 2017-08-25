#ifndef __OHPACKET_H__
#define __OHPACKET_H__

#include <SFML\Network.hpp>

/*
	Christopher De Pasquale
	18 December 2016 
*/
 
class OHPacket : public sf::Packet {
	int m_iPacketID;
public:
	enum EPacketID { 
		ID_NONE,	/* NO ID */ 
		ID_NOTSET,	/* ID WASN'T SET */

		/* Client connect sequence */
		ID_CLIENT_INFO_UPDATE, 		/* Client->Server, client sends local info (Sent on connect & on info change) */
		ID_SERVER_INFO_RESPONSE, 	/* Server->Client, server response to client info (accepted/not accepted) */

		/* Room changes */
		ID_CLIENT_ENTER_LOBBY, 		/* Client->Server, client requests to join lobby */
		ID_CLIENT_ENTER_ROOM, 		/* Client->Server, client requests to join a room */
		ID_SERVER_ROOM_REQUEST_RESPONSE, /* Server->Client, contains response for room request + actual room data */
		ID_CLIENT_REQUEST_ROOM_DATA,/* Client->Server, client asks for room information (layout etc) */
		ID_SERVER_ROOM_DATA,		/* Server->Client, contains all data for a room */

		/* Chat */
		ID_CLIENT_CHAT_MESSAGE,		/* Client->Server, client requests a chat message */
		ID_SERVER_CHAT_UPDATE, 		/* Server->Client, server sends chat message update */

		/* Money */
		ID_SERVER_BALANCE_UPDATE, 	/* Server->Client, informs client of $ change */

		/* Client mode */
		ID_CLIENT_SET_MODE,			/* Client->Server, requests change of client mode (e.g. to Operator edit mode)*/
		ID_SERVER_MODE_RESPONSE,	/* Server->Client, response to mode change request */

		/* Room editing */
		ID_CLIENT_SET_TILE,			/* Client->Server, client wants to change tile */
		ID_SERVER_TILE_UPDATE,		/* Server->Client, lets client know a tile has been changed */
	};

	OHPacket() {
		m_iPacketID = ID_NOTSET;
	}
	void SetID(int id) {
		m_iPacketID = id;
	} 
	int GetID() {
		return m_iPacketID;
	}
	/* onReceive isn't called on server end ??
	virtual const void* onSend(std::size_t &size) {
		*this << sf::Int8(m_iPacketID);
		return getData();
	}
	virtual void onReceive(const void *data, std::size_t size) {
		std::size_t s_size = sizeof(sf::Int8);
		std::memcpy(&m_iPacketID, (char*)(data)+size - s_size, s_size);
		append(data, size - s_size);
	} */
};

#endif