#ifndef __CINPUT_H__
#define __CINPUT_H__

/*
	Christopher De Pasquale
	22 December 2016

	Input handler 
*/
#include "SFML\Graphics.hpp"
#include <vector>

class CWindow;

/* Mouse events base class */

/* Press down mouse button */
struct SMouseEvent  {
	enum EEventType {
		TYPE_MOUSEPRESS,
		TYPE_MOUSERELEASE,
		TYPE_SCROLL,
	};
	EEventType m_eType;

	struct ButtonEvent {
		sf::Mouse::Button m_eButton;
		sf::Vector2i m_vCursorPosition;
	};
	struct ScrollEvent {
		float m_fWheelDelta;
	};

	ButtonEvent m_ButtonEvent;
	ScrollEvent m_ScrollEvent;

	static SMouseEvent CreateClickEvent(sf::Vector2i vCursorPosition, sf::Mouse::Button eButton) {
		SMouseEvent mouseEvent;
		mouseEvent.m_ButtonEvent.m_vCursorPosition = vCursorPosition;
		mouseEvent.m_ButtonEvent.m_eButton = eButton;
		mouseEvent.m_eType = TYPE_MOUSEPRESS;
		return mouseEvent;
	}
	static SMouseEvent CreateReleaseEvent(sf::Vector2i vCursorPosition, sf::Mouse::Button eButton) {
		SMouseEvent mouseEvent;
		mouseEvent.m_ButtonEvent.m_vCursorPosition = vCursorPosition;
		mouseEvent.m_ButtonEvent.m_eButton = eButton;
		mouseEvent.m_eType = TYPE_MOUSERELEASE;
		return mouseEvent;
	}
	static SMouseEvent CreateWheelEvent(float fWheelDelta) {
		SMouseEvent mouseEvent;
		mouseEvent.m_ScrollEvent.m_fWheelDelta = fWheelDelta;
		mouseEvent.m_eType = TYPE_SCROLL;
		return mouseEvent;
	}
};


class CInput {
	sf::Vector2i m_vCursorPosition;
	friend class CWindow;
	std::vector<SMouseEvent> m_MouseEvents;

	/* Maximum number of events in mouseevent queue */
	static const int MAX_MOUSEEVENTS_BUFFER_SIZE;

protected:

	void InjectMousePress(sf::Vector2i vCursorPosition, sf::Mouse::Button Button);
	void InjectMouseRelease(sf::Vector2i vCursorPosition, sf::Mouse::Button Button);
	void InjectMouseMove(sf::Vector2i vCursorPosition);
	void InjectMouseWheel(float delta);

public:

	/* Get mouse position relative to window */
	sf::Vector2i GetMousePosition() { return m_vCursorPosition; }

	/* */
	bool GetNextMouseEvent(SMouseEvent& m);
};

#endif