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

#include "command.h"
#include "../filenamevisitor.h"
#include "undoredoobserver.h"

#include <list>
#include <stdint.h>

// help out Eclipse's C++ parsing
#ifndef INT32_MAX
#define INT32_MAX 0x7FFFFFFF
#endif

/**
 * Command list, for example the Undo or Redo stack.
 */
class CommandList {
	typedef std::list<Command*> clist;
	clist cs;
public:
	/**
	 * Construct a NullAdder on the stack to be able to use FillNull() to add
	 * a command to the command list without the risk of an exception being
	 * thrown.
	 */
	class NullAdder {
		CommandList& cl;
		NullAdder(const NullAdder&);
		NullAdder& operator=(const NullAdder&);
	public:
		NullAdder(CommandList& c) : cl(c) {
			cl.cs.push_front(0);
		}
		~NullAdder() {
			if (!cl.cs.front())
				cl.cs.pop_front();
		}
	};
	friend class NullAdder;
	CommandList() {
	}
	~CommandList() {
		clear();
	}
	/**
	 * Removes all commands from the list
	 */
	void clear() {
		for (Command* com : cs) {
			delete com;
		}
		cs.clear();
	}
	/**
	 * Returns true if and only if the command list is empty.
	 */
	bool empty() const {
		return cs.empty();
	}
	/**
	 * Returns the command at the front of the list
	 */
	Command& front() const {
		return *cs.front();
	}
	/**
	 * Adds a command to the front of the list.
	 */
	void push(Command& c) {
		cs.push_front(&c);
	}
	/**
	 * Adds a command to the back of the list.
	 */
	void add(Command& c) {
		cs.push_back(&c);
	}
	/**
	 * Fills a previously-added null with c. There must be no calls to
	 * Push, Pop, Splice or Clear in between the construction of NullAdder and
	 * FillNull. Will not throw an exception.
	 */
	void fillNull(Command& c) {
		cs.front() = &c;
	}
	/**
	 * Splices in the commands from the list newCommands into the front of
	 * the list. This function will not throw an exception. newCommands
	 * will be empty after this call.
	 */
	void splice(CommandList& newCommands) {
		cs.splice(cs.begin(), newCommands.cs);
	}
	/**
	 * Calls v.Add(f) for each filename f referenced by the commands in the
	 * list.
	 */
	void accept(FileNameVisitor& v) const {
		for (const Command* com : cs) {
			com->accept(v);
		}
	}
	/**
	 * Returns true if there is exactly one command in the list, false
	 * otherwise
	 */
	bool singleton() const {
		return cs.size() == 1;
	}
	/**
	 * Executes the command at the front (if it exists) and puts its inverse
	 * onto the front of 'to'.
	 * @param to The list to receive the inverse command.
	 */
	void executeFront(CommandList& to);
};

void CommandList::executeFront(CommandList& to) {
	if (empty())
		return;
	NullAdder na(to);
	Command* c = cs.front();
	Command* inv = c->execute();
	to.fillNull(*inv);
	// remove command c, which should have been deleted or recycled by
	// execute()
	cs.pop_front();
}

FileNameVisitor::~FileNameVisitor() {
}

void FileNameVisitor::reportNewScene() {
}

Command::Command() {
}

Command::~Command() {
}

void Command::accept(FileNameVisitor&) const {
}

class CommandNull : public Command {
public:
	Command* execute() override {
		// a null command is its own inverse
		return this;
	}
};

Command* createNullCommand() {
	return new CommandNull;
}

CommandFactory::~CommandFactory() {
}

Parameters::~Parameters() {
}

int32_t Parameters::getHowMany() {
	return getInteger(1, INT32_MAX);
}

CommandHistory::CommandHistory() : past(0), future(0), observer(0),
		previousCanUndo(false), previousCanRedo(false) {
	past = new CommandList();
	future = new CommandList();
}

CommandHistory::~CommandHistory() {
	delete past;
	delete future;
}

bool CommandHistory::canUndo() const {
	return !past->empty();
}

bool CommandHistory::canRedo() const {
	return !future->empty();
}

void CommandHistory::undo() {
	past->executeFront(*future);
	notifyObserver();
}

void CommandHistory::redo() {
	future->executeFront(*past);
	notifyObserver();
}

void CommandHistory::execute(Command& c) {
	future->clear();
	future->push(c);
	redo();
	notifyObserver();
}

void CommandHistory::clear() {
	future->clear();
	past->clear();
	notifyObserver();
}

void CommandHistory::accept(FileNameVisitor& v) const {
	future->accept(v);
	past->accept(v);
}

void CommandHistory::setUndoRedoObserver(UndoRedoObserver* ob) {
	observer = ob;
}

void CommandHistory::notifyObserver() {
	bool cu = canUndo();
	if (cu != previousCanUndo) {
		previousCanUndo = cu;
		if (observer)
			observer->updateCanUndo(cu);
	}
	bool cr = canRedo();
	if (cr != previousCanRedo) {
		previousCanRedo = cr;
		if (observer)
			observer->updateCanRedo(cr);
	}
}
