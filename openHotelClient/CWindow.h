#ifndef __CWINDOW_H__
#define __CWINDOW_H__

/*
	Christopher De Pasquale
	20 December 2016
*/
#include "SFML\Graphics.hpp"
#include "CClientSettings.h"
#include "TGUI\TGUI.hpp"
class CInput;

class CWindow {
	sf::RenderWindow m_Window;
	bool m_bWindowCreated = false;

	bool m_bWindowClosed = false;
	bool m_bFocus = true;

	/* Clear color, defaults to black */
	sf::Color m_ClearColor = sf::Color(0, 0, 0, 255);

	/* Size as ints */
	int m_iWidth;
	int m_iHeight;

	/* Size as sf::Vector2f */
	sf::Vector2f m_vSize;

	/* GUI */
	tgui::Gui* m_pGUI = NULL;
	bool m_bGUIActive = false;

public:
	CWindow() {}
	~CWindow() {}

	bool CreateRenderWindow(CClientSettings& settings, std::string& sTitle);

	/* Handle events */
	void Update(CInput& input);

	/* Clears scene */
	void BeginDrawing();
	/* Presents scene */
	void EndDrawing();

	/* Destroy window */
	void Destroy();

	/**/
	sf::RenderWindow& GetRenderWindow() { return m_Window; }
	/**/
	void RestoreTranslation();

	/* Applies translated view to camera, must call RestoreTranslation() afterwards */
	void ApplyTranslation(sf::Vector2f vTranslation);

	void Draw(sf::Drawable& Item);

	bool HasFocus() { return m_bFocus; }
	bool WindowClosed() { return m_bWindowClosed; }

	int GetWidth() { return m_iWidth; }
	int GetHeight() { return m_iHeight; }

	/* GUI */
	void SetGUI(tgui::Gui* pGUI) { m_pGUI = pGUI; }
	void SetGUIActive(bool bActive) { m_bGUIActive = bActive; }
	bool IsGUIActive() { return m_bGUIActive; }
};

#endif