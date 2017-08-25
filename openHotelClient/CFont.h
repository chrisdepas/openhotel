#ifndef __CFONT_H__
#define __CFONT_H__
#include "CWindow.h"
#include <SFML\Graphics.hpp>
class CFont {
	sf::Font m_Font;
	std::string m_sFontFile;
public:
	CFont(std::string sFontFile) { m_sFontFile = sFontFile; }
	bool Initialise();
	
	/* Draw non-formatted text */
	void Draw(float x, float y, CWindow& window, std::string string, int iSize = 16, sf::Color color = sf::Color::Black);
	void DrawCentred(float x, float y, CWindow& window, std::string string, int iSize = 16, sf::Color color = sf::Color::Black);

	/* Draw formatted text (like printf) */
	void DrawFormatted(float x, float y, CWindow& window, char* szFormat, int iSize = 16, sf::Color color = sf::Color::Black, ...);
	void DrawFormattedCentred(float x, float y, CWindow& window, char* szFormat, int iSize = 16, sf::Color color = sf::Color::Black, ...);
};

#endif