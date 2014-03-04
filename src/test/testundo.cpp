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

#include "oomtestutil.h"

#include "src/domain/undo/executor.h"
#include "src/domain/undo/commandlogger.h"
#include "src/domain/undo/filelogger.h"
#include "src/domain/undo/random.h"

#include <sstream>
#include <stdio.h>

#include <QtTest/QtTest>

class StringLoggerWrapper: public CommandLogger {
	CommandLogger* delegate;
	std::string* out;
public:
	StringLoggerWrapper(std::string* output) :
			delegate(0), out(output) {
	}
	/**
	 * Create a logger that writes to a std::string and also passes writes
	 * along to @a wrapped.
	 * @param wrapped Ownership is not passed.
	 * @param output The string to be logged to. Ownership is not passed.
	 */
	StringLoggerWrapper(std::string* output, CommandLogger* wrapped) :
			delegate(wrapped), out(output) {
	}
	StringLoggerWrapper(const StringLoggerWrapper&); // unimplemented
	StringLoggerWrapper& operator=(const StringLoggerWrapper&); // unimplemented
	~StringLoggerWrapper() {
	}
	void setOutputString(std::string* output) {
		out = output;
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
	void setDelegate(CommandLogger* newLogger) {
		delegate = newLogger;
	}
};

ModelTestHelper::~ModelTestHelper() {
}

bool executeLineFromFile(Executor& e, FILE* logFile) {
	enum {
		lineBufferSize = 1024
	};
	static char lineBuffer[lineBufferSize];
	if (!fgets(lineBuffer, lineBufferSize, logFile))
		return false;
	e.executeFromLog(lineBuffer);
	return true;
}

// runs a part of the test measuring the mallocs and logging the activity
class ExecutorStep {
	static int failures;
	long mallocCount;
	ExecutorStep* previous;
	std::string log[2];
	StringLoggerWrapper stringLogger;
	RandomSource final;
	void setup(Executor& e, CommandLogger* logger, int whichLog) {
		cancelAnyMallocFailure();
		log[whichLog].clear();
		stringLogger.setOutputString(&log[whichLog]);
		stringLogger.setDelegate(logger);
		e.setCommandLogger(&stringLogger);
	}
	/**
	 * Runs the step. Sets malloc count and log.
	 */
	void run(Executor& e, RandomSource& rng, int whichLog) {
		if (previous)
			previous->run(e, rng, whichLog);
		setup(e, logger(), whichLog);
		long start = mallocsSoFar();
		doStep(e, rng);
		long end = mallocsSoFar();
		mallocCount = end - start;
		final = rng;
	}
public:
	ExecutorStep* getPrevious() const {
		return previous;
	}
	RandomSource finalRng() const {
		return final;
	}
	static int failureCount() {
		return failures;
	}
	ExecutorStep(ExecutorStep* following)
		: mallocCount(0), previous(following), stringLogger(&log[0]) {
	}
	virtual ~ExecutorStep() {
	}
	virtual const char* name() const = 0;
	virtual void doStep(Executor& e, RandomSource& rng) = 0;
	virtual CommandLogger* logger() const {
		return 0;
	}
	void getLog(std::string& out, int which) const {
		if (previous)
			previous->getLog(out, which);
		out.append(";\n");
		out.append(name());
		out.append(": ");
		out.append(log[which]);
	}
	long getMallocCount() const {
		return mallocCount;
	}
	/**
	 * Runs this series of steps and {@a other} and tests the results against
	 * one another. {@a other} is run first.
	 */
	void runAndCheck(const char* name, ExecutorStep& other, Executor& executor,
			ModelTestHelper& helper, RandomSource rng, int testNum) {
		RandomSource r2 = rng;
		other.run(executor, rng, 0);
		Hash h = helper.hashModel(executor);
		run(executor, r2, 1);
		Hash h2 = helper.hashModel(executor);
		if (h != h2) {
			++failures;
			std::string logS1;
			other.getLog(logS1, 0);
			std::string logS2;
			getLog(logS2, 1);
			std::ostringstream ss;
			ss << "Failed test '" << name << "' on iteration " << testNum
					<< "\nTesting:" << logS1 << "\nAgainst:" << logS2;
			std::string s = ss.str();
			QFAIL(s.c_str());
		}
	}
};

int ExecutorStep::failures = 0;

/**
 * Can only be run successfully if the delegate has already been run.
 */
class FailingStep : public ExecutorStep {
	ExecutorStep* del;
	std::string nameString;
	bool fail;
public:
	FailingStep(ExecutorStep* delegate)
		: ExecutorStep(delegate? delegate->getPrevious() : 0),
		  del(delegate), nameString("failing "), fail(false) {
		nameString.append(del->name());
		nameString.c_str();
	}
	const char* name() const {
		return nameString.c_str();
	}
	CommandLogger* logger() const {
		return del->logger();
	}
	bool failed() const {
		return fail;
	}
	void doStep(Executor& e, RandomSource& rng) {
		fail = false;
		long mallocCount = del->getMallocCount();
		if (mallocCount < 1) {
			del->doStep(e, rng);
			return;
		}
		int muf = rng.getUniform(mallocCount - 1);
		setMallocsUntilFailure(muf);
		try {
			del->doStep(e, rng);
		} catch(...) {
			fail = true;
		}
		cancelAnyMallocFailure();
	}
};

class ExecutorInit : public ExecutorStep {
	ModelTestHelper& mth;
public:
	ExecutorInit(ModelTestHelper& helper) : ExecutorStep(0), mth(helper) {
	}
	const char* name() const {
		return "initialize";
	}
	void doStep(Executor& e, RandomSource&) {
		mth.resetModel(e);
	}
};

class ExecutorConstruct : public ExecutorStep {
public:
	ExecutorConstruct(ExecutorStep* following) : ExecutorStep(following) {
	}
	const char* name() const {
		return "constructive commands";
	}
	void doStep(Executor& e, RandomSource& rng) {
		e.executeRandomConstructiveCommands(rng);
	}
};

class ClearHistory : public ExecutorStep {
public:
	ClearHistory(ExecutorStep* following)
		: ExecutorStep(following) {
	}
	const char* name() const {
		return "clear history";
	}
	void doStep(Executor& e, RandomSource&) {
		e.clearHistory();
	}
};

class ExecutorDo : public ExecutorStep {
	int min;
	int max;
	FileCommandLogger* fLogger;
public:
	ExecutorDo(ExecutorStep* following, FileCommandLogger* fileLogger)
		: ExecutorStep(following), min(1), max(40), fLogger(fileLogger) {
	}
	void setMinimumAndMaximumCommands(int minimum, int maximum) {
		min = minimum;
		max = maximum;
	}
	const char* name() const {
		return "random commands";
	}
	void doStep(Executor& e, RandomSource& rng) {
		if (fLogger)
			freopen(0, "w", fLogger->getLogFile());
		int cc;
		e.executeRandomCommands(cc, rng, min, max);
	}
	CommandLogger* logger() const {
		return fLogger->getLogger();
	}
};

class ExecutorDoesThenRandomUndoesAndRedoes : public ExecutorStep {
	FileCommandLogger* fLogger;
public:
	ExecutorDoesThenRandomUndoesAndRedoes(ExecutorStep* following,
			FileCommandLogger* fileLogger)
		: ExecutorStep(following), fLogger(fileLogger) {
	}
	const char* name() const {
		return "random undoes and redoes";
	}
	void doStep(Executor& e, RandomSource& rng) {
		if (fLogger)
			freopen(0, "w", fLogger->getLogFile());
		int commandCount = 0;
		e.executeRandomCommands(commandCount, rng, 1, 20);
		int maxUndoes = rng.getUniform(commandCount);
		for (int j = 0; j != maxUndoes; ++j) {
			e.undo();
		}
		int redoCount = rng.getUniform(maxUndoes);
		for (int i = 0; i != redoCount; ++i) {
			e.redo();
		}
	}
	CommandLogger* logger() const {
		return fLogger->getLogger();
	}
};

class ExecutorUndoAll : public ExecutorStep {
public:
	ExecutorUndoAll(ExecutorStep* following) : ExecutorStep(following) {
	}
	const char* name() const {
		return "undo all";
	}
	void doStep(Executor& e, RandomSource&) {
		while (e.canUndo()) {
			e.undo();
		}
	}
};

class ExecutorRedoAll : public ExecutorStep {
public:
	ExecutorRedoAll(ExecutorStep* following) : ExecutorStep(following) {
	}
	const char* name() const {
		return "redo all";
	}
	void doStep(Executor& e, RandomSource&) {
		while (e.canRedo()) {
			e.redo();
		}
	}
};

class ExecutorReplay : public ExecutorStep {
	FILE* file;
public:
	ExecutorReplay(ExecutorStep* following, FILE* fh)
		: ExecutorStep(following), file(fh) {
	}
	const char* name() const {
		return "replay";
	}
	void doStep(Executor& e, RandomSource&) {
		freopen(0, "r", file);
		try {
			while (executeLineFromFile(e, file)) {
			}
		} catch (...) {
		}
	}
};

// Use this if you want FailingStep to refer to a pair of steps.
class TwoSteps : public ExecutorStep {
	ExecutorStep& s1;
	ExecutorStep& s2;
	std::string nameStr;
public:
	// The steps that first and second are following are ignored here.
	TwoSteps(ExecutorStep& first, ExecutorStep& second, ExecutorStep* following)
		: ExecutorStep(following), s1(first), s2(second), nameStr(s1.name()) {
		nameStr.append(" then ");
		nameStr.append(s2.name());
		nameStr.c_str();
	}
	const char* name() const {
		return nameStr.c_str();
	}
	void doStep(Executor& e, RandomSource& rng) {
		s1.doStep(e, rng);
		s2.doStep(e, rng);
	}
};

// Do then replay
// Do then replay then undo
// Do then replay then undo then redo
// Do then replay then OOM[undo then redo] then undo
// Do then replay then OOM[undo then redo] then redo
// OOM[do] then replay
// Also need: Do then undo some then redo some (checkpoint) then replay
void testUndo(Executor& e, ModelTestHelper& helper) {
	FileCommandLogger fileLogger;
	FILE* logFile = tmpfile();
	// ownership of logFile is passed here
	fileLogger.setLogFile(logFile);

	// the tree of possible execution paths that we are going to check:
	// (1) Do = replay
	ExecutorInit init(helper);
	ExecutorConstruct construct(&init);
	ClearHistory clearHistory(&construct);
	ExecutorDo doStuff(&clearHistory, &fileLogger);
	ExecutorReplay replay(&clearHistory, logFile);

	// (2) Do, undo = replay, undo
	ExecutorUndoAll undoToConstruct(&doStuff);
	ExecutorUndoAll undoAfterReplay(&replay);

	// (3) Do = replay, undo, redo
	ExecutorRedoAll redoAgain(&undoToConstruct);

	// (4) Construct = Do, replay, OOM[undo then redo], undo
	TwoSteps undoThenRedo(undoToConstruct, redoAgain, &undoToConstruct);
	FailingStep failToUndoThenRedo(&undoThenRedo);
	ExecutorUndoAll undoAfterFail(&failToUndoThenRedo);

	// (5) Do = replay, OOM[undo then redo], redo
	ExecutorRedoAll redoAfterFail(&failToUndoThenRedo);

	// (6) OOM[Do] = replay
	FailingStep failToDo(&doStuff);

	// (7) Do, undo some, redo some, do = replay
	// (8) OOM[Do, undo some, redo some, do] = replay
	//TODO

	const int commandCount = e.commandCount();
	const int testCount = 4 * commandCount * commandCount;
	const int firstPhaseEnds = testCount / 2;
	const int secondPhaseEnds = (testCount * 3) / 4;
	bool oomLoaded = loadOomTestUtil();
	QVERIFY2(oomLoaded, "Oom Test Util not loaded!");
	std::string logString;
	RandomSource rng;
	StringLoggerWrapper stringLogger(&logString);
	// Now we will check pairs of steps in the tree against each other to check
	// that they produce identical results.
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
		doStuff.setMinimumAndMaximumCommands(minCommands, maxCommands);

		// (1)
		replay.runAndCheck("replay", doStuff, e, helper, rng, i);
		// (2)
		undoAfterReplay.runAndCheck("undo after replay",
				undoToConstruct, e, helper, rng, i);
		// (3)
		redoAgain.runAndCheck("redo after replay and undo",
				doStuff, e, helper, rng, i);
		// (4)
		undoAfterFail.runAndCheck("undo after fail",
				construct, e, helper, rng, i);
		// (5)
		redoAfterFail.runAndCheck("redo after fail",
				doStuff, e, helper, rng, i);
		// (6)
		replay.runAndCheck("replays failing sequence correctly",
				failToDo, e, helper, rng, i);
		// (7)
		//TODO
		rng = redoAgain.finalRng();
	}
	cancelAnyMallocFailure();
	// allow the logger to write out any remaining buffer upon its
	// destruction
	freopen(0, "w", logFile);
}
