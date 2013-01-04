#include <QtTest/QtTest>
#include "moc_treplay.cpp"

#include <string>
#include <memory>
#include <limits>
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>

#include "src/domain/undo/replay.h"
#include "src/domain/undo/command.h"
#include "src/domain/undo/logger.h"

static const int32_t no_num = std::numeric_limits<int32_t>::min();

// Test factory for commands just to test parsing: the commands produced are
// not for execution.
class EmptyTestCommandFactory : public CommandFactory {
	std::string name;
public:
	EmptyTestCommandFactory(const char* nameForCommand)
			: name(nameForCommand) {
	}
	EmptyTestCommandFactory() : name("") {
	}
	class EtCommand : public CommandAtomic {
	public:
		std::string name;
		std::string s1;
		std::string s2;
		int32_t i1;
		int32_t i2;
		int32_t i3;
		int32_t i4;
		int32_t i5;
		EtCommand(std::string commandName)
				: name(commandName), s1(""), s2(""),
				  i1(no_num), i2(no_num), i3(no_num), i4(no_num), i5(no_num) {
		}
		Command& DoAtomic() {
			// This will fail if this command is ever executed.
			return *this;
		}
		bool operator==(const EtCommand& other) const {
			return name == other.name
					&& s1 == other.s1
					&& s2 == other.s2
					&& i1 == other.i1
					&& i2 == other.i2
					&& i3 == other.i3
					&& i4 == other.i4
					&& i5 == other.i5;
		}
		bool operator!=(const EtCommand& other) const {
			return !(*this == other);
		}
	};
	~EmptyTestCommandFactory() {
	}
	Command& Make() const {
		EtCommand* e = new EtCommand(name);
		return *e;
	}
	Command& Make(int32_t a) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		return *e;
	}
	Command& Make(int32_t a, int32_t b) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->i2 = b;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c, int32_t d) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		e->i4 = d;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c, int32_t d, int32_t f) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		e->i4 = d;
		e->i5 = f;
		return *e;
	}
	Command& Make(const char* s) const {
		EtCommand* e = new EtCommand(name);
		e->s1 = s;
		return *e;
	}
	Command& Make(int32_t a, const char* s) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->s1 = s;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, const char* s) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->i2 = b;
		e->s1 = s;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c, const char* s) const {
		EtCommand* e = new EtCommand(name);
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		e->s1 = s;
		return *e;
	}
};

TestCommandFactory::TestCommandFactory() : str(0), strNext(0), strAllocLen(0) {
	cr = new CommandReplayer();
	cr->RegisterCommandFactory("et", *new EmptyTestCommandFactory("et"));
	cr->RegisterCommandFactory("sec", *new EmptyTestCommandFactory("sec"));
}

TestCommandFactory::~TestCommandFactory() {
	delete cr;
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
		cr->MakeCommand("fakeCommand");
	} catch (CommandFactoryNoSuchCommandException& e) {
		return;
	}
	QFAIL("Empty CommandReplayer did not throw "
			"CommandFactoryNoSuchCommandException");
}

class CloneLogger : public CommandLogger {
	CommandReplayer& cr;
	std::auto_ptr<Command> clone;
public:
	CloneLogger(CommandReplayer& replayer) : cr(replayer) {
	}
	~CloneLogger() {
	}
	void WriteCommand(const char* lineToLog) {
		clone.reset(&cr.MakeCommand(lineToLog));
	}
	const Command* GetClone() const {
		return clone.get();
	}
};

void TestEtCommand(Command* c,
		int32_t exi1, int32_t exi2, int32_t exi3, int32_t exi4, int32_t exi5,
		const char* exs) {
	EmptyTestCommandFactory::EtCommand* e
			= static_cast<EmptyTestCommandFactory::EtCommand*>(c);
	QCOMPARE(e->i1, exi1);
	QCOMPARE(e->i2, exi2);
	QCOMPARE(e->i3, exi3);
	QCOMPARE(e->i4, exi4);
	QCOMPARE(e->i5, exi5);
	QCOMPARE(e->s1.c_str(), exs);
}

