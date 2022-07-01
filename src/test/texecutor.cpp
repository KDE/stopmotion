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
#include "texecutor.h"

#include <QtTest/QtTest>

#include <string.h>
#include <memory>
#include <limits>
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <sstream>
#include <exception>
#include <assert.h>
#include <utility>

#include "src/domain/undo/executor.h"
#include "src/domain/undo/command.h"
#include "src/domain/undo/commandlogger.h"
#include "src/domain/undo/filelogger.h"
#include "src/domain/animation/errorhandler.h"

#include "hash.h"
#include "testundo.h"
#include "oomtestutil.h"



static const int32_t no_num = std::numeric_limits<int32_t>::min();

class TestException : public std::exception {
	const char* msg;
public:
	TestException(const char* message) : msg(message) {
	}
	const char* what() const throw() {
		return msg;
	}
};

/**
 * Test factory for commands that test parsing. When executed, they write a
 * string of their contents to a list of strings.
 */
class EmptyTestCommandFactory : public CommandFactory {
	std::string name;
	typedef TestCommandFactory::output_t output_t;
	output_t& output;
public:
	EmptyTestCommandFactory(const char* nameForCommand,
			output_t& executionOutput)
			: name(nameForCommand), output(executionOutput) {
	}
	class EtCommand : public Command {
	public:
		std::string name;
		output_t& output;
		std::string s1;
		int32_t i1;
		int32_t i2;
		EtCommand(std::string commandName, output_t& out)
				: name(commandName), output(out),
				  s1(""), i1(no_num), i2(no_num) {
		}
		Command* execute() {
			std::stringstream ss;
			ss << name << ",i:" << i1 << ",s:" << s1 << ",i:" << i2;
			output.push_back(ss.str());
			return createNullCommand();
		}
		bool operator==(const EtCommand& other) const {
			return name == other.name
					&& s1 == other.s1
					&& i1 == other.i1
					&& i2 == other.i2;
		}
		bool operator!=(const EtCommand& other) const {
			return !(*this == other);
		}
	};
	~EmptyTestCommandFactory() {
	}
	Command* create(Parameters& ps, ErrorHandler&) {
		EtCommand* e = new EtCommand(name, output);
		e->i1 = ps.getInteger(-RAND_MAX/2, RAND_MAX/2);
		ps.getString(e->s1, 0);
		e->i2 = ps.getInteger(-RAND_MAX/2, RAND_MAX/2);
		return e;
	}
	void Fail(const char* s) {
		QFAIL(s);
	}
};

/**
 * Re-executes the command that is logged.
 * This is used to test that executing from the log is the same as executing
 * normally, without the bother of actually using a log; the logged commands
 * are executed as we go along. We can then test the results of executing from
 * the log against the results of executing the commands normally.
 */
class CloneLogger : public CommandLogger {
	Executor* ex;
	std::string command;
	bool alreadyIn;
	class AlreadyIn {
		bool& r;
	public:
		AlreadyIn(bool& a) : r(a) {
			r = true;
		}
		~AlreadyIn() {
			r = false;
		}
	};
public:
	CloneLogger() : ex(0), alreadyIn(false) {
	}
	~CloneLogger() {
	}
	/** @param e Ownership is not passed. */
	void SetExecutor(Executor* e) {
		ex = e;
	}
	void writePendingCommand(const char* lineToLog) {
		command = lineToLog;
	}
	void commit() {
		if (!alreadyIn) {
			// Make sure we don't recursively call ourselves
			AlreadyIn a(alreadyIn);
			// Some tests are not logging; don't try to execute "!"
			if (!command.empty()) {
				command.append(1, '!');
				ex->executeFromLog(command.c_str(), *ErrorHandler::getThrower());
			}
		}
	}
	void writePendingUndo() {
		assert(false);
	}
	void writePendingRedo() {
		assert(false);
	}
	void flush() {
		// For the tests we will assume that commit() never fails, so we do not
		// need an implementation here.
	}
};

TestCommandFactory::TestCommandFactory()
		: ce(0), cl(0), str(0), strNext(0), strAllocLen(0) {
	cl = new CloneLogger();
	ce = makeExecutor();
	ce->setCommandLogger(cl);
	cl->SetExecutor(ce);
	std::unique_ptr<CommandFactory> et(
			new EmptyTestCommandFactory("et", executionOutput));
	std::unique_ptr<CommandFactory> sec(
			new EmptyTestCommandFactory("sec", executionOutput));
	ce->addCommand("et", std::move(et));
	ce->addCommand("sec", std::move(sec));
}

TestCommandFactory::~TestCommandFactory() {
	delete ce;
	delete[] str;
}

void TestCommandFactory::AddCharToRandomString(char c) {
	if (strNext == str + strAllocLen) {
		int32_t newLen = strAllocLen == 0? 64 : strAllocLen * 2;
		char* newStr = new char[newLen];
		strncpy(newStr, str, strAllocLen);
		delete[] str;
		strNext = newStr + (strNext - str);
		str = newStr;
		strAllocLen = newLen;
	}
	*strNext = c;
	++strNext;
}

const char* TestCommandFactory::RandomString() {
	strNext = str;
	int type = rand() % 4;
	while (type != 0) {
		switch (type) {
		case 1:
			AddCharToRandomString('0' + (rand() % 10));
			break;
		case 2:
			AddCharToRandomString('a' + (rand() % 26));
			break;
		default:
			AddCharToRandomString('A' + (rand() % 26));
			break;
		}
		type = rand() % 4;
	}
	AddCharToRandomString('\0');
	return str;
}

