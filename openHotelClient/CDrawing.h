#ifndef __CDRAWING_H__
#define __CDRAWING_H__

/*
	Christopher De Pasquale
	20 December 2016
*/
#include "SFML\Graphics.hpp"
#include "CWindow.h"
class CDrawing {
	CDrawing();
public:
	enum EMirrorDirection {
		MIRROR_VERTICAL,
		MIRROR_HORIZONTAL,
	};
	static void DrawTexture(float x, float y, sf::Texture& texture, CWindow& window);
	static void DrawTextureMirrored(float x, float y, sf::Texture& texture, CWindow& window, EMirrorDirection mirror = MIRROR_HORIZONTAL);
};
#endif