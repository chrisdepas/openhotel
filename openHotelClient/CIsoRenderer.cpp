#include "stdafx.h"
#include "CIsoRenderer.h"
#include "CDrawing.h"
#include "ClientAssets.h"
#include "CDebugLogger.h"
#include "CGameView.h"

bool CIsoRenderer::PreloadTiles() {
	for (int i = 0; i < TileID::TILE_LAST; i++) {
		if (i == TileID::TILE_BLANK)
			continue;
		if (!m_TileTextures[i].loadFromFile(ClientAssets::TILE_ASSETS[i])) {
			CDebugLogger::LogError("PreloadTiles() - Failed to load asset %s\n", ClientAssets::TILE_ASSETS[i]);
			return false;
		}
	}
	return (m_bTilesPreloaded = true);
}

int CIsoRenderer::CreateTileGUI(tgui::Gui* pGameGUI, CGameView* pGameView, int _yOffset) {
	tgui::Panel::Ptr tilesPanel = (tgui::Panel::Ptr)pGameGUI->get<tgui::Panel>("TilesPanel");
	int xOffset = 0;
	int yOffset = _yOffset;
	for (unsigned int i = 0; i < TileID::TILE_LAST; i++) { 
		tgui::Picture::Ptr picTile = std::make_shared<tgui::Picture>();
		picTile->setSize(32, 32);
		picTile->setPosition(tgui::Layout((float)xOffset), tgui::Layout((float)yOffset));

		if (i == TileID::TILE_BLANK)
			picTile->setTexture(m_SelectedTileTexture);
		else
			picTile->setTexture(m_TileTextures[i]);
			
		picTile->connect("Clicked", &CGameView::SetTileID, pGameView, i);
		tilesPanel->add(picTile);

		xOffset += 34;
		if (xOffset + 32 > tilesPanel->getSize().x) {
			xOffset = 0;
			yOffset += 34;
		}
	}
	if (xOffset != 0)
		yOffset += 34;
	return yOffset;
}

bool CIsoRenderer::Initialise() {
	m_bInitialised = PreloadTiles();
	if (!m_bInitialised) {
		CDebugLogger::LogFatal("Failed to Initialise Isometric renderer\n");
		return false;
	}
	m_bInitialised = m_SelectedTileTexture.loadFromFile(ClientAssets::TILE_SELECT_OVERLAY);

	return m_bInitialised;
}

sf::Vector2i CIsoRenderer::GetTileAtPosition(sf::Vector2f vWorldPosition) {
	sf::Vector2i result; 

	// i = x/w - y/h + 1/2;
	// j = y/h + x/w - 1/2;
	result.x = (int)(((vWorldPosition.x / (float)TILE_DRAW_WIDTH) - (vWorldPosition.y / (float)TILE_DRAW_HEIGHT)) + 0.5f);
	result.y = (int)(((vWorldPosition.y / (float)TILE_DRAW_HEIGHT) + (vWorldPosition.x / (float)TILE_DRAW_WIDTH)) - 0.5f);

	return result;
}
sf::Vector2f CIsoRenderer::GetCameraTranslationForTilePoint(sf::Vector2f tilePosition) {
	int width = TILE_DRAW_WIDTH;
	int height = TILE_DRAW_HEIGHT;
	
	float x = (tilePosition.x * width / 2.0f) + (tilePosition.y * width / 2.0f);
	float y = (tilePosition.y * height / 2.0f) - (tilePosition.x * height / 2.0f);
	return sf::Vector2f(x, y);
}
void CIsoRenderer::DrawHotelRoom(CHotelRoom* hotelRoom, CWindow& window) {
	HotelRoomTiles& tiles = hotelRoom->GetTiles();
	int height = hotelRoom->GetHeight();
	int width = hotelRoom->GetWidth();
	for (int j = 0; j < height; j++) {
		for (int i = width-1; i >= 0; i--) {
			if (tiles[j][i].eTileType != ETileType::TYPE_EMPTY) {
				int width = TILE_DRAW_WIDTH;
				int height = TILE_DRAW_HEIGHT;
				float x = (i * width / 2.0f) + (j * width / 2.0f);
				float y = (j * height / 2.0f) - (i * height / 2.0f);
				CDrawing::DrawTexture(x, y, m_TileTextures[tiles[j][i].iTileID], window);
			}
			else {
				/*
				int width = TILE_DRAW_WIDTH;
				int height = TILE_DRAW_HEIGHT;
				float x = (i * width / 2.0f) + (j * width / 2.0f);
				float y = (j * height / 2.0f) - (i * height / 2.0f);
				CDrawing::DrawTexture(x, y, m_TileTextures[TileID::TILE_DEV], window);*/
			}
		}
	}
}
void CIsoRenderer::DrawPlayer(CPlayer& player, sf::Vector2f playerPosition, CWindow& window) {
	float x = (playerPosition.x * TILE_DRAW_WIDTH / 2.0f) + (playerPosition.y * TILE_DRAW_WIDTH / 2.0f);
	float y = (playerPosition.y * TILE_DRAW_HEIGHT / 2.0f) - (playerPosition.x * TILE_DRAW_HEIGHT / 2.0f);
	player.Draw(x, y, window);
}
void CIsoRenderer::DrawSelectedTile(CHotelRoom* hotelRoom, CWindow& window, int tileX, int tileY) {
	if (tileX < 0 || tileY < 0)
		return;
	if (tileY >= hotelRoom->GetHeight() || tileX >= hotelRoom->GetWidth())
		return;
	int width = TILE_DRAW_WIDTH;
	int height = TILE_DRAW_HEIGHT;
	float x = (tileX * width / 2.0f) + (tileY * width / 2.0f);
	float y = (tileY * height / 2.0f) - (tileX * height / 2.0f);
	CDrawing::DrawTexture(x, y, m_SelectedTileTexture, window);
}