#ifndef __CISORENDERER_H__
#define __CISORENDERER_H__

#include "Hotel.h"
#include "CPlayer.h"
#include "SFML\Graphics.hpp"
#include "CWindow.h"
#include "TileTypes.h"
/*
	Christopher De Pasquale
	20th December 2016
*/
	
class CGameView;

class CIsoRenderer {
	friend class CGameView;

	sf::Texture m_TileTextures[TileID::TILE_LAST];

	bool m_bInitialised;

	bool m_bTilesPreloaded = false;

	bool PreloadTiles();

	const int TILE_DRAW_WIDTH = 64;
	const int TILE_DRAW_HEIGHT = 32;

	sf::Texture m_SelectedTileTexture;

	/* Create Tile selector GUI, return YOffset of last GUI element */
	int CreateTileGUI(tgui::Gui* pGameGUI, CGameView* pGameView, int yOffset);
public:

	/* Init renderer */
	bool Initialise();

	/*  */
	sf::Vector2i GetTileAtPosition(sf::Vector2f vWorldPosition);

	sf::Vector2f GetCameraTranslationForTilePoint(sf::Vector2f tilePosition);

	void DrawHotelRoom(CHotelRoom* hotelRoom, CWindow& window);
	void DrawPlayer(CPlayer& player, sf::Vector2f playerPosition, CWindow& window);

	void DrawSelectedTile(CHotelRoom* hotelRoom, CWindow& window, int tileX, int tileY); 
};

#endif