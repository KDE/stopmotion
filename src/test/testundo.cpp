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

#include "testundo.h"

#include "hash.h"
#include "oomtestutil.h"
#include "src/domain/undo/executor.h"
#include "src/domain/undo/commandlogger.h"
#include "src/domain/undo/filelogger.h"
#include "src/domain/undo/random.h"
#include "src/domain/animation/errorhandler.h"
#include "src/foundation/stringwriter.h"

#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <cerrno>
#include <exception>
#include <assert.h>
#include <unistd.h>

#include <QTest>



class StringLoggerWrapper: public CommandLogger {
	CommandLogger* delegate;
	std::string* out;
	std::string pending;
	int committedUpTo;
public:
	StringLoggerWrapper(std::string* output) :
			delegate(0), out(output), committedUpTo(0) {
	}
	/**
	 * Create a logger that writes to a std::string and also passes writes
	 * along to @a wrapped.
	 * @param wrapped Ownership is not passed.
	 * @param output The string to be logged to. Ownership is not passed.
	 */
	StringLoggerWrapper(std::string* output, CommandLogger* wrapped) :
			delegate(wrapped), out(output), committedUpTo(0) {
	}
	StringLoggerWrapper(const StringLoggerWrapper&); // unimplemented
	StringLoggerWrapper& operator=(const StringLoggerWrapper&); // unimplemented
	~StringLoggerWrapper() {
	}
	void setOutputString(std::string* output) {
		out = output;
	}
	void output(std::string* to) {
		if (to)
			to->append(pending, 0, committedUpTo);
		pending.erase(0, committedUpTo);
		committedUpTo = 0;
	}
	void writePendingCommand(const char* command) {
		pending.resize(committedUpTo);
		pending.append(command);
		pending.append("!\n");
		if (delegate)
			delegate->writePendingCommand(command);
	}
	void commit() {
		committedUpTo = pending.length();
		if (delegate)
			delegate->commit();
		output(out);
	}
	void writePendingUndo() {
		pending.resize(committedUpTo);
		pending.append("--undo---\n");
		if (delegate)
			delegate->writePendingUndo();
	}
	void writePendingRedo() {
		pending.resize(committedUpTo);
		pending.append("--redo---\n");
		if (delegate)
			delegate->writePendingRedo();
	}
	void setDelegate(CommandLogger* newLogger) {
		delegate = newLogger;
	}
	void flush() {
		output(out);
		if (delegate)
			delegate->flush();
	}
};

ModelTestHelper::~ModelTestHelper() {
}

// In order to test the executor and its ability to survive exceptions being
// thrown, we chain together a load of executor steps, and compare the
// result of running both. For example, we might run a load of commands
// with a randomly-failing mallocker and compare the result against
// running the commands from the log thus produced.
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
	int activeLog;
	void finishRun(long mallocsAtStart, RandomSource& rng) {
		long end = mallocsSoFar();
		mallocCount = end - mallocsAtStart;
		final = rng;
	}
	/**
	 * Runs the step. Sets malloc count and log.
	 */
	void run(Executor& e, RandomSource& rng, int whichLog) {
		activeLog = whichLog;
		if (previous)
			previous->run(e, rng, whichLog);
		setup(e, logger(), whichLog);
		long start = mallocsSoFar();
		try {
			doStep(e, rng);
		} catch(...) {
			finishRun(start, rng);
			throw;
		}
		finishRun(start, rng);
	}
