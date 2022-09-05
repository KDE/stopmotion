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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdint.h>
#include <string>

class CommandList;
class FileNameVisitor;
class UndoRedoObserver;
class ErrorHandler;

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
	 * @par
	 * This command must delete itself within the {@c execute} method, unless
	 * it returns itself as its own inverse or otherwise recycles itself.
	 * @return The inverse command; ownership is relinquished.
	 */
	virtual Command* execute() = 0;
	/**
	 * calls v.Add(f) for each file f referenced by the command
	 */
	virtual void accept(FileNameVisitor& v) const;
};

/**
 * A command factory reads the parameters for its command from here. They
 * either come from a log file (if it is being replayed) or from the
 * application.
 */
class Parameters {
public:
	virtual ~Parameters() = 0;
	/**
	 * Returns an integer. Might throw IncorrectParameterException if
	 * unsuccessful, but this behaviour must not be relied upon.
	 * @param min The minimum permissible value that could be returned based
	 * on the current state of the model being altered.
	 * @param max The maximum permissible value that could be returned based
	 * on the current state of the model being altered.
	 * @note @a min and @a max are used by @ref TestUndo in order to create
	 * commands that are appropriate for testing. They are also used to assert
	 * that the values passed to @ref Executor::Execute are within range.
	 */
	virtual int32_t getInteger(int32_t min, int32_t max) = 0;
	/**
	 * Returns an integer from 1 to {@c 1^31-1}. Test code assumes that smaller
	 * numbers are just as good a test as larger numbers, unlike
	 * {@ref getInteger}, which assumes the full range must be tested.
	 */
	virtual int32_t getHowMany();
	/**
	 * Returns a string. If the parameters come from test code, the string will
	 * match the pattern provided. The pattern is ignored in normal code and
	 * may be null.
	 * @param out The returned string.
	 * @param pattern The pattern to which the output should conform; with a
	 * random alphanumeric character replacing each {@c ?} and a random
	 * (possibly zero-length) string of such characters replacing each {@c *}.
	 * Ignored in non-test code. If null, a string of at length at least one is
	 * produced.
	 */
	virtual void getString(std::string& out, const char* pattern) = 0;
};

/**
 * Produces one sort of command.
 */
class CommandFactory {
public:
	virtual ~CommandFactory() = 0;
	/**
	 * Creates a command from the Parameters given in ps.
	 * @param The source of parameters to use, ownership is not passed.
	 * @param e An error handler that collects errors that would have been thrown.
	 * @return The command created, ownership is returned. @c NULL is returned
	 * if no such command can be created at the moment (for example a delete
	 * when the model is empty).
	 */
	virtual Command* create(Parameters& ps, ErrorHandler& e) = 0;
};

/**
 * Creates a command that does nothing. This can be used whenever a command's
 * {@ref Command::execute} function discovers that it does nothing, and it
 * wants to return an inverse that also does nothing.
 * @return New command that does nothing. Ownership is passed.
 */
Command* createNullCommand();

/**
 * Command history for undo and redo.
 */
class CommandHistory {
	CommandList* past;
	CommandList* future;
	UndoRedoObserver* observer;
	bool previousCanUndo;
	bool previousCanRedo;
	void notifyObserver();
public:
	CommandHistory();
	~CommandHistory();
	/**
	 * Sets a new observer to be notified when the functions {@ref canUndo} and
	 * {@ref canRedo} change what they would return.
	 * @param observer The new observer. Any previous observer is unset. A null
	 * pointer means that the old observer will be unset and no new observer
	 * will be set. Ownership is not passed.
	 */
	void setUndoRedoObserver(UndoRedoObserver* observer);
	/**
	 * Returns 'true' if and only if Undo will perform an action, i.e. if
	 * there are any actions in the history to undo.
	 */
	bool canUndo() const;
	/**
	 * Returns 'true' if and only if Redo will perform an action, i.e. if
	 * there are any actions in the history to redo.
	 */
	bool canRedo() const;
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
	 */
	void execute(Command& c);
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
