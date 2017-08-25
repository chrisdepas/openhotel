#include "stdafx.h"
#include "CInput.h"

const int CInput::MAX_MOUSEEVENTS_BUFFER_SIZE = 16;
/*
SMouseReleaseEvent::SMouseReleaseEvent(sf::Vector2i vCursorPosition, sf::Mouse::Button eButton) {
	m_vCursorPosition = vCursorPosition;
	m_eButton = eButton;
	m_eType = TYPE_MOUSERELEASE;
}

SMouseWheelEvent::SMouseWheelEvent(float fWheelDelta) {
	m_fWheelDelta = fWheelDelta;
	m_eType = TYPE_SCROLL;
}

*/
void CInput::InjectMousePress(sf::Vector2i vCursorPosition, sf::Mouse::Button button) {
	/* Discard old queue items if buffer exceeds max */
	if (m_MouseEvents.size() >= MAX_MOUSEEVENTS_BUFFER_SIZE)
		m_MouseEvents.clear();
	/* Add event to queue */
	m_MouseEvents.push_back(SMouseEvent::CreateClickEvent(vCursorPosition, button));
}

void CInput::InjectMouseRelease(sf::Vector2i vCursorPosition, sf::Mouse::Button button) {
	/* Discard old queue items if buffer exceeds max */
	if (m_MouseEvents.size() >= MAX_MOUSEEVENTS_BUFFER_SIZE)
		m_MouseEvents.clear();
	/* Add event to queue */
	m_MouseEvents.push_back(SMouseEvent::CreateReleaseEvent(vCursorPosition, button));
}

void CInput::InjectMouseMove(sf::Vector2i vCursorPosition) {
	m_vCursorPosition = vCursorPosition;
}

void CInput::InjectMouseWheel(float delta) {
	/* Discard old queue items if buffer exceeds max */
	if (m_MouseEvents.size() >= MAX_MOUSEEVENTS_BUFFER_SIZE)
		m_MouseEvents.clear();
	/* Add event to queue */
	m_MouseEvents.push_back(SMouseEvent::CreateWheelEvent(delta));
}

bool CInput::GetNextMouseEvent(SMouseEvent& m) {
	if (m_MouseEvents.empty())
		return false;
	m = m_MouseEvents.back();
	m_MouseEvents.pop_back();
	return true;
}