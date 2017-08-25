#ifndef __CLIENTASSETS_H__
#define __CLIENTASSETS_H__

/*
	Christopher De Pasquale
	20 dec 2016
	Uses patented 'Worst-practice method of defining assets'
*/
#include "TileTypes.h"

class ClientAssets {
public:
	static const char* MENU_BACKGROUND;
	static const char* MENU_TITLE;
	static const char* MAIN_TEXT;

	static const char* TILE_SELECT_OVERLAY;

	static const char* TILE_ASSETS[TileID::TILE_LAST];
};

#endif