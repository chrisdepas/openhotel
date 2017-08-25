#include "stdafx.h"
#include "Hotel.h"

void CHotelRoom::InitTiles() {
	for (int j = 0; j < m_iHeight; j++) {
		for (int i = 0; i < m_iWidth; i++) {
			m_Tiles[j][i].iElevation = 0;
			m_Tiles[j][i].eTileType = ETileType::TYPE_EMPTY;
		}
	}
}

void CHotelRoom::EncodeToPacket(sf::Packet& packet) {
	/* Save name and type */
	packet << m_sRoomName;
	packet << (sf::Int32)(m_eRoomType);

	/* Save size */
	packet << m_iWidth;
	packet << m_iHeight;

	/* Save tile data */
	for (int j = 0; j < m_iHeight; j++) {
		for (int i = 0; i < m_iWidth; i++) {
			/* Tile X/Y */
			packet << i;
			packet << j;
			/* Tile data */
			packet << m_Tiles[j][i].iElevation;
			packet << m_Tiles[j][i].eTileType;
		}
	}
}

sf::Vector2f GetEntryPosition() {
	printf("GetEntryPosition not implemented!!\n");
	return sf::Vector2f(0, 0);
}

void CHotelRoom::DecodeFromPacket(sf::Packet& packet) {
	/* Read name and type */
	packet >> m_sRoomName;
	int roomType;
	packet >> roomType;
	m_eRoomType = (CHotelRoom::ERoomType)roomType;

	/* Read size */
	packet >> m_iWidth;
	packet >> m_iHeight;

	/* Read tile data */
	for (int j = 0; j < m_iHeight; j++) {
		for (int i = 0; i < m_iWidth; i++) {
			/* Read X/Y */
			int x, y;
			packet >> x;
			packet >> y;
			/* Tile data */
			int iTileType;
			packet >> m_Tiles[y][x].iElevation;
			packet >> iTileType;
			m_Tiles[y][x].eTileType = (ETileType)iTileType;
		}
	}
}

bool CHotelRoom::TilePositionInBounds(int x, int y) {
	return ((x >= 0 && x < m_iWidth) && (y >= 0 && y < m_iHeight));
}

void CHotelRoom::PrintToConsole() {
	printf("Hotel Room '%s'\n", m_sRoomName.c_str());

	switch (m_eRoomType) {

	case ROOMTYPE_LOBBY:
		puts("Lobby Room."); break;
	case ROOMTYPE_HALL:
		puts("Hall Room."); break;
	case ROOMTYPE_RESIDENT:
		puts("Resident Room."); break;
	}

	for (int j = 0; j < m_iHeight; j++) {
		for (int i = 0; i < m_iWidth; i++) {
			if (m_Tiles[j][i].eTileType == ETileType::TYPE_EMPTY)
				printf("E");
			else if (m_Tiles[j][i].eTileType == ETileType::TYPE_FLOOR)
				printf("F");
			else if (m_Tiles[j][i].eTileType == ETileType::TYPE_STAIRS)
				printf("S");
			else
				printf("?");
		}
		printf("\n");
	}
}

void CHotelRoom::DeleteTile(int x, int y) {
	m_Tiles[y][x].eTileType = ETileType::TYPE_EMPTY;
	m_Tiles[y][x].iTileID = TileID::TILE_BLANK;
}

void CHotelRoom::SetTile(int x, int y, TileID::ETileID iTileID) {
	m_Tiles[y][x].eTileType = ETileType::TYPE_FLOOR;
	m_Tiles[y][x].iTileID = iTileID;
}

int CHotel::GetLobbyIndex() {
	for (unsigned int i = 0; i < m_vHotelRooms.size(); i++) {
		if (m_vHotelRooms[i].GetType() == CHotelRoom::ERoomType::ROOMTYPE_LOBBY)
			return i;
	}
	return -1;
}

