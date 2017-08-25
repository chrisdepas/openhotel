#ifndef __HOTEL_H__
#define __HOTEL_H__

/*
	Christopher De Pasquale
		19 December 2016
 - Lost old version of file :( 
*/

#include "rapidxml-1.13\rapidxml.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include "SFML\Network.hpp"
#include "TileTypes.h"
class CHotel;

/* Dimensions of hotel room, in tiles */
#define HOTEL_ROOM_WIDTH 64
#define HOTEL_ROOM_HEIGHT 64

/* Hotel tile */
struct SHotelTile { 
	ETileType eTileType;		/* Tile type */
	int iElevation;				/* Z-Level */
	TileID::ETileID iTileID;	/* Tile ID (Img to draw) */
	bool IsEmpty() {
		return eTileType == ETileType::TYPE_EMPTY;
	}
};

/*  Hotel tile sent over network 
  (Contains location data for tile) */
struct SNetworkedHotelTile : public SHotelTile {
	int tileX; /* Index */
	int tileY; /* Index */

	SNetworkedHotelTile() {}
	/* Helper constructor */
	SNetworkedHotelTile(ETileType tiletype, int elevation, int x, int y) {
		eTileType = tiletype;
		iElevation = elevation;
		tileX = x;
		tileY = y;
	}
};

typedef SHotelTile HotelRoomTiles[HOTEL_ROOM_WIDTH][HOTEL_ROOM_HEIGHT];

class CHotelRoom {
public:
	/* Type of room */
	enum ERoomType {
		ROOMTYPE_FIRST = 0, /* Keep this first */

		ROOMTYPE_LOBBY,		/* Lobby, new players join this first */
		ROOMTYPE_HALL,		/* Hallway connector room */
		ROOMTYPE_RESIDENT,	/* Standard room */

		ROOMTYPE_LAST,		/* Keep this last */
	};

	/* Room doorway */
	struct SRoomDoorway {
		int iFlags;
	};

private:
	friend class CHotel;

	const static int ROOM_WIDTH = HOTEL_ROOM_WIDTH;
	const static int ROOM_HEIGHT = HOTEL_ROOM_HEIGHT;

	int m_iWidth = ROOM_WIDTH; /* In tiles */
	int m_iHeight = ROOM_HEIGHT; /* In tiles */

	HotelRoomTiles m_Tiles;
	
protected:

	/* Name of room */
	std::string m_sRoomName;

	/* Type of room */
	ERoomType m_eRoomType;

	/* Doors in room */
	std::vector<SRoomDoorway> m_Doorways;

public:
	/* Types of hotel room */
	

	/* Access room data */
	std::string GetName()		{ return m_sRoomName; }
	ERoomType GetType()			{ return m_eRoomType; }
	int GetWidth()				{ return m_iWidth; }
	int GetHeight()				{ return m_iHeight; }
	HotelRoomTiles& GetTiles()	{ return m_Tiles; }

	/* Free resources */
	void Cleanup() {}

	/* Write all tiles to empty */
	void InitTiles();

	/* Write to packet */
	void EncodeToPacket(sf::Packet& packet);
	/* Read from packet */
	void DecodeFromPacket(sf::Packet& packet);

	/* Get enter position */
	sf::Vector2f GetEntryPosition();

	/* Bounds check */
	bool TilePositionInBounds(int tileIndexX, int tileIndexY);

	/* */
	void PrintToConsole(); 

	/* Remove a tile (Set to blank) */
	void DeleteTile(int x, int y);
	/* Change a tile */
	void SetTile(int x, int y, TileID::ETileID iTileID);
};

class CHotel {
	/* Name of hotel */
	std::string m_sHotelName;

	/* Hotel rooms */
	std::vector<CHotelRoom> m_vHotelRooms;

	/* Hotel join message */
	std::string m_sHotelMOTD;

	/* Has hotel been initialised */
	bool m_bIntitialised = false;

public:
	/* File format version for saving/loading */
	const int FILE_FORMAT_VERSION = 1;

	/* Range of accepted values for room count */
	const int HOTEL_ROOM_COUNT_MIN = 1;
	const int HOTEL_ROOM_COUNT_MAX = 256;

	/* Get index of lobby room, returns -1 on fail */
	int GetLobbyIndex();

	/* Load hotel from disk */
	bool LoadHotelFromFile(std::string sHotelFile);

	/* Save hotel to disk */
	bool SaveHotelToFile(std::string sHotelFile);

	/* Init hotel as default hotel with lobby */
	void InitAsDefault();

	/* Get number of rooms in hotel */
	int GetRoomCount();

	/* Get hotel room */
	CHotelRoom& GetHotelRoomByIndex(int index);

};
#endif