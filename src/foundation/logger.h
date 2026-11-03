/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef LOGGER_H
#define LOGGER_H

class Logger
{
public:
	static Logger get();
	static void logDebug(const char * msg, ...);
	static void logWarning(const char * msg, ...);
	static void logFatal(const char * msg, ...);
	
private:
	static Logger logger;
};

#endif
