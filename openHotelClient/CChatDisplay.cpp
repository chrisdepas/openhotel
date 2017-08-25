#include "stdafx.h"
#include "CChatDisplay.h"
#include <stdlib.h>
#include "CFont.h"
#include "CWindow.h"

CChatDisplay::~CChatDisplay() {
	for (int i = 0; i < m_iNumMessages; i++)
	{
		free(m_pszMessageBuffer[i]);
	}
}
void CChatDisplay::Draw(CFont& font, CWindow& window)
{
	if (m_fTimeSinceLastShown >= CHATLIST_END_FADE_TIME)
		return;

	int alpha = 255;
	if (m_fTimeSinceLastShown > CHATLIST_BEGIN_FADE_TIME) {
		float fFadeTime = CHATLIST_END_FADE_TIME - CHATLIST_BEGIN_FADE_TIME;
		alpha = (int)(255.0f * (1.0f - ((m_fTimeSinceLastShown - CHATLIST_BEGIN_FADE_TIME) / fFadeTime)));
	}
	int yoffset = 0;
	for (int i = 0; i < m_iNumMessages; i++)
	{
		font.Draw(-2, yoffset + 1, window, m_pszMessageBuffer[i], m_iFontSize, sf::Color(0, 0, 0, alpha) );
		font.Draw( 0, yoffset    , window, m_pszMessageBuffer[i], m_iFontSize, sf::Color(255, 255, 255, alpha));
		yoffset += m_iFontSize;
	}
}

void CChatDisplay::AddMessage(char* szMessage)
{
	if (!szMessage)
		return;

	m_fTimeSinceLastShown = 0.0f;

	/* We have room, add message to queue */
	if (m_iNumMessages < iChatlistBufferSize)
	{
		int slen = strlen(szMessage);
		m_pszMessageBuffer[m_iNumMessages] = (char*)malloc(slen + 1);
		memcpy(m_pszMessageBuffer[m_iNumMessages], szMessage, slen);
		m_pszMessageBuffer[m_iNumMessages][slen] = 0;
		m_iNumMessages++;
		return;
	}

	/* Already have max number of messages, delete oldest one */
	free(m_pszMessageBuffer[0]);

	/* Shift down */
	for (int i = 0; i < m_iNumMessages - 1; i++)
	{
		m_pszMessageBuffer[i] = m_pszMessageBuffer[i + 1];
	}

	/* Decrement amount of messages - one has been removed */
	m_iNumMessages--;

	/* Add message to queue */
	AddMessage(szMessage);
}

void CChatDisplay::Update(float fFrameTime) {
	if (m_fTimeSinceLastShown < CHATLIST_END_FADE_TIME)
		m_fTimeSinceLastShown += fFrameTime;
}
void CChatDisplay::Show() {
	m_fTimeSinceLastShown = 0.0f;
}