void TestCommandFactory::emptyCommandReplayerThrows() {
	try {
		ce->execute("fakeCommand");
	} catch (UnknownCommandException& e) {
		return;
	}
	QFAIL("Empty CommandReplayer did not throw "
			"CommandFactoryNoSuchCommandException");
}

void TestCommandFactory::canParseFromLog() {
	executionOutput.clear();
	ce->executeFromLog("et -5 \"hello world!\" 412345!", *ErrorHandler::getThrower());
	QCOMPARE(executionOutput.begin()->c_str(),
			"et,i:-5,s:hello world!,i:412345");
}

int32_t randomInt() {
	return rand() - RAND_MAX/2;
}

void TestCommandFactory::parsingDescriptionIsCloning() {
	char description[512];
	const size_t desLen = sizeof(description)/sizeof(description[0]) - 1;
	description[desLen] = '\0';
	executionOutput.clear();
	for (int i = 0; i != 100; ++i) {
		const char* commandName = rand() % 2 == 0? "et" : "sec";
		int32_t i1 = randomInt();
		int32_t i2 = randomInt();
		std::string s1 = RandomString();
		ce->execute(commandName, i1, s1.c_str(), i2);
		output_t::iterator eo1 = executionOutput.begin();
		output_t::iterator eo2 = eo1;
		++eo2;
		QCOMPARE(eo1->c_str(), eo2->c_str());
	}
}

namespace {
const char alphanumeric[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
}

class AddCharFactory : public CommandFactory {
	std::string* model;
public:
	class AddChar : public Command {
		std::string* m;
		char c;
		int32_t p;
	public:
		AddChar(std::string& model, int32_t character, int32_t position)
			: m(&model), c(character), p(position) {
		}
		Command* execute();
	};
	AddCharFactory(std::string* m) : model(m) {
	}
	Command* create(Parameters& ps, ErrorHandler&) {
		int32_t i = ps.getInteger(0, sizeof(alphanumeric) - 1);
		int32_t character = alphanumeric[i];
		int32_t position = ps.getInteger(0, model->length());
		return new AddChar(*model, character, position);
	}
};

class DelCharFactory : public CommandFactory {
	std::string* model;
public:
	class DelChar : public Command {
		std::string* m;
		int32_t p;
	public:
		DelChar(std::string& model, int32_t position)
			: m(&model), p(position) {
		}
		Command* execute();
	};
	DelCharFactory(std::string* m) : model(m) {
	}
	Command* create(Parameters& ps, ErrorHandler&) {
		int32_t len = model->length();
		if (len == 0)
			return 0;
		int32_t position = ps.getInteger(0, len - 1);
		return new DelChar(*model, position);
	}
};

Command* AddCharFactory::AddChar::execute() {
	if (p < 0 || static_cast<int32_t>(m->length()) < p)
		throw TestException("AddCharFactory parameters out-of-range");
	// insert might throw, so use an unique_ptr to avoid leaks.
	std::unique_ptr<Command> inv(new DelCharFactory::DelChar(*m, p));
	std::string::iterator i = m->begin();
	i += p;
	m->insert(i, c);
	delete this;
	return inv.release();
}

Command* DelCharFactory::DelChar::execute() {
	if (m->size() == 0) {
		return createNullCommand();
	}
	char removedChar = (*m)[p];
	Command* inv = new AddCharFactory::AddChar(*m, removedChar, p);
	m->erase(p, 1);
	delete this;
	return inv;
}

class StringModelTestHelper : public ModelTestHelper {
	// not owned
	std::string* s;
public:
	StringModelTestHelper(std::string& model) : s(&model) {
	}
	~StringModelTestHelper() {
	}
	void resetModel(Executor&) {
		s->clear();
	}
	Hash hashModel(const Executor&) {
		Hash h;
		h.add(s->c_str());
		return h;
	}
	void dumpModel(std::string& out, const Executor&) {
		out = *s;
	}
};

class AddDelTestBed {
	enum { lineBufferSize = 512 };
	char lineBuffer[lineBufferSize];
	std::string finalString;
	std::string originalString;
	std::string expected;
	std::unique_ptr<CommandFactory> af;
	std::unique_ptr<CommandFactory> df;
	std::unique_ptr<FileCommandLogger> logger;
	std::unique_ptr<Executor> ex;
	FILE* logFile;
	StringModelTestHelper helper;
public:
	AddDelTestBed() :
			af(new AddCharFactory(&finalString)),
			df(new DelCharFactory(&finalString)),
			logger(new FileCommandLogger),
			ex(makeExecutor()),
			logFile(0),
			helper(finalString) {
		ex->setCommandLogger(logger->getLogger());
		ex->addCommand("add", std::move(af), true);
		ex->addCommand("del", std::move(df));
		lineBuffer[lineBufferSize - 1] = '\0';
	}
	void init(const char* initialString) {
		finalString = initialString;
		originalString = finalString;
		logFile = tmpfile();
		// ownership of logFile is passed here
		logger->setLogFile(logFile);
		ex->clearHistory();
	}
	void testUndo() {
		::testUndo(*ex, helper);
	}
};

void TestCommandFactory::testUndo() {
	AddDelTestBed test;
	test.testUndo();
}

void TestCommandFactory::replayIsRobust() {
	loadOomTestUtil();
	setMallocsUntilFailure(0);
	QVERIFY2(0 == malloc(1), "SetMallocsUntilFailure(0) not working");
	setMallocsUntilFailure(1);
	void* shouldSucceed = malloc(1);
	QVERIFY2(shouldSucceed,
			"SetMallocsUntilFailure not allowing mallocs at all");
	free(shouldSucceed);
	QVERIFY2(0 == malloc(1), "SetMallocsUntilFailure(1) not working");
}
