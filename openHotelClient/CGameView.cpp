/*
	Christopher De Pasquale
	21 December 2016
*/

#include "stdafx.h"
#include "CGameView.h"
#include "ClientAssets.h"
#include "CDebugLogger.h"
#include "CDrawing.h"
#include "OHNetwork.h"
#include "ClientModes.h"
#include <Windows.h>

bool CGameView::Initialise(CClientSettings& settings, std::string sGameTitle, CHotelClient* pHotelClient) {
	if (!m_Window.CreateRenderWindow(settings, sGameTitle)) {
		CDebugLogger::LogWarning("Failed to Create RenderWindow\n");
		return false;
	}
	if (!m_MenuBackground.loadFromFile(ClientAssets::MENU_BACKGROUND)) {
		CDebugLogger::LogWarning("Warning - Failed to load ClientAsset Menu background\n");
	}

	if (!m_TextFont.Initialise()) {
		CDebugLogger::LogWarning("Warning - Failed to load TextFont asset\n");
	}

	if (!m_Renderer.Initialise()) {
		CDebugLogger::LogWarning("Fatal - Failed to init renderer\n");
		return false;
	}

	if (!m_Player.Initialise()) {
		CDebugLogger::LogWarning("Fatal - Failed to init player\n");
		return false;
	}

	/* Create GUIs */
	m_pGameGUI = new tgui::Gui(m_Window.GetRenderWindow());
	m_pMenuGUI = new tgui::Gui(m_Window.GetRenderWindow());

	try {
		tgui::Theme::Ptr theme = std::make_shared<tgui::Theme>("widgets/Black.txt");

		/* Create tiles panel */
		tgui::Panel::Ptr tilesPanel = theme->load("Panel");
		tilesPanel->setSize(100, 600);
		tilesPanel->setPosition(0, 0);
		m_pGameGUI->add(tilesPanel, "TilesPanel");
		m_pTilesPanel = tilesPanel;

		/* Panel header */
		tgui::Layout tilesVertOffset(2.0f);
		tgui::Label::Ptr lblPanelTitle = std::make_shared<tgui::Label>();
		lblPanelTitle->setText("Edit Tiles");
		lblPanelTitle->setPosition(0.0f, tilesVertOffset); tilesVertOffset += 20.0f;
		lblPanelTitle->setTextColor(sf::Color::White);
		tilesPanel->add(lblPanelTitle);

		/* Tiles */
		tilesVertOffset = (tgui::Layout)m_Renderer.CreateTileGUI(m_pGameGUI, this, (int)tilesVertOffset.getValue());

		/* Edit button */
		tgui::Button::Ptr editBtn = theme->load("Button");
		editBtn->setText("Finish Editing");
		editBtn->setPosition(0.0f, tilesVertOffset); tilesVertOffset += 20.0f;
		editBtn->setSize(100.0f, 20.0f);
		editBtn->connect("pressed", &CGameView::StopEdit, this, pHotelClient);
		tilesPanel->add(editBtn, "EditBtn");

		/* Chat */
		m_pChatbox = theme->load("EditBox");
		m_pChatbox->setSize((float)m_Window.GetWidth(), 30.0f);
		m_pChatbox->setPosition(0.0f, (float)m_Window.GetHeight() - 30.0f);
		m_pChatbox->setText("");
		m_pChatbox->setTextSize(24);
		m_pChatbox->connect("ReturnKeyPressed", &C6SMultiplayerGameState::SendChatMessage, this, pGame);
		m_pChatbox->hide();
		m_pChatbox->disable();
		m_pGameGUI->add(m_pChatbox, "ChatMsgBox");

		m_Window.GetWidth();
		/* Finally, hide panel */
		if (pHotelClient->GetCurrentMode() != EClientMode::MODE_OPERATOREDIT) {
			tilesPanel->hide();
			m_Window.SetGUIActive(false);
		}
	}
	catch (...) {
		CDebugLogger::LogFatal("Failed to create GUI\n");
		return false;
	}

	return true;
}

