#include "stdafx.h"
#include "CPlayer.h"
#include "CDrawing.h"

void CPlayer::Draw(float x, float y, CWindow& window) {
	switch (m_eDirection) {
		case DIRECTION_S:
			CDrawing::DrawTexture(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_STexture, window);
			break;
		case DIRECTION_SW: 
			CDrawing::DrawTexture(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_SWTexture, window);
			break;
		case DIRECTION_W: 
			CDrawing::DrawTexture(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_WTexture, window);
			break;
		case DIRECTION_NW:
			CDrawing::DrawTexture(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_NWTexture, window);
			break;
		case DIRECTION_N: 
			CDrawing::DrawTexture(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_NTexture, window);
			break;
		case DIRECTION_NE: 
			CDrawing::DrawTextureMirrored(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_NWTexture, window);
			break;
		case DIRECTION_E: 
			CDrawing::DrawTextureMirrored(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_WTexture, window);
			break;
		case DIRECTION_SE: 
			CDrawing::DrawTextureMirrored(x + m_vRenderOffset.x, y + m_vRenderOffset.y, m_SWTexture, window);
			break;
	}
}
void CPlayer::Update(bool bInputEnabled) {

	/* Update rendering position */
	if (bInputEnabled) {
		bool bLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
		bool bRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
		bool bUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
		bool bDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

		if (bLeft && bDown) {
			m_eDirection = DIRECTION_SW;
		}
		else if (bLeft && bUp) {
			m_eDirection = DIRECTION_NW;
		}
		else if (bRight && bDown) {
			m_eDirection = DIRECTION_SE;
		}
		else if (bRight && bUp) {
			m_eDirection = DIRECTION_NE;
		}
		else if (bLeft) {
			m_eDirection = DIRECTION_W;
		}
		else if (bUp) {
			m_eDirection = DIRECTION_N;
		}
		else if (bDown) {
			m_eDirection = DIRECTION_S;
		}
		else if (bRight) {
			m_eDirection = DIRECTION_E;
		}
	}

}

bool CPlayer::Initialise() {
	if (!m_STexture.loadFromFile("Graphics\\Player\\1.png")) return false;
	if (!m_SWTexture.loadFromFile("Graphics\\Player\\2.png")) return false;
	if (!m_WTexture.loadFromFile("Graphics\\Player\\3.png")) return false;
	if (!m_NWTexture.loadFromFile("Graphics\\Player\\4.png")) return false;
	if (!m_NTexture.loadFromFile("Graphics\\Player\\5.png")) return false;
	return true;
}

void CPlayer::SetPosition(sf::Vector2i vTilePosition) {

}