bool CHotel::LoadHotelFromFile(std::string sHotelFile) {
	/* Open hotel file */
	FILE* fileHotel = 0;
	if (fopen_s(&fileHotel, sHotelFile.c_str(), "r") != 0 || !fileHotel) {
		printf("Unable to open Hotel file '%s'\n", sHotelFile.c_str());
		return false;
	}

	/* Get filesize */
	fseek(fileHotel, 0, SEEK_END);
	int hotelFileSize = ftell(fileHotel);
	rewind(fileHotel);

	/* Load entire file into memory */
	char* hotelFileBuf = (char*)malloc(hotelFileSize + 1);
	hotelFileBuf[hotelFileSize] = '\0';
	fread_s(hotelFileBuf, hotelFileSize + 1, hotelFileSize, 1, fileHotel);
	fclose(fileHotel);

	/* Parse XML */
	rapidxml::xml_document<> doc;
	try {
		doc.parse<0>(hotelFileBuf);
	}
	catch (...) {
		printf("Parsing hotel file '%s' failed - Check XML\n", sHotelFile.c_str());
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();
		return false;
	}

	/* Find hotel data node  */
	rapidxml::xml_node<>* hotelDataNode = doc.first_node("Hotel");
	if (!hotelDataNode) {
		puts("Unable to load Hotel file - missing hotel data node.\n");
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();
		return false;
	}

	/* Check file format version */
	rapidxml::xml_attribute<>* versionAttribute = hotelDataNode->first_attribute("version");
	if (!versionAttribute || !versionAttribute->value()) {
		puts("Unable to read hotel, missing version attribute in Hotel data node.\n");
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();

		return false;
	}
	else {
		int version = strtol(versionAttribute->value(), NULL, 10);
		if (version != FILE_FORMAT_VERSION) {
			printf("Unable to parse Hotel - parser code doesn't match version (File version: %f, Parser version: %f)\n", version, FILE_FORMAT_VERSION);
			/* Free resources */
			free(hotelFileBuf);
			doc.clear();

			return false;
		}
	}

	/* Read hotel name */
	rapidxml::xml_attribute<>* nameAttribute = hotelDataNode->first_attribute("name");
	if (!nameAttribute || !nameAttribute->value()) {
		puts("Unable to parse hotel - missing name attribute.\n");
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();

		return false;
	}
	else {
		m_sHotelName = nameAttribute->value();
	}

	/* Read hotel motd */
	rapidxml::xml_attribute<>* motdAttribute = hotelDataNode->first_attribute("motd");
	if (!motdAttribute || !motdAttribute->value()) {
		puts("Unable to parse hotel - missing motd attribute.\n");
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();

		return false;
	}
	else {
		m_sHotelMOTD = motdAttribute->value();
	}

	/* Read rooms count */
	rapidxml::xml_attribute<>* roomCountAttribute = hotelDataNode->first_attribute("roomcount");
	int roomCount = -1;
	if (!roomCountAttribute || !roomCountAttribute->value()) {
		puts("Unable to read hotel, missing room count attribute.\n");
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();

		return false;
	}
	else {
		roomCount = strtol(roomCountAttribute->value(), NULL, 10);
		if (roomCount < HOTEL_ROOM_COUNT_MIN || roomCount > HOTEL_ROOM_COUNT_MAX) {
			printf("Unable to load hotel rooms - room count out of range [%i, %i]\n",
				HOTEL_ROOM_COUNT_MIN, HOTEL_ROOM_COUNT_MAX);

			/* Free resources */
			free(hotelFileBuf);
			doc.clear();
			return false;
		}
	}

	/* Read room nodes */
	rapidxml::xml_node<>* curRoomNode = hotelDataNode->first_node("Room");
	rapidxml::xml_attribute<>* roomAttribute = NULL;
	int iRoomCount = 0;
	while (curRoomNode) {
		/* Check for room overflow */
		if (iRoomCount + 1 > roomCount) {
			printf("Unable to load hotel. Rooms in hotel file exceed specified count.\n");

			/* Free resources */
			free(hotelFileBuf);
			doc.clear();
			return false;
		}
		CHotelRoom newRoom;

		/* Default room tiles to empty */
		newRoom.InitTiles();

		/* Read room name */
		roomAttribute = curRoomNode->first_attribute("name");
		if (!roomAttribute || !roomAttribute->value()) {
			printf("Unable to load hotel room - missing name attribute\n");

			/* Free resources */
			free(hotelFileBuf);
			doc.clear();
			return false;
		}
		newRoom.m_sRoomName = roomAttribute->value();

		/* Read room type */
		roomAttribute = curRoomNode->first_attribute("type");
		if (!roomAttribute || !roomAttribute->value()) {
			printf("Unable to load hotel room - missing type attribute\n");

			/* Free resources */
			free(hotelFileBuf);
			doc.clear();
			return false;
		}
		/* Validate room type */
		int roomTypeVal = strtol(roomAttribute->value(), NULL, 10);
		if (roomTypeVal <= CHotelRoom::ROOMTYPE_FIRST || roomTypeVal >= CHotelRoom::ROOMTYPE_LAST) {
			printf("Unable to load hotel room - room type out of range (%i, %i)\n",
				CHotelRoom::ROOMTYPE_FIRST, CHotelRoom::ROOMTYPE_LAST);

			/* Free resources */
			free(hotelFileBuf);
			doc.clear();
			return false;
		}
		newRoom.m_eRoomType = (CHotelRoom::ERoomType)roomTypeVal;

		newRoom.m_iHeight;
		newRoom.m_iWidth;
		newRoom.m_Tiles;

		/* Add room */
		m_vHotelRooms.push_back(newRoom);

		/* Next */
		curRoomNode = curRoomNode->next_sibling();
		iRoomCount++;
	}

	/* Check for room underflow */
	if (iRoomCount != roomCount) {
		printf("Unable to load hotel. Rooms in hotel file exceed specified count.\n");
		/* Free resources */
		free(hotelFileBuf);
		doc.clear();

		return false;
	}

	/* Free resources */
	free(hotelFileBuf);
	doc.clear();

	m_bIntitialised = true;
	return true;
}

