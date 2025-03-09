/***************************************************************************
 *   Copyright (C) 2013-2017 by Linuxstopmotion contributors;              *
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

#include <stdio.h>
#include <string.h>
#include <string>

class FileCommandLoggerImpl : public CommandLogger {
	FILE* fh;
	std::string buffer;
	std::string::size_type committedUpTo;
public:
	void close() {
		if (fh) {
			flush();
			fclose(fh);
		}
		fh = 0;
	}
	void flush() override {
		if (!fh) {
			buffer.erase(0, committedUpTo);
			committedUpTo = 0;
			return;
		}
		while (0 < committedUpTo) {
			ssize_t s = fwrite(buffer.c_str(), 1, committedUpTo, fh);
			if (s <= 0) {
				throw LoggerWriteFailedException(ferror(fh));
			}
			buffer.erase(0, s);
			committedUpTo -= s;
		}
		fflush(fh);
	}
	FileCommandLoggerImpl() : fh(0), committedUpTo(0) {
	}
	~FileCommandLoggerImpl() override {
		close();
	}
	void setLogFile(FILE* f) {
		close();
		fh = f;
	}
	FILE* getLogFile() const {
		return fh;
	}
	void deleteUncommitted() {
		buffer.resize(committedUpTo);
	}
	void writePendingCommand(const char* text) override {
		deleteUncommitted();
		buffer.append(text);
		buffer.append("!\n");
	}
	void writePendingUndo() override {
		deleteUncommitted();
		buffer.append("?\n");
	}
	void writePendingRedo() override {
		deleteUncommitted();
		buffer.append("!\n");
	}
	void commit() override {
		committedUpTo = buffer.length();
		flush();
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

FILE* FileCommandLogger::getLogFile() const {
	return pImpl->getLogFile();
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
