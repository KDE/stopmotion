#include <QtTest/QtTest>
#include "moc_treplay.cpp"

#include "../domain/undo/replay.h"

void TestCommandFactory::emptyCommandReplayerThrows() {
	CommandReplayer* cr = MakeCommandReplayer();
	try {
		MakeCommand(*cr, "fakeCommand");
	} catch (CommandFactoryNoSuchCommandException& e) {
		return;
	}
	QFAIL("Empty CommandReplayer did not throw "
			"CommandFactoryNoSuchCommandException");
}
