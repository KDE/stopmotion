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

#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <stdint.h>
#include <memory>
#include <string>

/**
 * Thrown if a command factory attempts to read a parameter from the log files
 * that is of the wrong type, is not present or otherwise does not parse.
 */
class IncorrectParameterException {
};

class MalformedLineException {
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
	 */
	virtual int32_t GetInteger() = 0;
	/**
	 * Returns the length of string read, which may be greater than maxLength
	 * although no more than maxLength characters will be output into out.
	 * Might throw IncorrectParameterException if unsuccessful, but this
	 * behaviour must not be relied upon.
	 */
	virtual void GetString(std::string& out) = 0;
};

class Command;

/**
 * Produces one sort of command.
 */
class CommandFactory {
public:
	virtual ~CommandFactory() = 0;
	/**
	 * Creates a command from the Parameters given in ps, returning ownership
	 * of the command, not taking ownership of ps.
	 */
	virtual Command* Create(Parameters& ps) = 0;
};

class CommandReplayerImpl;
class CommandAndDescriptionFactory;

/**
 * For receiving the command's string representation during calls to Make
 * methods.
 */
class CommandLogger {
protected:
	virtual ~CommandLogger() = 0;
public:
	/**
	 * For receiving the serialized command. This function is allowed to
	 * throw an exception.
	 */
	virtual void WriteCommand(const char*) = 0;
};

class UnknownCommandException {
};

class CommandNameAlreadyUsedException {
};

class Executor {
public:
	virtual ~Executor() = 0;
	/**
	 * Executes a command, logging it and putting its inverse onto the undo
	 * stack. Be careful that the argument list exactly matches what the
	 * command expects; in particular, all integers should be int32_t. If you
	 * supply a command factory, you should also supply a wrapper for this
	 * Execute function to make sure this is done correctly, for example:
	 * @code{.cpp}
	 * void ExecuteAddFrame(Executor& e, const char* filename, int32_t sceneNo,
	 *     int32_t frameNo) {
	 *   e.Execute("addf", filename, sceneNo, frameNo);
	 * }
	 * @endcode
	 */
	virtual void Execute(const char* name, ...) = 0;
	/**
	 * Executes the command described by (the first line of) @c line, a line
	 * from a command log previously written by a call to @ Execute.
	 * @param line Null- or line-ending-terminated string; a line from the log.
	 * @return true if a command was executed, false if the line was empty.
	 * @throws MalformedLineException if the line is neither empty nor starts
	 * with a command name.
	 */
	virtual bool ExecuteFromLog(const char* line) = 0;
	/**
	 * Make a command available for execution. It is assumed that @c name
	 * endures for the lifetime of the Executor. Ideally name should be a
	 * static constant, not heap allocated; for example
	 * @code{.cpp}
	 * executor.AddCommand("addf", addFactory);
	 * @endcode
	 * Throws CommandNameAlreadyUsedException if a factory has already
	 * been registered under @c name.
	 */
	virtual void AddCommand(const char* name,
			std::auto_ptr<CommandFactory>factory) = 0;
	/**
	 * Clears the undo and redo stacks.
	 */
	virtual void ClearHistory() = 0;
	/**
	 * Undoes the most recent command.
	 * @return true on success, false if nothing was on the undo stack.
	 */
	virtual bool Undo() = 0;
	/**
	 * Redoes the most recently undone command.
	 * @return true on success, false if nothing was on the redo stack.
	 */
	virtual bool Redo() = 0;
};

/**
 * @param logger The logger to be used; ownership is not passed.
 */
Executor* MakeExecutor(CommandLogger* logger);

#endif /* EXECUTOR_H_ */
