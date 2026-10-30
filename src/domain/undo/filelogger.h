/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILELOGGER_H_
#define FILELOGGER_H_

#include <cstdio>
#include <exception>

class FileCommandLoggerImpl;
class CommandLogger;

class LoggerWriteFailedException : public std::exception {
	char msg[400];
public:
	LoggerWriteFailedException(int error);
	const char* what() const noexcept override;
};

/**
 * Logs commands to a file
 */
class FileCommandLogger {
	FileCommandLoggerImpl* pImpl;
public:
	FileCommandLogger();
	~FileCommandLogger();
	FileCommandLogger(const FileCommandLogger&) = delete;
	FileCommandLogger& operator=(const FileCommandLogger&) = delete;
	void setLogFile(FILE* fileHandle);
	FILE* getLogFile() const;
	CommandLogger* getLogger();
	const CommandLogger* getLogger() const;
};

#endif /* FILELOGGER_H_ */
