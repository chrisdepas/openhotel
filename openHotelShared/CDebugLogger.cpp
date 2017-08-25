/*
	Christopher De Pasquale
	November 2016
*/


#include "stdafx.h"
#include "CDebugLogger.h"
#include <fstream>
#include <cstdarg> 


char* CDebugLogger::GetPrefixString() {
	switch (m_eLastLogLevel) {
	case LOGLEVEL_DEBUG:
		return "[DEBUG]";
	case LOGLEVEL_INFO:
		return "[INFO]";
	case LOGLEVEL_WARNING:
		return "[WARNING]";
	case LOGLEVEL_ERROR:
		return "[ERROR]";
	case LOGLEVEL_FATAL:
		return "[FATAL]";
	default:
		return "";
	}
}

CDebugLogger::CDebugLogger() {
	/* Default to fatal, error & warning */
	m_iVerbosityFlags = LOGLEVEL_FATAL | LOGLEVEL_ERROR | LOGLEVEL_WARNING;

	m_bFile = false;
	m_sLogFile = "";

	/* Default print to stdout */
	m_bStdout = true;
	m_bLoggingEnabled = true;

	/* Default disable loglevel printing*/
	m_bPrintLogLevel = false;

	m_eLastLogLevel = LOGLEVEL_WARNING;
}

// Named-Parameter idiom	
CDebugLogger& CDebugLogger::LogLevel(ELoggingLevel level) {
	m_eLastLogLevel = level;
	return *this;
}

void CDebugLogger::Log(char* format, ...) {
	if (!m_bLoggingEnabled)
		return;

	/* Init buffer as formatted string */
	char logOutputBuf[512];
	va_list args;
	va_start(args, format);
	int result = vsprintf_s(logOutputBuf, 512, format, args);
	va_end(args);

	if (result < 0) {
#ifdef __DEBUGLOGGER_SHOW_MSGBOX__
		char errorBuf[256];
		if (sprintf_s(errorBuf, 256, "CDebugLogger::Log failed\n", m_sLogFile.c_str()) > 0) {
			MessageBoxA(0, errorBuf, "LOGGER ERROR", 0);
		}
#endif
		return;
	}

	/* Write to log */
	LogUnformatted(logOutputBuf);
}

void CDebugLogger::LogUnformatted(char* szLog) {
	if (!m_bLoggingEnabled)
		return;

	/* Handle file logging */
	if (m_bFile) {
		std::ofstream fstream;
		fstream.open(m_sLogFile, std::ofstream::app);

		if (fstream.bad()) {
			/* Disable file output */
			m_bFile = false;
			
#ifdef __DEBUGLOGGER_SHOW_MSGBOX__
			/* Show messagebox */
			char errorBuf[256];
			if (sprintf_s(errorBuf, 256, "Unable to write to file' %s'. file output has been disabled", m_sLogFile.c_str()) > 0) {
				MessageBoxA(0, errorBuf, "LOGGER ERROR", 0);
			}
#endif
			return;
		}
		/* Prefix */
		if (m_bWriteLogLevel) {
			fstream.write(GetPrefixString(), strlen(GetPrefixString()));
		}
		/* Write to file*/
		fstream.write(szLog, strlen(szLog));

		fstream.close();
	}

	/* Handle stdout logging */
	if (m_bStdout) {
		fputs(szLog, stdout);
	}
}

void CDebugLogger::LogDebug(char* format, ...) {
	/* Write formatted string */
	char logOutputBuf[512];
	va_list args;
	va_start(args, format);
	int result = vsprintf_s(logOutputBuf, 512, format, args);
	va_end(args);

	/* Set debug logging mode */
	Instance()->LogLevel();

	/* Log debug string */
	Instance()->Log(logOutputBuf);
}
void CDebugLogger::LogInfo(char* format, ...) {
	/* Write formatted string */
	char logOutputBuf[512];
	va_list args;
	va_start(args, format);
	int result = vsprintf_s(logOutputBuf, 512, format, args);
	va_end(args);

	/* Set info logging mode */
	Instance()->LogLevel(CDebugLogger::LOGLEVEL_INFO);

	/* Log debug string */
	Instance()->Log(logOutputBuf);
}
void CDebugLogger::LogWarning(char* format, ...) {
	/* Write formatted string */
	char logOutputBuf[512];
	va_list args;
	va_start(args, format);
	int result = vsprintf_s(logOutputBuf, 512, format, args);
	va_end(args);

	/* Set warning logging mode */
	Instance()->LogLevel(CDebugLogger::LOGLEVEL_WARNING);

	/* Log debug string */
	Instance()->Log(logOutputBuf);
}
void CDebugLogger::LogError(char* format, ...) {
	/* Write formatted string */
	char logOutputBuf[512];
	va_list args;
	va_start(args, format);
	int result = vsprintf_s(logOutputBuf, 512, format, args);
	va_end(args);

	/* Set error logging mode */
	Instance()->LogLevel(CDebugLogger::LOGLEVEL_ERROR);

	/* Log debug string */
	Instance()->Log(logOutputBuf);
}
void CDebugLogger::LogFatal(char* format, ...) {
	/* Write formatted string */
	char logOutputBuf[512];
	va_list args;
	va_start(args, format);
	int result = vsprintf_s(logOutputBuf, 512, format, args);
	va_end(args);

	/* Set fatal logging mode */
	Instance()->LogLevel(CDebugLogger::LOGLEVEL_FATAL);

	/* Log debug string */
	Instance()->Log(logOutputBuf);
}

void CDebugLogger::SetVerbosityRange(ELoggingLevel Start, ELoggingLevel end) {
	/* Clear flags */
	m_iVerbosityFlags = 0;

	/* Add flags for each level in range */
	if (Start >= LOGLEVEL_FATAL && end <= LOGLEVEL_FATAL)
		m_iVerbosityFlags |= LOGLEVEL_FATAL;
	if (Start >= LOGLEVEL_ERROR && end <= LOGLEVEL_ERROR)
		m_iVerbosityFlags |= LOGLEVEL_ERROR;
	if (Start >= LOGLEVEL_WARNING && end <= LOGLEVEL_WARNING)
		m_iVerbosityFlags |= LOGLEVEL_WARNING;
	if (Start >= LOGLEVEL_INFO && end <= LOGLEVEL_INFO)
		m_iVerbosityFlags |= LOGLEVEL_INFO;
	if (Start >= LOGLEVEL_DEBUG && end <= LOGLEVEL_DEBUG)
		m_iVerbosityFlags |= LOGLEVEL_DEBUG;
}

void CDebugLogger::SetVerbosityFlag(ELoggingLevel level, bool bEnabled) {
	if (bEnabled) {
		m_iVerbosityFlags |= level;
	}
	else {
		m_iVerbosityFlags &= ~level;
	}
}

void CDebugLogger::SetLogFile(char* szLogFile) {
	m_sLogFile = szLogFile;
	m_bFile = true;

	return;
	std::ofstream fstream;
	fstream.open(m_sLogFile);
	if (!fstream.fail() && fstream.is_open() && !fstream.bad())
		fstream.close();
}

void CDebugLogger::EnableStdoutLog() {
	m_bStdout = true;
}

void CDebugLogger::DisableStdoutLog() {
	m_bStdout = false;
}

void CDebugLogger::DisableLogFile() {
	m_bFile = false;
}

void CDebugLogger::Enable() {
	m_bLoggingEnabled = true;
}

void CDebugLogger::Disable() {
	m_bLoggingEnabled = false;
}