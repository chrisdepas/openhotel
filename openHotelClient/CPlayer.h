#ifndef __CPLAYER_H__
#define __CPLAYER_H__

/*
	Christopher De Pasquale
	20th December 2016

	Player class, handles drawing of player (not movement)
*/
#include "SFML\Main.hpp"
#include "SFML\Graphics.hpp"

class CWindow;
class CIsoRenderer;
class CPlayer {
	friend class CIsoRenderer;
	enum EPlayerDirection {
		DIRECTION_S,
		DIRECTION_SW,
		DIRECTION_W,
		DIRECTION_NW,
		DIRECTION_N,
		DIRECTION_NE,
		DIRECTION_E,
		DIRECTION_SE,
	};

	/* Direction textures */
	sf::Texture m_STexture;
	sf::Texture m_SWTexture;
	sf::Texture m_WTexture;
	sf::Texture m_NWTexture;
	sf::Texture m_NTexture;

	/* Offset to render player at */
	sf::Vector2i m_vRenderOffset = sf::Vector2i(5, -66);

	/* Current direction */
	EPlayerDirection m_eDirection = DIRECTION_N;

protected:
	void Draw(float x, float y, CWindow& window);

public:
	void Update(bool bInputEnabled);

	bool Initialise();

	void SetPosition(sf::Vector2i vTilePosition);

	void Render(CWindow& window);
};

#endif