public:
	int getCurrentlyActiveLog() const {
		return activeLog;
	}
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
		: mallocCount(0), previous(following), stringLogger(&log[0]),
		  activeLog(0) {
	}
	virtual ~ExecutorStep() {
	}
	virtual const char* name() const = 0;
	virtual void doStep(Executor& e, RandomSource& rng) = 0;
	virtual CommandLogger* logger() {
		return 0;
	}
	virtual void cleanup() {
	}
	virtual void appendCommandLog(std::string& out, int which) {
		stringLogger.output(&log[which]);
		out.append(log[which]);
	}
	void getLog(std::string& out, int which) {
		if (previous)
			previous->getLog(out, which);
		out.append(";\n");
		out.append(name());
		out.append(": ");
		appendCommandLog(out, which);
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
		try {
			other.run(executor, rng, 0);
		} catch (std::exception& e) {
			other.cleanup();
			std::string log;
			other.getLog(log, 0);
			std::ostringstream ss;
			ss << "Failed to run 'other' step in test '" << name
					<< "' on iteration " << testNum
					<< "\nSuccessful log:" << log;
			std::string s = ss.str();
			QFAIL(s.c_str());
		}
		other.cleanup();
		Hash h = helper.hashModel(executor);
		std::string model;
		helper.dumpModel(model, executor);
		try {
			run(executor, r2, 1);
		} catch (std::exception& e) {
			cleanup();
			std::string logS1;
			other.getLog(logS1, 0);
			std::string logS2;
			getLog(logS2, 1);
			std::ostringstream ss;
			ss << "Failed to run 'this' step in test '" << name
					<< "' on iteration " << testNum
					<< "\nOther log:" << logS1
					<< "\nSuccessful portion of 'this' log:" << logS2;
			std::string s = ss.str();
			QFAIL(s.c_str());
		}
		cleanup();
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
			std::string model2;
			helper.dumpModel(model2, executor);
			ss << "Resulting in:\n" << model << "And:\n" << model2;
			std::string s = ss.str();
			QFAIL(s.c_str());
		}
	}
	/**
	 * Runs this series of steps and {@a other} and tests the results against
	 * one another. {@a other} is run first.
	 * Just like runAndCheck but pre-runs this.run. Useful if "this" contains a
	 * FailingStep (whose delegate needs to be pre-run).
	 */
	void runAndCheckWithPreRun(const char* name, ExecutorStep& other, Executor& executor,
			ModelTestHelper& helper, RandomSource rng, int testNum) {
		RandomSource r1 = rng;
		try {
			run(executor, r1, 1);
		} catch (std::exception& e) {
			cleanup();
			std::string logS1;
			other.getLog(logS1, 0);
			std::string logS2;
			getLog(logS2, 0);
			std::ostringstream ss;
			ss << "Failed to pre-run 'this' step in test '" << name
					<< "' on iteration " << testNum
					<< "\nOther log:" << logS1
					<< "\nSuccessful portion of 'this' log:" << logS2;
			std::string s = ss.str();
			QFAIL(s.c_str());
		}
		cleanup();
		runAndCheck(name, other, executor, helper, rng, testNum);
	}
};

int ExecutorStep::failures = 0;

FILE* fileOpen(const char* path, const char* mode) {
	FILE* fh = fopen(path, mode);
	if (!fh) {
		sleep(1);
		fh = fopen(path, mode);
	}
	if (!fh) {
		int err = errno;
		// for some reason errno can be 0 when fopen returns 0
		if (err == 0 || err == ENOMEM) {
			throw std::bad_alloc();
		}
		StringWriter sw;
		sw.writeIdentifier("fopen failed for file");
		sw.writeString(path);
		sw.writeIdentifier("with error code");
		sw.writeInteger(err);
		sw.writeChar('(');
		sw.writeIdentifier(strerror(err));
		sw.writeChar(')');
		qWarning("%s", sw.result());
	}
	return fh;
}

/**
 * Runs the delegate, failing one of its mallocs.
 * It works best as part of the "this" in a call to runAndCheck with
 * the delegate a non-failing part of the "other" argument.
 */