void TestCommandFactory::allMakeCallsParse() {
	std::auto_ptr<Command> c(&cr->MakeCommand("et"));
	TestEtCommand(c.get(), no_num, no_num, no_num, no_num, no_num, "");
	std::auto_ptr<Command> ci(&cr->MakeCommand("et 5"));
	TestEtCommand(ci.get(), 5, no_num, no_num, no_num, no_num, "");
	std::auto_ptr<Command> cii(&cr->MakeCommand("et -5 412345"));
	TestEtCommand(cii.get(), -5, 412345, no_num, no_num, no_num, "");
	std::auto_ptr<Command> ciii(&cr->MakeCommand("et 765 5 3"));
	TestEtCommand(ciii.get(), 765, 5, 3, no_num, no_num, "");
	std::auto_ptr<Command> ciiii(&cr->MakeCommand("et 1 2 3 4"));
	TestEtCommand(ciiii.get(), 1, 2, 3, 4, no_num, "");
	std::auto_ptr<Command> ciiiii(&cr->MakeCommand("et 1 2 3 4 5"));
	TestEtCommand(ciiiii.get(), 1, 2, 3, 4, 5, "");
	std::auto_ptr<Command> cs(&cr->MakeCommand("et \"jim\""));
	TestEtCommand(cs.get(), no_num, no_num, no_num, no_num, no_num, "jim");
	std::auto_ptr<Command> cis(&cr->MakeCommand("et 0 \"ziggy\""));
	TestEtCommand(cis.get(), 0, no_num, no_num, no_num, no_num, "ziggy");
	std::auto_ptr<Command> ciis(&cr->MakeCommand("et -56 -925235 \"ziggy\""));
	TestEtCommand(ciis.get(), -56, -925235, no_num, no_num, no_num, "ziggy");
	std::auto_ptr<Command> ciiis(&cr->MakeCommand("et 1 2 10000000 \"ziggy\""));
	TestEtCommand(ciiis.get(), 1, 2, 10000000, no_num, no_num, "ziggy");
}

int32_t randomInt() {
	return rand() - RAND_MAX/2;
}

