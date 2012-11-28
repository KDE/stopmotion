#include <QtTest/QtTest>
#include "moc_treplay.cpp"

#include <string>
#include <memory>
#include <limits>
#include <stdint.h>

#include "../domain/undo/replay.h"
#include "../domain/undo/command.h"

static const int32_t no_num = std::numeric_limits<int32_t>::min();

// Test factory for commands just to test parsing: the commands produced are
// not for execution.
class EmptyTestCommandFactory : public CommandFactory {
public:
	class EtCommand : public CommandAtomic {
	public:
		std::string s;
		int32_t i1;
		int32_t i2;
		int32_t i3;
		EtCommand() : s(""), i1(no_num), i2(no_num), i3(no_num) {
		}
		Command& DoAtomic() {
			// This will fail if this command is ever executed.
			return *this;
		}
	};
	~EmptyTestCommandFactory() {
	}
	Command& Make() const {
		EtCommand* e = new EtCommand;
		return *e;
	}
	Command& Make(int32_t a) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		return *e;
	}
	Command& Make(int32_t a, int32_t b) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->i2 = b;
		return *e;
	}
	Command& Make(int32_t a, const char* s) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->s = s;
		return *e;
	}
};

TestCommandFactory::TestCommandFactory() {
	cr = CommandReplayer::Make();
}

TestCommandFactory::~TestCommandFactory() {
	delete cr;
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

void TestEtCommand(Command* c,
		int32_t exi1, int32_t exi2, int32_t exi3,
		const char* exs) {
	EmptyTestCommandFactory::EtCommand* e
			= static_cast<EmptyTestCommandFactory::EtCommand*>(c);
	QCOMPARE(e->i1, exi1);
	QCOMPARE(e->i2, exi2);
	QCOMPARE(e->i3, exi3);
	QCOMPARE(e->s.c_str(), exs);
}

void TestCommandFactory::allMakeCallsParse() {
	cr->RegisterCommandFactory("et", *new EmptyTestCommandFactory());
	std::auto_ptr<Command> c(&cr->MakeCommand("et"));
	TestEtCommand(c.get(), no_num, no_num, no_num, "");
	std::auto_ptr<Command> ci(&cr->MakeCommand("et 5"));
	TestEtCommand(ci.get(), 5, no_num, no_num, "");
	std::auto_ptr<Command> cii(&cr->MakeCommand("et -5 412345"));
	TestEtCommand(cii.get(), -5, 412345, no_num, "");
	std::auto_ptr<Command> cis(&cr->MakeCommand("et 0 \"ziggy\""));
	TestEtCommand(cis.get(), 0, no_num, no_num, "ziggy");
}
