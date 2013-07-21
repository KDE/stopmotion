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
#include "texecutor.h"

#include <QtTest/QtTest>

#include <memory>
#include <limits>
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <sstream>

#include "src/domain/undo/executor.h"
#include "src/domain/undo/command.h"
#include "src/domain/undo/logger.h"

#include "oomtestutil.h"

static const int32_t no_num = std::numeric_limits<int32_t>::min();

class NullCommand : public CommandAtomic {
public:
	Command& DoAtomic() {
		return *new NullCommand();
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
	class EtCommand : public CommandAtomic {
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
		Command& DoAtomic() {
			std::stringstream ss;
			ss << name << ",i:" << i1 << ",s:" << s1 << ",i:" << i2;
			output.push_back(ss.str());
			return *new NullCommand();
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
	Command* Create(Parameters& ps) {
		EtCommand* e = new EtCommand(name, output);
		e->i1 = ps.GetInteger();
		ps.GetString(e->s1);
		e->i2 = ps.GetInteger();
		return e;
	}
};

/**
 * Re-executes the command that is logged.
 */
class CloneLogger : public CommandLogger {
	Executor* ex;
public:
	CloneLogger() : ex(0) {
	}
	~CloneLogger() {
	}
	/** @param e Ownership is not passed. */
	void SetExecutor(Executor* e) {
		ex = e;
	}
	void WriteCommand(const char* lineToLog) {
		ex->ExecuteFromLog(lineToLog);
	}
};

TestCommandFactory::TestCommandFactory()
		: ce(0), cl(0), str(0), strNext(0), strAllocLen(0) {
	cl = new CloneLogger();
	ce = MakeExecutor(cl);
	cl->SetExecutor(ce);
	std::auto_ptr<CommandFactory> et(
			new EmptyTestCommandFactory("et", executionOutput));
	std::auto_ptr<CommandFactory> sec(
			new EmptyTestCommandFactory("sec", executionOutput));
	ce->AddCommand("et", et);
	ce->AddCommand("sec", sec);
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
		ce->Execute("fakeCommand");
	} catch (UnknownCommandException& e) {
		return;
	}
	QFAIL("Empty CommandReplayer did not throw "
			"CommandFactoryNoSuchCommandException");
}

void TestCommandFactory::canParseFromLog() {
	executionOutput.clear();
	ce->ExecuteFromLog("et -5 \"hello world!\" 412345");
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
		ce->Execute(commandName, i1, s1.c_str(), i2);
		output_t::iterator eo1 = executionOutput.begin();
		output_t::iterator eo2 = eo1;
		++eo2;
		QCOMPARE(eo1->c_str(), eo2->c_str());
	}
}

class AddCharFactory : public CommandFactory {
	std::string* model;
public:
	class AddChar : public CommandAtomic {
		std::string* m;
		char c;
		int32_t p;
	public:
		AddChar(std::string& model, int32_t character, int32_t position)
			: m(&model), c(Nice(character)), p(position) {
		}
		Command& DoAtomic();
		static char Nice(int32_t c) {
			// Make sure any valid character maps to itself.
			// This makes constructing inverses easy (otherwise we would need
			// an UnNice function)
			if ('0' <= c && c <= 'z')
			{
				if ('a' <= c || c <= '9')
					return c;
				if ('A' <= c && c <= 'Z')
					return c;
				if (c == '@' || c == '=')
					return c;
			}
			if (c < 0)
				c = 1 - c;
			return "abcdefghijklmnopqrstuvwxyz"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"0123456789@="[c%64];
		}
	};
	AddCharFactory(std::string* m) : model(m) {
	}
	Command* Create(Parameters& ps) {
		int32_t character = ps.GetInteger();
		int32_t position = ps.GetInteger();
		return new AddChar(*model, character, position);
	}
};

class DelCharFactory : public CommandFactory {
	std::string* model;
public:
	class DelChar : public CommandAtomic {
		std::string* m;
		int32_t p;
	public:
		DelChar(std::string& model, int32_t position)
			: m(&model), p(position) {
		}
		Command& DoAtomic();
	};
	DelCharFactory(std::string* m) : model(m) {
	}
	Command* Create(Parameters& ps) {
		int32_t position = ps.GetInteger();
		return new DelChar(*model, position);
	}
};

Command& AddCharFactory::AddChar::DoAtomic() {
	int32_t pos = (p < 0? 1 - p : p) % (m->size() + 1);
	// insert might throw, so use an auto_ptr to avoid leaks.
	std::auto_ptr<Command> inv(new DelCharFactory::DelChar(*m, pos));
	std::string::iterator i = m->begin();
	i += pos;
	m->insert(i, c);
	return *inv.release();
}

Command& DelCharFactory::DelChar::DoAtomic() {
	if (m->size() == 0) {
		return *new CommandNull;
	}
	int32_t pos = (p < 0? 1 - p : p) % m->size();
	char removedChar = (*m)[pos];
	Command* inv = new AddCharFactory::AddChar(*m, removedChar, pos);
	m->erase(pos, 1);
	return *inv;
}

class AddDelTestBed {
	enum { lineBufferSize = 512 };
	char lineBuffer[lineBufferSize];
	std::string finalString;
	std::string originalString;
	std::string expected;
	std::auto_ptr<CommandFactory> af;
	std::auto_ptr<CommandFactory> df;
	std::auto_ptr<FileCommandLogger> logger;
	std::auto_ptr<Executor> ex;
	FILE* logFile;
public:
	AddDelTestBed() :
			af(new AddCharFactory(&finalString)),
			df(new DelCharFactory(&finalString)),
			logger(new FileCommandLogger),
			ex(MakeExecutor(logger->GetLogger())),
			logFile(0) {
		ex->AddCommand("add", af);
		ex->AddCommand("del", df);
		lineBuffer[lineBufferSize - 1] = '\0';
	}
	void Init(const char* initialString) {
		finalString = initialString;
		originalString = finalString;
		logFile = tmpfile();
		// ownership of logFile is passed here
		logger->SetLogFile(logFile);
		ex->ClearHistory();
	}
	void SetExpected() {
		expected = finalString;
	}
	void ResetToOriginal() {
		finalString = originalString;
		fflush(logFile);
		rewind(logFile);
		ex->ClearHistory();
	}
	void CheckRunsEqual() {
		QCOMPARE(finalString.c_str(), expected.c_str());
	}
	void CheckAsOriginal() {
		QCOMPARE(finalString.c_str(), originalString.c_str());
	}
	bool ExecuteLineFromFile() {
		if (!fgets(lineBuffer, lineBufferSize, logFile))
			return false;
		ex->ExecuteFromLog(lineBuffer);
		return true;
	}
	bool Undo() {
		return ex->Undo();
	}
	bool Redo() {
		return ex->Redo();
	}
	bool ExecuteRandomCommand() {
		int32_t commandType = rand() % 5;
		if (commandType == 0)
			return false;
		switch (commandType) {
		case 1:
		case 2:
		{
			int32_t ch = rand();
			int32_t pos = rand();
			ex->Execute("add", ch, pos);
			break;
		}
		default:
		{
			int32_t pos = rand();
			ex->Execute("del", pos);
			break;
		}
		}
		return true;
	}
	void ExecuteRandomCommands() {
		while (ExecuteRandomCommand()) {
			logger->CommandComplete();
		}
	}
};

void TestCommandFactory::replaySequenceProducesSameOutput() {
	AddDelTestBed test;
	for (int i = 0; i != 100; ++i) {
		test.Init(RandomString());
		test.ExecuteRandomCommands();
		test.SetExpected();
		test.ResetToOriginal();
		while (test.ExecuteLineFromFile()) {
		}
		test.CheckRunsEqual();
		// and check that the reproduced history undoes all the way back
		while (test.Undo()) {
		}
		test.CheckAsOriginal();
		// finally check that the reproduced history redoes all the way
		while (test.Redo()) {
		}
		test.CheckRunsEqual();
	}
}

void TestCommandFactory::undoPutsModelBack() {
	AddDelTestBed test;
	for (int i = 0; i != 100; ++i) {
		test.Init(RandomString());
		while (!test.ExecuteRandomCommand()) {
		}
		test.SetExpected();
		test.Undo();
		test.CheckAsOriginal();
		test.Redo();
		test.CheckRunsEqual();
	}
}

void TestCommandFactory::replayIsRobust() {
	LoadOomTestUtil();
	SetMallocsUntilFailure(0);
	QVERIFY2(0 == malloc(1), "SetMallocsUntilFailure(0) not working");
	SetMallocsUntilFailure(1);
	void* shouldSucceed = malloc(1);
	QVERIFY2(shouldSucceed,
			"SetMallocsUntilFailure not allowing mallocs at all");
	free(shouldSucceed);
	QVERIFY2(0 == malloc(1), "SetMallocsUntilFailure(1) not working");
}
