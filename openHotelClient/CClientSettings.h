#ifndef __CCLIENTSETTINGS_H__
#define __CCLIENTSETTINGS_H__

#include <string>
#include "SFML\Graphics.hpp"
class CClientSettings {
	/*
		Default settings
		*/
	const sf::Vector2i DEFAULT_WINDOWSIZE = sf::Vector2i(800, 600);
	const bool DEFAULT_FULLSCREEN = false;

	/* Size of game window */
	sf::Vector2i m_vWindowSize = DEFAULT_WINDOWSIZE;

	/* Fullscreen */
	bool m_bFullscreen = DEFAULT_FULLSCREEN;

public:
	CClientSettings() {}
	~CClientSettings() {}

	bool ReadFromFile(std::string sConfigFile);
	bool SaveToFile(std::string sConfigFile);

	/* Get window dimensions */
	sf::Vector2i GetWindowSize() { return m_vWindowSize; }
	bool GetFullscreen() { return m_bFullscreen; }
};
#endif