void TestCommandFactory::parsingDescriptionIsCloning() {
	char description[512];
	const size_t desLen = sizeof(description)/sizeof(description[0]) - 1;
	description[desLen] = '\0';
	CloneLogger cl(*cr);
	cr->SetLogger(&cl);
	for (int i = 0; i != 100; ++i) {
		const char* commandName = rand() % 2 == 0? "et" : "sec";
		const CommandFactory* cf = cr->GetCommandFactory(commandName);
		std::auto_ptr<Command> c;
		int32_t i1, i2, i3, i4, i5;
		std::string s1;
		switch(rand() % 10) {
		case 0:
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make()");
			c.reset(&cf->Make());
			break;
		case 1:
			i1 = randomInt();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d)", i1);
			c.reset(&cf->Make(i1));
			break;
		case 2:
			i1 = randomInt();
			i2 = randomInt();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %d)", i1, i2);
			c.reset(&cf->Make(i1, i2));
			break;
		case 3:
			i1 = randomInt();
			i2 = randomInt();
			i3 = randomInt();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %d, %d)", i1, i2, i3);
			c.reset(&cf->Make(i1, i2, i3));
			break;
		case 4:
			i1 = randomInt();
			i2 = randomInt();
			i3 = randomInt();
			i4 = randomInt();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %d, %d, %d)", i1, i2, i3, i4);
			c.reset(&cf->Make(i1, i2, i3, i4));
			break;
		case 5:
			i1 = randomInt();
			i2 = randomInt();
			i3 = randomInt();
			i4 = randomInt();
			i5 = randomInt();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %d, %d, %d, %d)", i1, i2, i3, i4, i5);
			c.reset(&cf->Make(i1, i2, i3, i4, i5));
			break;
		case 6:
			s1 = RandomString();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%s)", s1.c_str());
			c.reset(&cf->Make(s1.c_str()));
			break;
		case 7:
			i1 = randomInt();
			s1 = RandomString();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %s)", i1, s1.c_str());
			c.reset(&cf->Make(i1, s1.c_str()));
			break;
		case 8:
			i1 = randomInt();
			i2 = randomInt();
			s1 = RandomString();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %d, %s)", i1, i2, s1.c_str());
			c.reset(&cf->Make(i1, i2, s1.c_str()));
			break;
		case 9:
			i1 = randomInt();
			i2 = randomInt();
			i3 = randomInt();
			s1 = RandomString();
			snprintf(description, sizeof(description)/sizeof(description[0]), "Make(%d, %d, %d, %s)", i1, i2, i3, s1.c_str());
			c.reset(&cf->Make(i1, i2, i3, s1.c_str()));
			break;
		}
		QVERIFY2(*static_cast<const EmptyTestCommandFactory::EtCommand*>(c.get())
				== *static_cast<const EmptyTestCommandFactory::EtCommand*>(cl.GetClone()),
				description);
	}
	cr->SetLogger(0);
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
			: m(&model), c(character % 128), p(position) {
		}
		Command& DoAtomic();
	};
	AddCharFactory(std::string& m) : model(&m) {
	}
	Command& Make(int32_t character, int32_t position) const {
		return *new AddChar(*model, character, position);
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
	DelCharFactory(std::string& m) : model(&m) {
	}
	Command& Make(int32_t position) const {
		return *new DelChar(*model, position);
	}
};

Command& AddCharFactory::AddChar::DoAtomic() {
	int32_t pos = (p < 0? -p : p) % (m->size() + 1);
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
	int32_t pos = (p < 0? -p : p) % m->size();
	char removedChar = (*m)[pos];
	Command* inv = new AddCharFactory::AddChar(*m, pos, removedChar);
	m->erase(pos, 1);
	return *inv;
}

void TestCommandFactory::replaySequenceProducesSameOutput() {
	char lineBuffer[512];
	CommandHistory history;
	std::string finalString;
	std::string originalString;
	std::auto_ptr<AddCharFactory> af(new AddCharFactory(finalString));
	std::auto_ptr<DelCharFactory> df(new DelCharFactory(finalString));
	std::auto_ptr<CommandReplayer> rep(new CommandReplayer());
	rep->RegisterCommandFactory("add", *af);
	rep->RegisterCommandFactory("del", *df);
	FILE* logFile = tmpfile();
	std::auto_ptr<FileCommandLogger> logger(new FileCommandLogger);
	logger->SetLogFile(logFile);
	rep->SetLogger(logger->GetLogger());
	history.SetPartialCommandObserver(logger->GetPartialCommandObserver());
	for (int i = 0; i != 100; ++i) {
		rewind(logFile);
		history.Clear();
		finalString = RandomString();
		originalString = finalString;
		int32_t commandType;
		while (0 != (commandType = rand() % 45)) {
			Command* c;
			switch (commandType) {
			case 1:
			case 2:
				c = &rep->GetCommandFactory("add")->Make(rand(), rand());
				break;
			default:
				c = &rep->GetCommandFactory("del")->Make(rand());
				break;
			}
			history.Do(*c);
		}
		std::string finalString1 = finalString;
		finalString = originalString;
		rewind(logFile);
		history.Clear();
		const size_t bufferSize = sizeof(lineBuffer)/sizeof(lineBuffer[0]) - 1;
		lineBuffer[bufferSize] = '\0';
		while (fgets(lineBuffer, bufferSize, logFile)) {
			history.Do(rep->MakeCommand(lineBuffer));
		}
		QCOMPARE(finalString, finalString1);
	}
}
