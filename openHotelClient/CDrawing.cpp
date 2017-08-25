#include "stdafx.h"
#include "CDrawing.h"

void CDrawing::DrawTexture(float x, float y, sf::Texture& texture, CWindow& window) {
	/* Create a renderable sprite */
	sf::Sprite sp;
	sp.setTexture(texture);
	sp.setPosition(x, y);

	/* Draw to window*/
	window.Draw(sp);
}
void CDrawing::DrawTextureMirrored(float x, float y, sf::Texture& texture, CWindow& window, EMirrorDirection mirror) {
	/* Create a renderable sprite */
	sf::Sprite sp;
	sp.setTexture(texture);
	sp.setPosition(x + texture.getSize().x, y);
	if (mirror == MIRROR_HORIZONTAL)
		sp.setScale(sf::Vector2f(-1.0f, 1.0f));
	else {
		printf("FATAL _ HORIZONTAL MIRROR ONLY\n");
		exit(0);
	}

	/* Draw to window*/
	window.Draw(sp);
}