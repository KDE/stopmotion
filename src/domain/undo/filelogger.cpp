/*
 * SPDX-FileCopyrightText: 2013-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filelogger.h"
#include "commandlogger.h"

#include <cstdio>
#include <cstring>
#include <string>

class FileCommandLoggerImpl : public CommandLogger {
	FILE* fh;
	std::string buffer;
	std::string::size_type committedUpTo;
public:
	FileCommandLoggerImpl(const FileCommandLoggerImpl&) = delete;
	FileCommandLoggerImpl& operator=(const FileCommandLoggerImpl&) = delete;
	void close() {
		if (fh) {
			flush();
			fclose(fh);
		}
		fh = nullptr;
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
	FileCommandLoggerImpl() : fh(nullptr), committedUpTo(0) {
	}
	~FileCommandLoggerImpl() override {
		// A destructor must not throw; if the final flush fails the
		// uncommitted commands are lost either way.
		try {
			close();
		} catch (...) {
			if (fh)
				fclose(fh);
		}
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

const char* LoggerWriteFailedException::what() const noexcept {
	return msg;
}
