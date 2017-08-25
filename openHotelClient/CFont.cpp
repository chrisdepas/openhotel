#include "stdafx.h"
#include "CFont.h"
#include <stdlib.h>
#include <stdarg.h>
#include "CDebugLogger.h"
#define CFONT_FORMATTED_BUFFER_SIZE 1024


bool CFont::Initialise() {
	return m_Font.loadFromFile(m_sFontFile);
}

void CFont::Draw(float x, float y, CWindow& window, std::string string, int iSize, sf::Color color) {
	sf::Text text;
	text.setFont(m_Font);
	text.setString(string);
	text.setCharacterSize(iSize);
	text.setColor(color);
	text.setPosition(x, y);
	
	window.Draw(text);
}
void CFont::DrawCentred(float x, float y, CWindow& window, std::string string, int iSize, sf::Color color) {
	sf::Text text;
	text.setFont(m_Font);
	text.setString(string);
	text.setCharacterSize(iSize);
	text.setColor(color);
	text.setPosition(x - text.getLocalBounds().width / 2.0f, y);

	window.Draw(text);
}
void CFont::DrawFormatted(float x, float y, CWindow& window, char* szFormat, int iSize, sf::Color color, ...) {
	/* Write formatted string to buffer */
	char buffer[CFONT_FORMATTED_BUFFER_SIZE];
	va_list args;
	va_start(args, szFormat);
	int result = vsprintf_s(buffer, CFONT_FORMATTED_BUFFER_SIZE, szFormat, args);
	if (result <= 0) {
		CDebugLogger::LogError("Unable to write formatted string - vsprintf_s failed\n");
		return;
	}
	va_end(args);

	/* Draw buffer text */
	Draw(x, y, window, buffer, iSize, color);
}
void CFont::DrawFormattedCentred(float x, float y, CWindow& window, char* szFormat, int iSize, sf::Color color, ...) {
	/* Write formatted string to buffer */
	char buffer[CFONT_FORMATTED_BUFFER_SIZE];
	va_list args;
	va_start(args, szFormat);
	int result = vsprintf_s(buffer, CFONT_FORMATTED_BUFFER_SIZE, szFormat, args);
	if (result <= 0) {
		CDebugLogger::LogError("Unable to write formatted string ");
		return;
	}
	va_end(args);

	/* Draw buffer text */
	DrawCentred(x, y, window, buffer, iSize, color);
}