bool CHotel::SaveHotelToFile(std::string sHotelFile) {
	/* Remove output file if it exists */
	if (std::remove(sHotelFile.c_str()) == 0) {
		printf("Removed file '%s'\n", sHotelFile.c_str());
	}

	/* Open output file */
	std::ofstream file(sHotelFile);
	if (file.bad()) {
		printf("Couldn't save hotel to file. Unable to write to '%s'\n", sHotelFile.c_str());
		return false;
	}

	/* Write hotel description into buffer */
	char hotelDescBuf[2048];
	int len = sprintf_s(hotelDescBuf, 2048, "<Hotel version=\"%i\" name=\"%s\" motd=\"%s\" roomcount=\"%i\" >\n",
		FILE_FORMAT_VERSION, m_sHotelName.c_str(), m_sHotelMOTD.c_str(), m_vHotelRooms.size());
	if (len == -1) {
		file.close();
		puts("Unable to write hotel file - couldn't format hotel description\n");
		return false;
	}
	/* Write hotel description buffer to file */
	file.write(hotelDescBuf, len);

	/* Write each room to file */
	for (unsigned int i = 0; i < m_vHotelRooms.size(); i++) {

		/* Write room header with details */
		len = sprintf_s(hotelDescBuf, 2048, "  <Room name=\"%s\" type=\"%i\">\n",
			m_vHotelRooms[i].m_sRoomName.c_str(), (int)m_vHotelRooms[i].m_eRoomType);
		if (len == -1) {
			file.close();
			printf("Unable to export hotel - couldn't write room description\n");
			return false;
		}
		file.write(hotelDescBuf, len);

		/* Write each non-empty tile */
		for (int k = 0; k < m_vHotelRooms[i].m_iHeight; k++) {
			for (int j = 0; j < m_vHotelRooms[i].m_iWidth; j++) {
				/* Ignore empty tiles */
				if (m_vHotelRooms[i].m_Tiles[k][j].eTileType == ETileType::TYPE_EMPTY)
					continue;

				/* Write tile data header */
				len = sprintf_s(hotelDescBuf, 2048, "    <Tile x=\"%i\" y=\"%i\" type=\"%i\" id=\"%i\">\n",
					k, j, m_vHotelRooms[i].m_Tiles[k][j].eTileType, m_vHotelRooms[i].m_Tiles[k][j].iTileID);
				if (len == -1) {
					file.close();
					printf("Unable to export hotel - couldn't write tile data\n");
					return false;
				}
				file.write(hotelDescBuf, len);

				/* Write tile data end tag*/
				file.write("    </Tile>\n", strlen("    </Tile>\n"));
			}
		}

		/* Write room end tag*/
		file.write("  </Room>\n", strlen("  </Room>\n"));
	}

	/* Write room data end */
	file.write("</Hotel>\n", strlen("</Hotel>\n"));

	/* Close file */
	file.close();

	return true;
}

void CHotel::InitAsDefault() {
	/* Default MODT/name */
	m_sHotelName = "OpenHotel";
	m_sHotelMOTD = "Welcome to OpenHotel";

	/* Create lobby */
	CHotelRoom lobby;
	lobby.m_eRoomType = CHotelRoom::ROOMTYPE_LOBBY;
	lobby.m_sRoomName = "Lobby";
	/* Init as completely empty room */
	lobby.InitTiles();
	/* Add to hotel */
	m_vHotelRooms.push_back(lobby);
}

int CHotel::GetRoomCount() {
	return m_vHotelRooms.size();
}

CHotelRoom& CHotel::GetHotelRoomByIndex(int index) {
	return m_vHotelRooms[index];
}