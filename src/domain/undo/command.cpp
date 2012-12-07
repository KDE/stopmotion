/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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

#include <list>
#include <memory>

/**
 * Command list, for example the Undo or Redo stack.
 * @author Tim Band
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
			cl.RemoveImpotentFront();
		}
	};
	friend class NullAdder;
	CommandList() {
	}
	~CommandList() {
		Clear();
	}
	/**
	 * Removes an impotent command from the front of the list, if any
	 */
	void RemoveImpotentFront() {
		if (!cs.empty()) {
			Command* c = cs.front();
			if (c && c->Impotent()) {
				delete c;
				c = 0;
			}
			if (!c)
				cs.pop_front();
		}
	}
	/**
	 * Removes all commands from the list
	 */
	void Clear() {
		for (clist::iterator i = cs.begin(); i != cs.end(); ++i) {
			delete *i;
		}
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
	bool Impotent() const {
		for (clist::const_iterator i = cs.begin(); i != cs.end(); ++i) {
			if (*i && !(*i)->Impotent())
				return false;
		}
		return true;
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
	 * @param parts The maximum number of sub-parts to be executed, or
	 * Command::allParts if there is to be no limit. If not all sub-parts are
	 * executed, the remainder remain on the front of this command list.
	 * @param partObserver partObserver.AtomicComplete() is called once for
	 * each sub-part of any composite.
	 * @return The actual number of sub-parts executed.
	 */
	int ExecuteFront(CommandList& to, int parts,
			PartialCommandObserver* partObserver);
};

int CommandList::ExecuteFront(CommandList& to, int parts,
		PartialCommandObserver* partObserver) {
	if (Empty())
		return 0;
	NullAdder na(to);
	int p = cs.front()->Do(to, parts, partObserver);
	// remove deleted command
	cs.pop_front();
	return p;
}

FileNameVisitor::~FileNameVisitor() {
}

Command::Command() {
}

Command::~Command() {
}

void Command::Accept(FileNameVisitor&) const {
}

int CommandAtomic::Do(CommandList& cs, int parts,
		PartialCommandObserver*) {
	if (parts == 0)
		throw CompositeInterruptedException();
	Command& c = DoAtomic();
	cs.FillNull(c);
	delete this;
	return 1;
}

bool CommandAtomic::Impotent() const {
	return false;
}

Command& CommandNull::DoAtomic() {
	return *new CommandNull;
}

CommandHistory::CommandHistory() : past(0), future(0), partObserver(0) {
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
	past->ExecuteFront(*future, Command::allParts, partObserver);
}

void CommandHistory::Undo(int parts) {
	past->ExecuteFront(*future, parts, partObserver);
}

void CommandHistory::Redo() {
	future->ExecuteFront(*past, Command::allParts, partObserver);
}

void CommandHistory::Redo(int parts) {
	future->ExecuteFront(*past, parts, partObserver);
}

void CommandHistory::Do(Command& c) {
	future->Clear();
	future->Push(c);
	Redo();
}

void CommandHistory::Clear() {
	future->Clear();
	past->Clear();
}

void CommandHistory::Accept(FileNameVisitor& v) const {
	future->Accept(v);
	past->Accept(v);
}

void CommandHistory::SetPartialCommandObserver(PartialCommandObserver* ob) {
	partObserver = ob;
}

CommandComposite::CommandComposite() : cs(0) {
	cs = new CommandList();
}

CommandComposite::~CommandComposite() {
	delete cs;
}

void CommandComposite::Add(Command& c) {
	cs->Add(c);
}

int CommandComposite::Do(CommandList& invs, int parts,
		PartialCommandObserver* partObserver) {
	int actualParts = 0;
	if (cs->Impotent()) {
		// nothing to be done here
	} else if (cs->Singleton()) {
		// Special case for a singleton; this will mean that instead of
		// creating a singleton composite inverse, we just create the
		// inverse of a singleton.
		actualParts = cs->Front().Do(invs, parts, partObserver);
		if (partObserver)
			partObserver->AtomicComplete();
	} else {
		CommandComposite *c = new CommandComposite();
		invs.FillNull(*c);
		while (!cs->Empty()) {
			int newParts = cs->ExecuteFront(*c->cs, parts, partObserver);
			parts -= newParts;
			actualParts += newParts;
			if (partObserver)
				partObserver->AtomicComplete();
		}
	}
	delete this;
	return actualParts;
}

void CommandComposite::Accept(FileNameVisitor& v) const {
	cs->Accept(v);
}

bool CommandComposite::Impotent() const {
	return cs->Impotent();
}

PartialCommandObserver::~PartialCommandObserver() {
}
