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

#include "logger.h"
#include "replay.h"

#include <stdio.h>
#include <string>

class FileCommandLoggerImpl :
		public CommandLogger,
		public PartialCommandObserver {
	FILE* fh;
	std::string buffer;
	bool newlineRequired;
public:
	void Close() {
		if (fh) {
			WriteBuffer();
			fclose(fh);
		}
		fh = 0;
	}
	void AppendNewline() {
		if (newlineRequired)
			buffer.push_back('\n');
	}
	void WriteBuffer() {
		AppendNewline();
		while (!buffer.empty()) {
			ssize_t s = fwrite(buffer.c_str(), 1, buffer.length(), fh);
			if (s < 0) {
				throw LoggerWriteFailedException();
			}
			buffer.erase(0, s);
		}
	}
	FileCommandLoggerImpl() : fh(0), newlineRequired(false) {
	}
	~FileCommandLoggerImpl() {
		Close();
	}
	void SetLogFile(FILE* f) {
		Close();
		fh = f;
	}
	void AtomicComplete() {
		buffer.push_back('.');
	}
	void WriteCommand(const char* c) {
		WriteBuffer();
		buffer.append(c);
		newlineRequired = true;
	}
	void CommandComplete() {
		buffer.append("!\n");
		newlineRequired = false;
		WriteBuffer();
	}
};

FileCommandLogger::FileCommandLogger() {
	pImpl = new FileCommandLoggerImpl;
}

FileCommandLogger::~FileCommandLogger() {
	delete pImpl;
}

void FileCommandLogger::SetLogFile(FILE* f) {
	pImpl->SetLogFile(f);
}

void FileCommandLogger::CommandComplete() {
	pImpl->CommandComplete();
}

PartialCommandObserver* FileCommandLogger::GetPartialCommandObserver() {
	return pImpl;
}

const PartialCommandObserver*
		FileCommandLogger::GetPartialCommandObserver() const {
	return pImpl;
}

CommandLogger* FileCommandLogger::GetLogger() {
	return pImpl;
}

const CommandLogger* FileCommandLogger::GetLogger() const {
	return pImpl;
}
