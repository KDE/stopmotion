/*   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef LOGGER_H_
#define LOGGER_H_

#include "command.h"

#include <iosfwd>

class FileCommandLoggerImpl;
class PartialCommandObserver;
class CommandReplayer;
class CommandLogger;

class LoggerWriteFailedException {
};

/**
 * Logs commands to a file
 */
class FileCommandLogger {
	FileCommandLoggerImpl* pImpl;
public:
	FileCommandLogger();
	~FileCommandLogger();
	void SetLogFile(FILE* fileHandle);
	void CommandComplete();
	PartialCommandObserver* GetPartialCommandObserver();
	const PartialCommandObserver* GetPartialCommandObserver() const;
	CommandLogger* GetLogger();
	const CommandLogger* GetLogger() const;
};

#endif /* LOGGER_H_ */
