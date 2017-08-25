#ifndef __CDEBUGLOGGER_H__
#define __CDEBUGLOGGER_H__

/* Enable this to show windows message box on error */
//#define __DEBUGLOGGER_SHOW_MSGBOX__

/*
	Christopher De Pasquale
	November 2016
*/

/*
USAGE:
Implemented as singleton, use Instance()
Set verbosity levels with either SetVerbosityRange or SetVerbosityFlag
Then write to the logger using named parameter idiom, i.e. Instance().LogLevel(level).Log(data);

QOL functions DEC 2016
static void LogDebug(char* format, ...);
static void LogInfo(char* format, ...);
static void LogWarning(char* format, ...);
static void LogError(char* format, ...);
static void LogFatal(char* format, ...);



VERBOSITY LEVEL DESCRIPTION:

'Rule of thumb' for logging levels from http://stackoverflow.com/questions/7839565/logging-levels-logback-rule-of-thumb-to-assign-log-levels
Credit goes to user Tom Anderson, with modifications by Chris De Pasquale

HIGH SEVERITY
/ \ 									FATAL
|		Something very unexpected happened that means that the program must terminate
|		straight away. This is VERY BAD, and should probably be enabled always.
|
|										ERROR
|		Execution of some task could not be completed; an email couldn't be sent,
a page couldn't be rendered, some data couldn't be stored to a database,
something like that. Something has definitively _GONE WRONG_, but the program
MIGHT be able to continue (i.e. execution isn't terminated), generally some
core functionality will probably stop working.

WARNING
Something unexpected happened, but that execution can definately continue,
in a degraded mode; a configuration file was missing but defaults were
used, a price was calculated as negative, so it was clamped to zero, etc.
Something is not right, but it hasn't gone properly wrong yet - warnings
are often a sign that there will be an error very soon.

INFO
something normal but significant happened; the system started, the system
stopped, the daily inventory update job ran, etc. There shouldn't be a
continual torrent of these, otherwise there's just too much to read.

|										DEBUG
|		Debug means that something normal and insignificant happened; a new
|		user came to the site, a page was rendered, an order was taken, a price
|		was updated. This is the stuff excluded from info because there would be
|		too much of it, but would be useful in tracking a bug down, e.g. the
|		value of a variable, or when a function is called
\ /
LOW SEVERITY




*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>

class CDebugLogger {
public:
	enum ELoggingLevel {
		LOGLEVEL_DEBUG = 1 << 0,
		LOGLEVEL_INFO = 1 << 1,
		LOGLEVEL_WARNING = 1 << 2,
		LOGLEVEL_ERROR = 1 << 3,
		LOGLEVEL_FATAL = 1 << 4,
		LOGLEVEL_DISABLED = 1 << 5,
	};

private:
	int m_iVerbosityFlags;

	/* Output file path */
	std::string m_sLogFile;

	/* Output info */
	bool m_bStdout;	// Output to stdout
	bool m_bFile;   // output to file
	bool m_bLoggingEnabled; // Should we output at all 
	bool m_bPrintLogLevel;	// Prepend loglevel as string 
	bool m_bWriteLogLevel; // Write logleve prefix to file 

	/* Last logging level set, used for named-parameter idiom */
	ELoggingLevel m_eLastLogLevel;

	/* Singleton - don't implement publicly */
	CDebugLogger();

	/* Internal - Get string for log level prefix */
	char* GetPrefixString();

public:


	/* Set current logging level */
	CDebugLogger& LogLevel(ELoggingLevel level = LOGLEVEL_DEBUG);	// Named-Parameter idiom	

	/* Print to log */
	void Log(char* format, ...);
	
	/* Print to log without formatting */
	void LogUnformatted(char* szLog);

	/* Set a range of verbosity levels */
	void SetVerbosityRange(ELoggingLevel Start, ELoggingLevel end = LOGLEVEL_DEBUG);

	/* Enable or disable specific logging levels */
	void SetVerbosityFlag(ELoggingLevel level, bool bEnabled = true);

	/* Set a file to write to log */
	void SetLogFile(char* szLogFile);

	/* Enable logging to stdout */
	void EnableStdoutLog();

	/* Disable logging to stdout */
	void DisableStdoutLog();

	/* Disable logging to file */
	void DisableLogFile();

	/* Enable logging */
	void Enable();

	/* Disable logging */
	void Disable();

	/* QOL functions */
	static void LogDebug(char* format, ...);
	static void LogInfo(char* format, ...);
	static void LogWarning(char* format, ...);
	static void LogError(char* format, ...);
	static void LogFatal(char* format, ...);

	/* Singleton */
	static CDebugLogger* Instance() {
		static CDebugLogger logger;
		return &logger;
	}
};
#endif