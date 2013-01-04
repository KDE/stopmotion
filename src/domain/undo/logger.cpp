/*
 * logger.cpp
 *
 *  Created on: 13 Dec 2012
 *      Author: tim
 */

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
