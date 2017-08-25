#include "stdafx.h"
#include "CWindow.h"
#include "CInput.h"

bool CWindow::CreateRenderWindow(CClientSettings& settings, std::string& sTitle) {
	if (m_bWindowCreated)
		return true;

	m_vSize.x = (float)settings.GetWindowSize().x;
	m_vSize.y = (float)settings.GetWindowSize().y;
	m_iWidth = settings.GetWindowSize().x;
	m_iHeight = settings.GetWindowSize().y;

	
	/* Create window */
	m_Window.create(sf::VideoMode(m_iWidth, m_iHeight), sTitle, sf::Style::Titlebar | sf::Style::Close);

	return (m_bWindowCreated = true);
}

void CWindow::Update(CInput& input) {
	/* Handle window/input events */
	sf::Event e;
	while (m_Window.pollEvent(e))
	{
		/* GUI input */
		if (m_bGUIActive && m_pGUI) {
			m_pGUI->handleEvent(e);
		}

		switch (e.type)
		{
		case sf::Event::Closed:
			m_bWindowClosed = true;
			break;

		case sf::Event::GainedFocus:
			m_bFocus = true;
			break;

		case sf::Event::LostFocus:
			m_bFocus = false;
			break;

		case sf::Event::MouseMoved:
			if (HasFocus()) {
				input.InjectMouseMove(sf::Vector2i(e.mouseMove.x, e.mouseMove.y));
			}
			break;

		case sf::Event::MouseButtonPressed:
			if (HasFocus()) {
				input.InjectMousePress(sf::Vector2i(e.mouseButton.x, e.mouseButton.y), e.mouseButton.button);
			}
			break;

		case sf::Event::MouseButtonReleased:
			if (HasFocus()) {
				input.InjectMouseRelease(sf::Vector2i(e.mouseButton.x, e.mouseButton.y), e.mouseButton.button);
			}
			break;

		case sf::Event::MouseWheelScrolled:
			if (HasFocus()) {
				input.InjectMouseWheel(e.mouseWheelScroll.delta);
			}

			break;
		}
	}
}


void CWindow::BeginDrawing() {
	if (m_bWindowCreated) {
		m_Window.clear(m_ClearColor);
	}
}

void CWindow::EndDrawing() {
	if (m_bWindowCreated) {
		m_Window.display();
	}
}

void CWindow::Destroy() {
	m_Window.close();
}
void CWindow::RestoreTranslation()
{
	m_Window.setView(m_Window.getDefaultView());
}
void CWindow::ApplyTranslation(sf::Vector2f vTranslation)
{
	m_Window.setView(sf::View(vTranslation, m_vSize));
}
void CWindow::Draw(sf::Drawable& item)
{
	m_Window.draw(item);
}
