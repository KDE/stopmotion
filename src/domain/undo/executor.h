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

#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <exception>
#include <memory>

/**
 * Thrown if a command factory attempts to read a parameter from the log files
 * that is of the wrong type, is not present or otherwise does not parse.
 */
class IncorrectParameterException : public std::exception {
	const char* what() const throw();
};

/**
 * Thrown if a line from the log parses correctly but has extra unexpected
 * characters at its end.
 */
class MalformedLineException : public std::exception {
	const char* what() const throw();
};

/**
 * Thrown if the parameters for a command (from the log or from a caller) are
 * inappropriate for the current state of the model.
 */
class ParametersOutOfRangeException : public std::exception {
public:
	ParametersOutOfRangeException() {
	}
	const char* what() const throw();
};

class CommandFactory;
class CommandLogger;
class ErrorHandler;
class Parameters;
class RandomSource;
class UndoRedoObserver;

/**
 * Thrown if @ref CommandFactory::Execute is called with a name for which no
 * command has been added, or if @ref CommandFactory::executeRandomCommands is
 * called on a @ref CommandFactory which has had no commands added, or if
 * @ref CommandFactory::executeRandomConstructiveCommands is called
 * on a @ref CommandFactory which has had no constructive commands set.
 */
class UnknownCommandException : public std::exception {
	const char* what() const throw();
};

class CommandNameAlreadyUsedException : public std::exception {
	const char* what() const throw();
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
	 * @param name The name with which the command's factory was registered.
	 */
	virtual void execute(const char* name, ...) = 0;
	/**
	 * Executes a command, logging it and putting its inverse onto the undo
	 * stack.
	 * @param name The name with which the command's factory was registered.
	 * @param params Supplier of the parameters to the command.
	 */
	virtual void execute(const char* name, Parameters& params) = 0;
	/**
	 * Executes the command described by (the first line of) @a line, a line
	 * from a command log previously written by a call to @ref execute.
	 * @param line Null- or line-ending-terminated string; a line from the log.
	 * @return true if a command was executed, false if the line was empty or
	 * contained only undo and redo instructions.
	 * @throws MalformedLineException if the line is neither empty nor starts
	 * with a command name.
	 */
	virtual bool executeFromLog(const char* line, ErrorHandler& e) = 0;
	/**
	 * Executes a random set of commands. Used for testing.
	 * @param rng The random number generator.
	 * @param min The minimum number of commands that should be generated.
	 * @param max The maximum number of commands that should be generated.
	 * @param [out] commandCount Returns the number of commands actually
	 * executed, even if an exception is thrown.
	 */
	virtual void executeRandomCommands(int& commandCount, RandomSource& rng,
			int min, int max) = 0;
	/**
	 * Executes a random set of commands. Used for testing for constructing
	 * fresh models from empty models. Only commands that were added with the
	 * @a constructive parameter of @ref addCommand set to @c true are used.
	 * @param rng The random number generator.
	 */
	virtual void executeRandomConstructiveCommands(RandomSource& rng) = 0;
	/**
	 * Make a command available for execution. It is assumed that @a name
	 * endures for the lifetime of the Executor. Ideally name should be a
	 * static constant, not heap allocated; for example
	 * @code{.cpp}
	 * executor.addCommand("addf", addFactory, true);
	 * @endcode
	 * @throws CommandNameAlreadyUsedException if a factory has already
	 * been registered under @a name.
	 * @param name The name of the command. Will need to be supplied in a
	 * call to @ref Execute in order to use @a factory.
	 * @param factory The factory that makes these sort of commands.
	 * @param constructive If true is passed, test code will use this
	 * factory to construct initial model states from empty models. Therefore
	 * @c true should be passed whenever the command is useful in constructing
	 * an initial state. For example, an add operation is constructive, but a
	 * delete or a move is not.
	 * @ref executeRandomConstructiveCommands uses only those factories
	 * passed here with @a constructive set to @c true.
	 */
	virtual void addCommand(const char* name,
			std::unique_ptr<CommandFactory>factory,
			bool constructive = false) = 0;
	/**
	 * Sets the logger to be used to record commands executed.
	 * @param logger The logger to be set. Ownership is not passed.
	 * Pass NULL to get no logging.
	 */
	virtual void setCommandLogger(CommandLogger* logger) = 0;
	/**
	 * Clears the undo and redo stacks.
	 */
	virtual void clearHistory() = 0;
	/**
	 * Undoes the most recent command.
	 * @return true on success, false if nothing was on the undo stack.
	 */
	virtual bool undo() = 0;
	/**
	 * Redoes the most recently undone command.
	 * @return true on success, false if nothing was on the redo stack.
	 */
	virtual bool redo() = 0;
	/**
	 * Returns true if and only if there is a command waiting to be undone.
	 */
	virtual bool canUndo() const = 0;
	/**
	 * Returns true if and only if there is a command waiting to be redone.
	 */
	virtual bool canRedo() const = 0;
	/**
	 * The number of commands registered.
	 * @par
	 * Useful for determining how many randomized tests to run.
	 */
	virtual int commandCount() const = 0;
	/**
	 * Sets the observer to be notified when {@ref canUndo} or {@ref canRedo}
	 * change what they would return.
	 * @param observer The new observer to be set. The old observer is unset.
	 * A null pointer means no observer. Ownership is not passed.
	 */
	virtual void setUndoRedoObserver(UndoRedoObserver* observer) = 0;
};

/**
 * Create a new executor.
 * @return The new {@ref Executor}; ownership is returned.
 */
Executor* makeExecutor();

#endif /* EXECUTOR_H_ */
