#include "stdafx.h"
#include "ClientAssets.h"

const char* ClientAssets::MENU_BACKGROUND = "Graphics\\Hotel.png";
const char* ClientAssets::MENU_TITLE = "Font\\HabboFont.ttf";
const char* ClientAssets::MAIN_TEXT = "Font\\Volter__28Goldfish_29.ttf";

const char* ClientAssets::TILE_SELECT_OVERLAY = "Graphics\\Tiles\\FloorSelect.png";

const char* ClientAssets::TILE_ASSETS[TileID::TILE_LAST] = {
	"BLANK",	//TILE_BLANK = 0,
	"Graphics\\Tiles\\CarpetWhite.png",		//TILE_CARPETWHITE,
	"Graphics\\Tiles\\Floorboard.png", //TILE_FLOORBOARD
	"Graphics\\Tiles\\DevFloor.png", // TILE_DEV
};