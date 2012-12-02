#include <QtTest/QtTest>
#include "moc_treplay.cpp"

#include <string>
#include <memory>
#include <limits>
#include <stdint.h>
#include <cstdlib>

#include "../domain/undo/replay.h"
#include "../domain/undo/command.h"

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
	cr = CommandReplayer::Make();
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

class CloneLogger : public CommandReplayer::Logger {
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
	CloneLogger cl(*cr);
	cr->SetLogger(&cl);
	for (int i = 0; i != 100; ++i) {
		const char* commandName = rand() % 2 == 0? "et" : "sec";
		const CommandFactory* cf = cr->GetCommandFactory(commandName);
		Command* c = 0;
		switch(rand() % 10) {
		case 0:
			c = &cf->Make();
			break;
		case 1:
			c = &cf->Make(randomInt());
			break;
		case 2:
			c = &cf->Make(randomInt(), randomInt());
			break;
		case 3:
			c = &cf->Make(randomInt(), randomInt(), randomInt());
			break;
		case 4:
			c = &cf->Make(randomInt(), randomInt(), randomInt(), randomInt());
			break;
		case 5:
			c = &cf->Make(randomInt(), randomInt(), randomInt(), randomInt(), randomInt());
			break;
		case 6:
			c = &cf->Make(RandomString());
			break;
		case 7:
			c = &cf->Make(randomInt(), RandomString());
			break;
		case 8:
			c = &cf->Make(randomInt(), randomInt(), RandomString());
			break;
		case 9:
			c = &cf->Make(randomInt(), randomInt(), randomInt(), RandomString());
			break;
		}
		QVERIFY(*static_cast<const EmptyTestCommandFactory::EtCommand*>(c)
				== *static_cast<const EmptyTestCommandFactory::EtCommand*>(cl.GetClone()));
	}
	cr->SetLogger(0);
}
