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

#ifndef COMMAND_H_
#define COMMAND_H_

class Command;
class CommandList;
class FileNameVisitor;
class CommandLogger;

/**
 * Base class of all command classes, objects of which are manipulated by the
 * undo system. Most commands should derive from CommandAtomic.
 */
class Command {
	Command(const Command&);
	Command& operator=(const Command&);
public:
	Command();
	virtual ~Command() = 0;
	/**
	 * Performs its action, creating a NullAdder from inverseStack once only to
	 * add its inverse to the history. For exception safety the model must not
	 * be updated until the inverse command has been constructed and all
	 * necessary memory preallocations have been done to enable an atomic
	 * update of the model.
	 * @return The inverse command, ownership is relinquished.
	 * @note This command will be deleted immediately after this call, so it is
	 * perfectly safe to leave it in an unusable state.
	 */
	virtual Command* execute() = 0;
	/**
	 * calls v.Add(f) for each file f referenced by the command
	 */
	virtual void accept(FileNameVisitor& v) const;
};

/**
 * Creates a command that does nothing. This can be used whenever a command's
 * @c execute function discovers that it does nothing, and it wants to return
 * an inverse that also does nothing.
 * @return New command that does nothing. Ownership is passed.
 */
Command* createNullCommand();

/**
 * Command history for undo and redo.
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
	bool canUndo();
	/**
	 * Returns 'true' if and only if Redo will perform an action, i.e. if
	 * there are any actions in the history to redo.
	 */
	bool canRedo();
	/**
	 * Undoes the last action in the history, if any.
	 */
	void undo();
	/**
	 * Redoes the next action in the history, if any.
	 */
	void redo();
	/**
	 * Executes the command c, placing its inverse into the undo history,
	 * deleting the Redo history. Any partial composite function remaining
	 * after a thrown exception will be on the Redo stack. Ownership of
	 * the command is passed.
	 * @param c The command to be executed.
	 * @param logger @c commandComplete will be called on @c logger if the
	 * execution is successful.
	 */
	void execute(Command& c, CommandLogger* logger);
	/**
	 * Clears all the undo history (and future)
	 */
	void clear();
	/**
	 * Calls v.add(f) for each filename f referenced by the commands in the
	 * history.
	 */
	void accept(FileNameVisitor& v) const;
};

#endif /* COMMAND_H_ */
