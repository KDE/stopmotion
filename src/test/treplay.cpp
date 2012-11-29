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
		std::string s1;
		std::string s2;
		int32_t i1;
		int32_t i2;
		int32_t i3;
		int32_t i4;
		int32_t i5;
		EtCommand() : s1(""), s2(""), i1(no_num), i2(no_num), i3(no_num),
				i4(no_num), i5(no_num) {
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
	Command& Make(int32_t a, int32_t b, int32_t c) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c, int32_t d) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		e->i4 = d;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c, int32_t d, int32_t f) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		e->i4 = d;
		e->i5 = f;
		return *e;
	}
	Command& Make(const char* s) const {
		EtCommand* e = new EtCommand;
		e->s1 = s;
		return *e;
	}
	Command& Make(int32_t a, const char* s) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->s1 = s;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, const char* s) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->i2 = b;
		e->s1 = s;
		return *e;
	}
	Command& Make(int32_t a, int32_t b, int32_t c, const char* s) const {
		EtCommand* e = new EtCommand;
		e->i1 = a;
		e->i2 = b;
		e->i3 = c;
		e->s1 = s;
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
	cr->RegisterCommandFactory("et", *new EmptyTestCommandFactory());
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
