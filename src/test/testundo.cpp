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

#include "testundo.h"

#include "src/test/oomtestutil.h"

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
	 * along to @a wrapped.
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
	void writeCommand(const char* command) {
		if (out) {
			out->append(command);
			out->append(1, '\n');
		}
		if (delegate)
			delegate->writeCommand(command);
	}
	void commandComplete() {
		if (delegate)
			delegate->commandComplete();
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

void Hash::add(uint64_t n) {
	h = h * 33 + n;
}

void Hash::addS(int64_t n) {
	add(static_cast<uint64_t>(n));
}

void Hash::add(const char* string) {
	while (*string) {
		add(static_cast<uint64_t>(*string));
		++string;
	}
	// Add the null on the end so that strings can be added one after another
	// without the risk of, for example "abc" "def" aliasing "abcd" "ef".
	add(0ull);
}

void Hash::add(Hash h) {
	add(h.h);
}

bool Hash::equals(const Hash& other) const {
	return h == other.h;
}

bool operator==(const Hash& a, const Hash& b) {
	return a.equals(b);
}

bool operator!=(const Hash& a, const Hash& b) {
	return !a.equals(b);
}

bool executeLineFromFile(Executor& e, FILE* logFile) {
	enum { lineBufferSize = 1024 };
	static char lineBuffer[lineBufferSize];
	if (!fgets(lineBuffer, lineBufferSize, logFile))
		return false;
	e.executeFromLog(lineBuffer);
	return true;
}

void testUndo(Executor& e, ModelTestHelper& helper) {
	const int testCount = 200;
	const int firstPhaseEnds = testCount / 2;
	const int secondPhaseEnds = (testCount * 3) / 4;
	bool oomLoaded = loadOomTestUtil();
	QVERIFY2(oomLoaded, "Oom Test Util not loaded!");
	std::string logString;
	StringLoggerWrapper stringLogger(&logString);
	for (int i = 0; i != testCount; ++i) {
		int minCommands = 1;
		int maxCommands = 40;
		if (i < secondPhaseEnds) {
			if (i < firstPhaseEnds) {
				maxCommands = 1;
			} else {
				minCommands = maxCommands = 2;
			}
		}
		cancelAnyMallocFailure();
		Hash initialState;
		Hash finalState;
		std::string constructLog;
		std::string doLog;
		long doMallocCount;
		long totalMallocCount;
		const RandomSource initial;
		RandomSource generalRng;
		{
			// get hashes for initial and final states
			// and logs to say what was done
			RandomSource rng(initial);
			FileCommandLogger fileLogger;
			FILE* logFile = tmpfile();
			// ownership of logFile is passed here
			fileLogger.setLogFile(logFile);
			logString.clear();
			stringLogger.SetDelegate(0);
			e.setCommandLogger(&stringLogger);
			helper.resetModel(e);
			long startDoMallocCount = mallocsSoFar();
			e.executeRandomConstructiveCommands(rng);
			doMallocCount = mallocsSoFar() - startDoMallocCount;
			constructLog = logString;
			if (constructLog.empty()) {
				constructLog = "<no commands>\n";
			}
			logString.clear();
			e.clearHistory();
			initialState = helper.hashModel(e);
			stringLogger.SetDelegate(fileLogger.getLogger());
			e.executeRandomCommands(rng, minCommands, maxCommands);
			finalState = helper.hashModel(e);
			doLog = logString;
			if (doLog.empty()) {
				doLog = "<no commands>\n";
			}
			stringLogger.SetDelegate(0);
			// recreate the initial state and see if executing from
			// the log file produces the same output
			RandomSource rng2(initial);
			e.setCommandLogger(0);
			logFile = freopen(0, "r", logFile);
			helper.resetModel(e);
			e.executeRandomConstructiveCommands(rng2);
			e.clearHistory();
			bool failed = false;
			try {
				while (executeLineFromFile(e, logFile)) {
				}
			} catch (ParametersOutOfRangeException&) {
				failed = true;
			}
			if (failed || helper.hashModel(e) != finalState) {
				std::stringstream ss;
				ss << "Failed to replay to final state on test "
						<< i << "\n[construction commands:\n"
						<< constructLog
						<< "do commands:\n"	<< doLog << "]";
				QFAIL(ss.str().c_str());
			}
			// test Undo and Redo on the same data
			long startUndoMallocCount = mallocsSoFar();
			while (e.undo()) {
			}
			long undoMallocCount = mallocsSoFar() - startUndoMallocCount;
			if (helper.hashModel(e) != initialState) {
				std::stringstream ss;
				ss << "Failed to undo to initial state on test "
						<< i << "\n[construction commands:\n"
						<< constructLog
						<< "do commands:\n"	<< doLog << "]";
				QFAIL(ss.str().c_str());
			}
			while (e.redo()) {
			}
			if (helper.hashModel(e) != finalState) {
				std::stringstream ss;
				ss << "Failed to redo to final state on test "
						<< i << "\n[construction commands:\n"
						<< constructLog
						<< "do commands:\n"	<< doLog << "]";
				QFAIL(ss.str().c_str());
			}
			totalMallocCount = doMallocCount + undoMallocCount;
			// allow the logger to write out any remaining buffer upon its
			// destruction
			freopen(0, "w", logFile);
		}

		if (oomLoaded && 0 < totalMallocCount) {
			RandomSource rng(initial);
			// Test undoing after an Out-Of-Memory failure
			int32_t failAt = generalRng.getUniform(totalMallocCount);
			e.setCommandLogger(0);
			helper.resetModel(e);
			e.executeRandomConstructiveCommands(rng);
			e.clearHistory();
			setMallocsUntilFailure(failAt);
			bool failed = false;
			try {
				e.executeRandomCommands(rng, minCommands, maxCommands);
				while (e.undo()) {
				}
			} catch (std::bad_alloc&) {
				failed = true;
			}
			cancelAnyMallocFailure();
			while (e.undo()) {
			}
			if (helper.hashModel(e) != initialState) {
				std::stringstream ss;
				ss << "Failed to undo to initial state ";
				if (failed) {
					ss << "(after malloc failure) " << failAt;
				} else {
					ss << "(even though there was no malloc failure)";
				}
				ss << " on test " << i
						<< "\n[construction commands:\n"
						<< constructLog
						<< "do commands:\n"	<< doLog << "]";
				QFAIL(ss.str().c_str());
			}
		}

		if (oomLoaded && 0 < totalMallocCount) {
			RandomSource rng(initial);
			// Test redoing after an Out-Of-Memory failure
			int32_t failAt2 = generalRng.getUniform(totalMallocCount);
			e.setCommandLogger(0);
			helper.resetModel(e);
			e.executeRandomConstructiveCommands(rng);
			e.clearHistory();
			e.executeRandomCommands(rng, minCommands, maxCommands);
			setMallocsUntilFailure(failAt2);
			bool failed = false;
			try {
				while (e.undo()) {
				}
				while (e.redo()) {
				}
			} catch (std::bad_alloc&) {
				failed = true;
			}
			cancelAnyMallocFailure();
			while (e.redo()) {
			}
			if (helper.hashModel(e) != finalState) {
				std::stringstream ss;
				ss << "Failed to redo to final state ";
				if (failed) {
					ss << "(after malloc failure) " << failAt2;
				} else {
					ss << "(even though there was no malloc failure)";
				}
				ss << " on test " << i
						<< "\n[construction commands:\n"
						<< constructLog
						<< "do commands:\n"	<< doLog << "]";
				QFAIL(ss.str().c_str());
			}
		}

		if (oomLoaded && 0 < doMallocCount) {
			// test that commands that fail do not get replayed from the log
			RandomSource rng(initial);
			cancelAnyMallocFailure();
			e.setCommandLogger(0);
			helper.resetModel(e);
			e.executeRandomConstructiveCommands(rng);
			e.clearHistory();
			FileCommandLogger fileLogger;
			FILE* logFile = tmpfile();
			// ownership of logFile is passed here
			fileLogger.setLogFile(logFile);
			stringLogger.SetDelegate(fileLogger.getLogger());
			e.setCommandLogger(&stringLogger);
			logString.clear();
			int32_t failAt = generalRng.getUniform(doMallocCount);
			setMallocsUntilFailure(failAt);
			try {
				e.executeRandomCommands(rng, minCommands, maxCommands);
			} catch (std::bad_alloc&) {
				cancelAnyMallocFailure();
				std::string beforeFailureLog(logString);
				logString.clear();
				e.executeRandomCommands(rng, minCommands, maxCommands);
				std::string afterFailureLog(logString);
				Hash afterFailureState(helper.hashModel(e));
				RandomSource rng2(initial);
				e.setCommandLogger(0);
				helper.resetModel(e);
				e.executeRandomConstructiveCommands(rng2);
				logFile = freopen(0, "r", logFile);
				bool failed = false;
				try {
					while (executeLineFromFile(e, logFile)) {
					}
				} catch (ParametersOutOfRangeException&) {
					failed = true;
				}
				if (failed || helper.hashModel(e) != afterFailureState) {
					std::stringstream ss;
					ss << "Failed to replay only successful commands from the log\n"
							<< "after failure at " << failAt << "\non test"
							<< i << "\n[construction commands:\n"
							<< constructLog
							<< "do commands:\n"	<< beforeFailureLog
							<< "after failure:\n" << afterFailureLog << "]";
					QFAIL(ss.str().c_str());
				}
			}
			cancelAnyMallocFailure();
			stringLogger.SetDelegate(0);
			// allow the logger to write out any remaining buffer upon its
			// destruction
			freopen(0, "w", logFile);
		}
	}
}