void CGameView::SetTileID(int ID) {
	m_eSelectedTile = (TileID::ETileID)ID;
	printf("Tile set to %i\n", ID);
}
void CGameView::StopEdit(CHotelClient* client) {
	if (client->GetCurrentMode() == EClientMode::MODE_OPERATOREDIT) {
		/* Send request to switch back to walk mode */
		OHPacket info = OHNetwork::CClientSetMode::Create(EClientMode::MODE_WALK);
		if (!client->SendPacket(info)) {
			CDebugLogger::LogWarning("Failed to send ClientSetMode packet to server.\n");
		}
	}
}
void CGameView::Update(CHotelClient& client, float fElapsedTime) {
	/* Handle input */
	m_Window.Update(m_GameInput);

	/* Chat */
	m_ChatDisplay.Update(fElapsedTime);

	/* Update camera centre */
	m_vGameCameraPoint = m_Renderer.GetCameraTranslationForTilePoint(client.GetLocalPosition());

	/* Set edit mode */
	if (GetAsyncKeyState(VK_F1)) {
		if (client.GetCurrentMode() != EClientMode::MODE_OPERATOREDIT) {
			/* send request to switch to OP edit mode */
			OHPacket info = OHNetwork::CClientSetMode::Create(EClientMode::MODE_OPERATOREDIT);
			if (!client.SendPacket(info)) {
				CDebugLogger::LogWarning("Failed to send local info packet to server.\n");
			}
		}
		while (GetAsyncKeyState(VK_F1)){}
		m_pTilesPanel->show();
	}

	/* Enable GUI */
	if (GetAsyncKeyState(VK_ESCAPE)) {
		if (m_Window.IsGUIActive()) {
			m_Window.SetGUIActive(false);
		}
		else {
			m_Window.SetGUI(m_pGameGUI);
			m_Window.SetGUIActive(true);
		}

		while (GetAsyncKeyState(VK_ESCAPE)){}
	}

	/* Mouse event handler */
	SMouseEvent mouseEvent;
	while (m_GameInput.GetNextMouseEvent(mouseEvent)) {

		switch (mouseEvent.m_eType) {

		case SMouseEvent::TYPE_MOUSEPRESS:
			{
				if (client.CanEditCurrentRoom()) {
					/* Selected tile index */
					sf::Vector2i tile = ScreenToTile(m_GameInput.GetMousePosition());

					/* Ensure selected tile index is valid */
					if (client.GetCurrentRoom()->TilePositionInBounds(tile.x, tile.y)) {

						/* Send delte/set tile packet */
						if (m_eSelectedTile == TileID::TILE_BLANK) {
							OHPacket setTile = OHNetwork::CClientSetTile::CreateDeleteTile(tile.x, tile.y);
							if (!client.SendPacket(setTile)) {
								CDebugLogger::LogWarning("Failed to send local info packet to server.\n");
							}
						}
						else {
							OHPacket setTile = OHNetwork::CClientSetTile::CreateSetTile(tile.x, tile.y, m_eSelectedTile);
							if (!client.SendPacket(setTile)) {
								CDebugLogger::LogWarning("Failed to send local info packet to server.\n");
							}
						}
						
					}
				}
			}
			break;

		case SMouseEvent::TYPE_MOUSERELEASE:
		case SMouseEvent::TYPE_SCROLL:
		default:
			break;
		}
	}

	if (GetAsyncKeyState(VK_RETURN)) {
		
		if (chatBox && !chatBox->isEnabled()) {
			/* Clear input of show chat key */
			pGame->m_Input.FlushInput(pGame->GetWindowMgr());

			/* Show & enable */
			chatBox->enable();
			chatBox->show();

			/* Enable GUI */
			pGame->m_WindowManager.SetGUIActive(true);
			chatBox->focus();

			/* Show chat window */
			m_ChatDisplay.Show();
		}
		while (GetAsyncKeyState(VK_RETURN)){}
	}

	m_Player.Update(true);
}

void CGameView::Draw(CHotelClient& client) {
	m_Window.BeginDrawing();

	/* Apply camera transform */
	m_Window.ApplyTranslation(m_vGameCameraPoint);

	/* Draw world */
	if (client.HasRoomData()) {
		/* Room */
		m_Renderer.DrawHotelRoom(client.GetCurrentRoom(), m_Window);

		/* If in edit mode, draw selected tile */
		if (client.CanEditCurrentRoom()) {
			sf::Vector2i selectedTile = ScreenToTile(m_GameInput.GetMousePosition());
			m_Renderer.DrawSelectedTile(client.GetCurrentRoom(), m_Window, selectedTile.x, selectedTile.y);
		}

		/* Draw local player */
		m_Renderer.DrawPlayer(m_Player, client.GetLocalPosition(), m_Window);		
	}

	/* Restore camera */
	m_Window.RestoreTranslation();

	/* GUI */
	if (!client.HasRoomData()) {
		/* Draw background and 'waiting for room' message */
		CDrawing::DrawTexture(0, 0, m_MenuBackground, m_Window);
		float centreX = m_Window.GetWidth() / 2.0f;
		float centreY = m_Window.GetHeight() / 2.0f;
		m_TextFont.DrawCentred(centreX - 2.0f, centreY - 2.0f, m_Window, "WAITING FOR ROOM...", 48, sf::Color(254, 132, 25));
		m_TextFont.DrawCentred(centreX - 2.0f, centreY, m_Window, "WAITING FOR ROOM...", 48, sf::Color(254, 132, 25));
		m_TextFont.DrawCentred(centreX, centreY, m_Window, "WAITING FOR ROOM...", 48);
	}
	else {
		/* Room */
		m_TextFont.Draw(204.0f, 4.0f, m_Window, client.GetCurrentRoom()->GetName().c_str(), 16, sf::Color(254, 132, 25));

		/* Game GUI */ 
		m_pGameGUI->draw();

		/* Chat text */
		m_ChatDisplay.Draw(m_TextFont, m_Window);
	}


	m_Window.EndDrawing();
}

sf::Vector2i CGameView::ScreenToTile(sf::Vector2i mousePos) {
	/* Create game view of camera translation */
	sf::View cameraView(m_vGameCameraPoint, sf::Vector2f((float)m_Window.GetWidth(), (float)m_Window.GetHeight()));

	/* Translate screen pixel to world co-ords */
	sf::Vector2f worldPos = m_Window.GetRenderWindow().mapPixelToCoords(mousePos, cameraView);

	/* Translate world co ord to tile co-ord*/
	return m_Renderer.GetTileAtPosition(worldPos);
}

void CGameView::Shutdown() {
	m_Window.Destroy();
}