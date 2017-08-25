#ifndef __TILETYPES_H__
#define __TILETYPES_H__

/*
	Christopher De Pasquale
	20 December 2016
*/

/* Floor tile types */
enum ETileType {
	TYPE_EMPTY, /* No tile */
	TYPE_FLOOR,
	TYPE_STAIRS,
};

namespace TileID {
	enum ETileID {
		TILE_BLANK = 0,
		TILE_CARPETWHITE,
		TILE_FLOORBOARD,
		TILE_DEV,
		TILE_LAST,
	};
};

#endif