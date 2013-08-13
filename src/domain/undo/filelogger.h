/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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

#ifndef FILELOGGER_H_
#define FILELOGGER_H_

#include "command.h"

#include <iosfwd>

class FileCommandLoggerImpl;
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
	FileCommandLogger(const FileCommandLogger&); // unimplemented
	FileCommandLogger& operator=(const FileCommandLogger&); //unimplemented
	void setLogFile(FILE* fileHandle);
	CommandLogger* getLogger();
	const CommandLogger* getLogger() const;
};

#endif /* FILELOGGER_H_ */
