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

#include "command.h"
#include "../filenamevisitor.h"
#include "commandlogger.h"

#include <list>
#include <memory>

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
		Clear();
	}
	/**
	 * Removes all commands from the list
	 */
	void Clear() {
		for (clist::iterator i = cs.begin(); i != cs.end(); ++i) {
			delete *i;
		}
		cs.clear();
	}
	/**
	 * Returns true if and only if the command list is empty.
	 */
	bool Empty() const {
		return cs.empty();
	}
	/**
	 * Returns the command at the front of the list
	 */
	Command& Front() const {
		return *cs.front();
	}
	/**
	 * Adds a command to the front of the list.
	 */
	void Push(Command& c) {
		cs.push_front(&c);
	}
	/**
	 * Adds a command to the back of the list.
	 */
	void Add(Command& c) {
		cs.push_back(&c);
	}
	/**
	 * Fills a previously-added null with c. There must be no calls to
	 * Push, Pop, Splice or Clear in between the construction of NullAdder and
	 * FillNull. Will not throw an exception.
	 */
	void FillNull(Command& c) {
		cs.front() = &c;
	}
	/**
	 * Splices in the commands from the list newCommands into the front of
	 * the list. This function will not throw an exception. newCommands
	 * will be empty after this call.
	 */
	void Splice(CommandList& newCommands) {
		cs.splice(cs.begin(), newCommands.cs);
	}
	/**
	 * Calls v.Add(f) for each filename f referenced by the commands in the
	 * list.
	 */
	void Accept(FileNameVisitor& v) const {
		for (clist::const_iterator i = cs.begin(); i != cs.end(); ++i) {
			(*i)->Accept(v);
		}
	}
	/**
	 * Returns true if there is exactly one command in the list, false
	 * otherwise
	 */
	bool Singleton() const {
		return cs.size() == 1;
	}
	/**
	 * Executes the command at the front (if it exists) and puts its inverse
	 * onto the front of 'to'.
	 * @param to The list to receive the inverse command.
	 */
	void ExecuteFront(CommandList& to);
};

void CommandList::ExecuteFront(CommandList& to) {
	if (Empty())
		return;
	NullAdder na(to);
	Command* c = cs.front();
	Command* inv = c->Do();
	to.FillNull(*inv);
	delete c;
	// remove deleted command
	cs.pop_front();
}

FileNameVisitor::~FileNameVisitor() {
}

Command::Command() {
}

Command::~Command() {
}

void Command::Accept(FileNameVisitor&) const {
}

class CommandNull : public Command {
public:
	Command* Do() {
		return new CommandNull;
	}
};

Command* CreateNullCommand() {
	return new CommandNull;
}

CommandHistory::CommandHistory() : past(0), future(0) {
	past = new CommandList();
	future = new CommandList();
}

CommandHistory::~CommandHistory() {
	delete past;
	delete future;
}

bool CommandHistory::CanUndo() {
	return !past->Empty();
}

bool CommandHistory::CanRedo() {
	return !future->Empty();
}

void CommandHistory::Undo() {
	past->ExecuteFront(*future);
}

void CommandHistory::Redo() {
	future->ExecuteFront(*past);
}

void CommandHistory::Do(Command& c, CommandLogger* logger) {
	future->Clear();
	future->Push(c);
	Redo();
	if (logger) {
		logger->CommandComplete();
	}
}

void CommandHistory::Clear() {
	future->Clear();
	past->Clear();
}

void CommandHistory::Accept(FileNameVisitor& v) const {
	future->Accept(v);
	past->Accept(v);
}
