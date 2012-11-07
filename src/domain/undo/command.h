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

#ifndef COMMAND_H_
#define COMMAND_H_

class Command;
class CommandList;
class CommandHistoryAdder;
class FileNameVisitor;

/**
 * Adds c to the history via a. For use in Command::Do.
 * Might throw an exception. If so, c is deleted.
 * @author Tim Band
 */
void AddToCommandHistory(CommandHistoryAdder& a, Command& c);

/**
 * Internal exception; command attempted to add two inverses to the history
 */
class CommandDoubleAddToHistory {
};

/**
 * Internal exception; attempt to remove an item from an empty command list
 */
class CommandListPopEmpty {
};

/**
 * Base class of all command classes, objects of which are manipulated by the
 * undo system. Most commands should derive from CommandAtomic.
 * @author Tim Band
 */
class Command {
	Command(const Command&);
	Command& operator=(const Command&);
public:
	static const int allParts = -1;
	Command();
	virtual ~Command() = 0;
	/**
	 * Performs its action, calling 'adder' once only to add its inverse
	 * to the history. For exception safety the following conditions must
	 * be met: At each point where an exception might be thrown (1) the
	 * 'this' object must represent only the remaining parts of the action
	 * yet to be performed and (2) the object (already) added to 'adder'
	 * must represent the inverse of all the actions that have so far been
	 * performed successfully.
	 * @param adder Proxy for the command list that the inverse will be
	 * added to.
	 * @param parts The maximum number of sub-parts to be executed. If there
	 * are more sub-parts than this, the remaining parts remain as part of
	 * this command and only the inverses of the parts executed are pushed to
	 * adder. Pass in Command::allParts for all the sub-parts to be executed.
	 * @return Number of sub-parts actually executed.
	 */
	virtual int Do(CommandHistoryAdder& adder, int parts) = 0;
	/**
	 * calls v.Add(f) for each file f referenced by the command
	 */
	virtual void Accept(FileNameVisitor& v) const;
};

/**
 * Base class of all 'atomic' command classes, that is, ones that will not
 * throw exceptions in their operation (except perhaps in a preparation method)
 * @author Tim Band
 */
class CommandAtomic : public Command {
public:
	// do not override further
	int Do(CommandHistoryAdder& adder, int parts);
	/**
	 * Perform the action itself, relinquishing ownership of any owned
	 * objects that have been passed to others (for example nulling their
	 * pointers), and returning an inverse command. Any exception thrown
	 * must be thrown before any part of the action has taken place.
	 */
	virtual Command& DoAtomic() = 0;
};

/**
 * Command made up of other commands. If execution of the composite is
 * interrupted by an exception being thrown then (provided that all the
 * commands making it up are well-behaved in this regard) the command history
 * will be in the correct state: with all parts not yet performed still on the
 * redo stack (or undo if undoing) and the inverses of all the parts so far
 * performed on the undo stack (or redo if undoing).
 * @author Tim Band
 */
class CommandComposite : public Command {
	CommandList* cs;
public:
	CommandComposite();
	~CommandComposite();
	/**
	 * Add another command to the list (to be executed after all the
	 * previously added commands).
	 */
	void Add(Command&);
	int Do(CommandHistoryAdder& adder, int parts);
	void Accept(FileNameVisitor& v) const;
};

/**
 * Command history for undo and redo.
 * @author Tim Band
 */
class CommandHistory {
	CommandList* past;
	CommandList* future;
public:
	CommandHistory();
	~CommandHistory();
	/**
	 * Returns 'true' if and only if Undo will perform an action, i.e. if
	 * there are any actions in the history to undo.
	 */
	bool CanUndo();
	/**
	 * Returns 'true' if and only if Redo will perform an action, i.e. if
	 * there are any actions in the history to redo.
	 */
	bool CanRedo();
	/**
	 * Undoes the last action in the history, if any.
	 */
	void Undo();
	/**
	 * Undoes the last action in the history, if any
	 * @param parts The maximum number of sub-parts to be executed. If this is
	 * fewer than the actual number of sub-parts in the command, the remainder
	 * remain on the Undo stack.
	 */
	void Undo(int parts);
	/**
	 * Redoes the next action in the history, if any.
	 */
	void Redo();
	/**
	 * Redoes the next action in the history, if any.
	 * @param parts The maximum number of sub-parts to be executed. If this is
	 * fewer than the actual number of sub-parts in the command, the remainder
	 * remain on the Redo stack.
	 */
	void Redo(int parts);
	/**
	 * Executes the command c, placing its inverse into the undo history,
	 * deleting the Redo history. Any partial composite function remaining
	 * after a thrown exception will be on the Redo stack.
	 */
	void Do(Command&);
	/**
	 * Clears all the undo history (and future)
	 */
	void Clear();
	/**
	 * Calls v.Add(f) for each filename f referenced by the commands in the
	 * history.
	 */
	void Accept(FileNameVisitor& v) const;
};

#endif /* COMMAND_H_ */
