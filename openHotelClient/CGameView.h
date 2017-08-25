#ifndef __CGAMEVIEW_H__
#define __CGAMEVIEW_H__

/*
Christopher De Pasquale
20 December 2016

GameView is main rendering part of client
abstracts away all rendering, window creation etc.
*/

#include "CWindow.h"
#include "CFont.h"
#include "ClientAssets.h"
#include "CHotelClient.h"
#include "CIsoRenderer.h"
#include "CInput.h"
#include "CPlayer.h"
#include "TileTypes.h"
#include "CChatDisplay.h"

class CIsoRenderer;
class CGameView {
	friend class CIsoRenderer;
public:
	enum EGameState {
		STATE_SERVERSELECT,
		STATE_GAME,
	};
private:
	CWindow m_Window;
	
	CInput m_GameInput;

	sf::Texture m_MenuBackground;
	CFont m_TextFont = CFont(ClientAssets::MAIN_TEXT);

	CIsoRenderer m_Renderer;

	sf::Vector2f m_vGameCameraPoint = sf::Vector2f(0.0f, 0.0f); 

	CPlayer m_Player;

	/* In-Game GUI */
	tgui::Gui* m_pGameGUI = NULL;
	/* Main menu GUI */
	tgui::Gui* m_pMenuGUI = NULL;
	/* In-Game tiles panel */
	tgui::Panel::Ptr m_pTilesPanel = NULL;
	/* In-Game chatbox */
	tgui::EditBox::Ptr m_pChatbox = NULL;

	TileID::ETileID m_eSelectedTile;

	EGameState m_GameState = STATE_SERVERSELECT;
	
	CChatDisplay m_ChatDisplay;

	/* GUI Callbacks */
	void SetTileID(int ID);
	void StopEdit(CHotelClient* client);

public:
	CGameView() {}
	~CGameView() {}

	/* Initialise game view */
	bool Initialise(CClientSettings& settings, std::string sGameTitle, CHotelClient* pHotelClient);

	/* Updates game view */
	void Update(CHotelClient& client, float fElapsedTime);

	/* Draw game */
	void Draw(CHotelClient& client);

	/* Returns tile X/Y index from a screen pixel */
	sf::Vector2i ScreenToTile(sf::Vector2i mousePos);

	/* Free resources & close gameview*/
	void Shutdown();

	bool ShouldQuit() { return m_Window.WindowClosed(); }
};
#endif