/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
 *   see the AUTHORS file for details.                                     *
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

#include "filelogger.h"
#include "commandlogger.h"
#include "executor.h"

#include <stdio.h>
#include <string.h>
#include <string>

class FileCommandLoggerImpl :
		public CommandLogger {
	enum Ending {
		noEnding, newline, bangNewline
	};
	FILE* fh;
	std::string buffer;
	Ending endingRequired;
public:
	void close() {
		if (fh) {
			writeBuffer();
			fclose(fh);
		}
		fh = 0;
	}
	void appendNewline() {
		switch (endingRequired) {
		case noEnding:
			break;
		case newline:
			buffer.push_back('\n');
			break;
		case bangNewline:
			buffer.append("!\n");
			break;
		}
	}
	void writeBuffer() {
		appendNewline();
		while (!buffer.empty()) {
			ssize_t s = fwrite(buffer.c_str(), 1, buffer.length(), fh);
			if (s <= 0) {
				throw LoggerWriteFailedException(ferror(fh));
			}
			buffer.erase(0, s);
		}
	}
	FileCommandLoggerImpl() : fh(0), endingRequired(noEnding) {
	}
	~FileCommandLoggerImpl() {
		close();
	}
	void setLogFile(FILE* f) {
		close();
		fh = f;
	}
	void writeCommand(const char* c) {
		writeBuffer();
		buffer.append(c);
		endingRequired = newline;
	}
	void commandComplete() {
		if (endingRequired != noEnding) {
			endingRequired = bangNewline;
			buffer.append("!\n");
			endingRequired = noEnding;
		}
		writeBuffer();
		fflush(fh);
	}
};

FileCommandLogger::FileCommandLogger() {
	pImpl = new FileCommandLoggerImpl;
}

FileCommandLogger::~FileCommandLogger() {
	delete pImpl;
}

void FileCommandLogger::setLogFile(FILE* f) {
	pImpl->setLogFile(f);
}

CommandLogger* FileCommandLogger::getLogger() {
	return pImpl;
}

const CommandLogger* FileCommandLogger::getLogger() const {
	return pImpl;
}

LoggerWriteFailedException::LoggerWriteFailedException(int error) {
	snprintf(msg, sizeof(msg),
			"Failed to write to command logger (%s)!\n"
			"Disaster recovery will be impossible after this point!",
			strerror(error));
}

const char* LoggerWriteFailedException::what() const throw() {
	return msg;
}