class FailingStep : public ExecutorStep {
	ExecutorStep* del;
	std::string nameString;
	bool fail;
	int totalFails;
	int noMallocsToFailCount;
public:
	FailingStep(ExecutorStep* delegate)
		: ExecutorStep(delegate? delegate->getPrevious() : 0),
		  del(delegate), nameString("failing "), fail(false),
		  totalFails(0), noMallocsToFailCount(0) {
		nameString.append(del->name());
		nameString.c_str();
	}
	const char* name() const {
		return nameString.c_str();
	}
	CommandLogger* logger() {
		return del->logger();
	}
	bool failed() const {
		return fail;
	}
	int failedCount() const {
		return totalFails;
	}
	int noMallocsCount() const {
		return noMallocsToFailCount;
	}
	void doStep(Executor& e, RandomSource& rng) {
		fail = false;
		long mallocCount = del->getMallocCount();
		if (mallocCount < 1) {
			del->doStep(e, rng);
			++noMallocsToFailCount;
			return;
		}
		int muf = rng.getUniform(mallocCount - 1);
		setMallocsUntilFailure(muf);
		try {
			del->doStep(e, rng);
		} catch(...) {
			fail = true;
			++totalFails;
		}
		cancelAnyMallocFailure();
	}
	void cleanup() {
		del->cleanup();
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
	FileCommandLogger fLogger;
	const char* logFName;
	FILE* fh;
public:
	ExecutorDo(ExecutorStep* following, const char* logFileName)
		: ExecutorStep(following), min(1), max(40), logFName(logFileName),
		  fh(0) {
	}
	void setMinimumAndMaximumCommands(int minimum, int maximum) {
		min = minimum;
		max = maximum;
	}
	const char* name() const {
		return "random commands";
	}
	void doStep(Executor& e, RandomSource& rng) {
		fh = fileOpen(logFName, "w");
		assert(fh);
		fLogger.setLogFile(fh);
		int cc;
		e.executeRandomCommands(cc, rng, min, max);
	}
	void cleanup() {
		fLogger.getLogger()->flush();
		fLogger.setLogFile(0);
		fh = 0;
	}
	CommandLogger* logger() {
		return fLogger.getLogger();
	}
};

class ExecutorDoesAndRandomUndoesAndRedoes : public ExecutorStep {
	FileCommandLogger fLogger;
	const char* logFName;
	FILE* fh;
public:
	ExecutorDoesAndRandomUndoesAndRedoes(ExecutorStep* following,
			const char* logFileName)
		: ExecutorStep(following), logFName(logFileName), fh(0) {
	}
	const char* name() const {
		return "random undoes and redoes";
	}
	void doStep(Executor& e, RandomSource& rng) {
		fh = fileOpen(logFName, "w");
		assert(fh);
		fLogger.setLogFile(fh);
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
		e.executeRandomCommands(commandCount, rng, 1, 3);
	}
	void cleanup() {
		fLogger.setLogFile(0);
		fh = 0;
	}
	CommandLogger* logger() {
		return fLogger.getLogger();
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
	const char* logFName;
	FILE* fh;
	enum {
		lineBufferSize = 1024
	};
	char lineBuffer[lineBufferSize];
	std::string replayed[2];
public:
	ExecutorReplay(ExecutorStep* following, const char* logFileName)
		: ExecutorStep(following),  logFName(logFileName), fh(0) {
	}
	~ExecutorReplay() {
		cleanup();
	}
	const char* name() const {
		return "replay";
	}
	void appendCommandLog(std::string& out, int which) {
		out.append(replayed[which]);
	}
	void doStep(Executor& e, RandomSource&) {
		cleanup();
		int whichLog = getCurrentlyActiveLog();
		replayed[whichLog].clear();
		fh = fileOpen(logFName, "r");
		assert(fh);
		while (fgets(lineBuffer, lineBufferSize, fh)) {
			e.executeFromLog(lineBuffer, *ErrorHandler::getThrower());
			replayed[whichLog].append(lineBuffer);
		}
	}
	void cleanup() {
		if (fh) {
			fclose(fh);
			fh = 0;
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
	static const char tmpDirTemplate[] = "/tmp/lsmXXXXXX";
	char tmpDirName[sizeof(tmpDirTemplate)];
	strncpy(tmpDirName, tmpDirTemplate, sizeof(tmpDirName));
	mkdtemp(tmpDirName);
	std::string tmpFileName(tmpDirName);
	tmpFileName += "/command.log";

	// the tree of possible execution paths that we are going to check:
	// (1) Do = replay
	ExecutorInit init(helper);
	ExecutorConstruct construct(&init);
	ClearHistory clearHistory(&construct);
	ExecutorDo doStuff(&clearHistory, tmpFileName.c_str());
	ExecutorReplay replay(&clearHistory, tmpFileName.c_str());

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
	ExecutorDoesAndRandomUndoesAndRedoes doesUndoesRedoes(&construct,
			tmpFileName.c_str());

	// (8) OOM[Do, undo some, redo sometestundo, do] = replay
	FailingStep failingDur(&doesUndoesRedoes);

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
		replay.runAndCheck("replays sequence of does, undoes and redoes correctly",
				doesUndoesRedoes, e, helper, rng, i);
		// (8)
		replay.runAndCheckWithPreRun(
				"replays failing sequence of does, undoes and redoes correctly",
				failingDur, e, helper, rng, i);

		rng = redoAgain.finalRng();
	}
	cancelAnyMallocFailure();

	QVERIFY2(0 == unlink(tmpFileName.c_str()), "Could not delete test command log file");
	QVERIFY2(0 == rmdir(tmpDirName), "Could not delete test directory");
	// The tests that rely on failing commands will pass if the fake malloc
	// somehow does not cause any command to fail (this will happen, for
	// example, if no allocations are actually made). If this happens too much
	// then these tests are not being useful, so we check here that at least
	// half of the tests run in each case did require recovering from a failure.
	QVERIFY2(testCount / 2 < failToUndoThenRedo.failedCount()
			|| testCount < failToUndoThenRedo.noMallocsCount(),
			"failToUndoThenRedo didn't fail very often");
	QVERIFY2(testCount / 2 < failToDo.failedCount(),
			"failToDo didn't fail very often");
}
