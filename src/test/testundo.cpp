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

#include "testundo.h"

#include "src/domain/undo/executor.h"
#include "src/domain/undo/commandlogger.h"
#include "src/domain/undo/filelogger.h"
#include "src/domain/undo/random.h"

#include <sstream>
#include <stdio.h>

#include <QtTest/QtTest>

class StringLoggerWrapper : public CommandLogger {
	CommandLogger* delegate;
	std::string* out;
public:
	StringLoggerWrapper(std::string* output) : delegate(0), out(output) {
	}
	/**
	 * Create a logger that writes to a std::string and also passes writes
	 * along to @c wrapped.
	 * @param wrapped Ownership is not passed.
	 * @param output The string to be logged to. Ownership is not passed.
	 */
	StringLoggerWrapper(std::string* output, CommandLogger* wrapped)
			: delegate(wrapped), out(output) {
	}
	StringLoggerWrapper(const StringLoggerWrapper&); // unimplemented
	StringLoggerWrapper& operator=(const StringLoggerWrapper&);// unimplemented
	~StringLoggerWrapper() {
	}
	void WriteCommand(const char* command) {
		if (out) {
			out->append(command);
			out->append(1, '\n');
		}
		if (delegate)
			delegate->WriteCommand(command);
	}
	void CommandComplete() {
		if (delegate)
			delegate->CommandComplete();
	}
	void SetDelegate(CommandLogger* newLogger) {
		delegate = newLogger;
	}
};

ModelTestHelper::~ModelTestHelper() {
}

Hash::Hash() {
	h = 5381;
}

void Hash::Add(uint64_t n) {
	h = h * 33 + n;
}

void Hash::AddS(int64_t n) {
	Add(static_cast<uint64_t>(n));
}

void Hash::Add(const char* string) {
	while (*string) {
		Add(static_cast<uint64_t>(*string));
		++string;
	}
}

void Hash::Add(Hash h) {
	Add(h.h);
}

bool Hash::Equals(const Hash& other) const {
	return h == other.h;
}

bool operator==(const Hash& a, const Hash& b) {
	return a.Equals(b);
}

bool operator!=(const Hash& a, const Hash& b) {
	return !a.Equals(b);
}

bool ExecuteLineFromFile(Executor& e, FILE* logFile) {
	enum { lineBufferSize = 1024 };
	static char lineBuffer[lineBufferSize];
	if (!fgets(lineBuffer, lineBufferSize, logFile))
		return false;
	e.ExecuteFromLog(lineBuffer);
	return true;
}

void TestUndo(Executor& e, ModelTestHelper& helper) {
	FileCommandLogger fileLogger;
	std::string logString;
	StringLoggerWrapper stringLogger(&logString);
	FILE* logFile = tmpfile();
	// ownership of logFile is passed here
	fileLogger.SetLogFile(logFile);
	RandomSource rng;
	RandomSource rng2(rng);
	for (int i = 0; i != 100; ++i) {
		// get hashes for initial and final states
		// and logs to say what was done
		logString.clear();
		stringLogger.SetDelegate(0);
		e.SetCommandLogger(&stringLogger);
		helper.ResetModel(e);
		e.ExecuteRandomConstructiveCommands(rng);
		std::string constructLog(logString);
		logString.clear();
		e.ClearHistory();
		Hash initialState(helper.HashModel(e));
		stringLogger.SetDelegate(fileLogger.GetLogger());
		e.ExecuteRandomCommands(rng);
		Hash finalState(helper.HashModel(e));
		std::string doLog(logString);
		e.SetCommandLogger(0);
		rewind(logFile);

		// recreate the initial state and see if executing from
		// the log file produces the same output
		helper.ResetModel(e);
		e.ExecuteRandomConstructiveCommands(rng2);
		e.ClearHistory();
		while (ExecuteLineFromFile(e, logFile)) {
		}
		if (helper.HashModel(e) != finalState) {
			std::stringstream ss;
			ss << "Failed to replay to final state on test "
					<< i << "[construction commands:\n"
					<< constructLog
					<< "do commands:\n"	<< doLog << "]\n";
			QFAIL(ss.str().c_str());
		}

		// test Undo and Redo on the same data
		while (e.Undo()) {
		}
		if (helper.HashModel(e) != initialState) {
			std::stringstream ss;
			ss << "Failed to undo to initial state on test "
					<< i << "[construction commands:\n"
					<< constructLog
					<< "do commands:\n"	<< doLog << "]\n";
			QFAIL(ss.str().c_str());
		}
		while (e.Redo()) {
		}
		if (helper.HashModel(e) != finalState) {
			std::stringstream ss;
			ss << "Failed to redo to final state on test "
					<< i << "[construction commands:\n"
					<< constructLog
					<< "do commands:\n"	<< doLog << "]\n";
			QFAIL(ss.str().c_str());
		}
	}
}
