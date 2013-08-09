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

/**
 * Thrown if a line from the log parses correctly but has extra unexpected
 * characters at its end.
 */
class MalformedLineException {
};

/**
 * Thrown if the parameters for a command (from the log or from a caller) are
 * inappropriate for the current state of the model.
 */
class ParametersOutOfRangeException {
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
	 * @note @c min and @c max are used by @ref TestUndo in order to create
	 * commands that are appropriate for testing. They are also used to assert
	 * that the values passed to @ref Executor::Execute are within range.
	 */
	virtual int32_t GetInteger(int32_t min, int32_t max) = 0;
	/**
	 * Returns the length of string read, which may be greater than maxLength
	 * although no more than maxLength characters will be output into out.
	 * Might throw IncorrectParameterException if unsuccessful, but this
	 * behaviour must not be relied upon.
	 */
	virtual void GetString(std::string& out) = 0;
};

class Command;
class RandomSource;

/**
 * Produces one sort of command.
 */
class CommandFactory {
public:
	virtual ~CommandFactory() = 0;
	/**
	 * Creates a command from the Parameters given in ps.
	 * @param The source of parameters to use, ownership is not passed.
	 * @return The command created, ownership is returned. @c NULL is returned
	 * if no such command can be created at the moment (for example a delete
	 * when the model is empty).
	 */
	virtual Command* Create(Parameters& ps) = 0;
};

class CommandReplayerImpl;
class CommandAndDescriptionFactory;
class CommandLogger;

/**
 * Thrown if @ref CommandFactory::Execute is called with a name for which no
 * command has been added, or if @ref CommandFactory::ExecuteRandomCommands is
 * called on a @ref CommandFactory which has had no commands added, or if
 * @ref CommandFactory::ExecuteRandomConstructiveCommands is called
 * on a @ref CommandFactory which has had no constructive commands set.
 */
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
	 * command expects; in particular, all integers should be @c int32_t. If
	 * you supply a command factory, you should also supply a wrapper for this
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
	 * from a command log previously written by a call to @ref Execute.
	 * @param line Null- or line-ending-terminated string; a line from the log.
	 * @return true if a command was executed, false if the line was empty.
	 * @throws MalformedLineException if the line is neither empty nor starts
	 * with a command name.
	 */
	virtual bool ExecuteFromLog(const char* line) = 0;
	/**
	 * Executes a random set of commands. Used for testing.
	 */
	virtual void ExecuteRandomCommands(RandomSource& rng) = 0;
	/**
	 * Executes a random set of commands. Used for testing for constructing
	 * fresh models from empty models. Only commands that were added with the
	 * @c constructive parameter of @ref AddCommand set to @c true are used.
	 */
	virtual void ExecuteRandomConstructiveCommands(RandomSource& rng) = 0;
	/**
	 * Make a command available for execution. It is assumed that @c name
	 * endures for the lifetime of the Executor. Ideally name should be a
	 * static constant, not heap allocated; for example
	 * @code{.cpp}
	 * executor.AddCommand("addf", addFactory, true);
	 * @endcode
	 * @throws CommandNameAlreadyUsedException if a factory has already
	 * been registered under @c name.
	 * @param name The name of the command. Will need to be supplied in a
	 * call to @ref Execute in order to use @c factory.
	 * @param factory The factory that makes these sort of commands.
	 * @param constructive If true is passed, test code will use this
	 * factory to construct initial model states from empty models. Therefore
	 * @c true should be passed whenever the command is useful in constructing
	 * an initial state. For example, an add operation is constructive, but a
	 * delete or a move is not.
	 * @ref ExecuteRandomConstructiveCommands uses only those factories
	 * passed here with @c constructive set to @c true.
	 */
	virtual void AddCommand(const char* name,
			std::auto_ptr<CommandFactory>factory,
			bool constructive = false) = 0;
	/**
	 * Sets the logger to be used to record commands executed.
	 * @param logger The logger to be set. Ownership is not passed.
	 * Pass NULL to get no logging.
	 */
	virtual void SetCommandLogger(CommandLogger* logger) = 0;
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
 * Create a new executor.
 * @return The new @ref Executor; ownership is returned.
 */
Executor* MakeExecutor();

#endif /* EXECUTOR_H_ */
