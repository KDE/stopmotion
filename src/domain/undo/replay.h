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

#ifndef REPLAY_H_
#define REPLAY_H_

#include <stdint.h>

/**
 * Exception thrown by CommandFactory if a method that hasn't been overridden
 * is called. This represents an attempt to make a command with incorrect
 * parameters.
 */
class CommandFactoryIncorrectParametersException {
};

/**
 * Exception thrown by MakeCommand if the parsing of the command failed.
 */
class CommandFactoryParseFailureException {
};

/**
 * Exception thrown by MakeCommand if the parsing of the command failed.
 */
class CommandFactoryNoSuchCommandException {
};

/**
 * Thrown if a command factory attempts to read a parameter from the log files
 * that is of the wrong type, is not present or otherwise does not parse.
 */
class IncorrectParameterException {
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
	virtual int32_t GetString(char* out, int32_t maxLength) = 0;
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
	 * void ExecuteAddFrame(Executor& e, const char* filename, int32_t sceneNo,
	 *     int32_t frameNo) {
	 *   e.Execute("addf", filename, sceneNo, frameNo);
	 * }
	 */
	virtual void Execute(const char* name, ...) = 0;
	/**
	 * Make a command available for execution. Ownership of 'factory' is
	 * passed. It is assumed that 'name' endures for the lifetime of the
	 * Executor (ideally name should be a static constant, not heap
	 * allocated; for example executor.AddCommand("addf", addFactory);).
	 * Throws CommandNameAlreadyUsedException if a factory has already
	 * been registered under 'name'.
	 */
	virtual void AddCommand(const char* name, CommandFactory* factory) = 0;
};

/**
 * General factory for commands. Either constructs a command from a line of
 * text describing the command and its parameters, or produces such a line of
 * text.
 * The specific commands are produced by CommandFactorys registers with the
 * CommandReplayer with RegisterCommandFactory().
 */
class CommandReplayer {
	CommandReplayerImpl* pImpl;
	CommandAndDescriptionFactory* describer;
public:
	/**
	 * Constructs a new command replayer.
	 */
	CommandReplayer();
	~CommandReplayer();
	/**
	 * Set the logger for the CommandAndDescriptionFactories returned by
	 * GetCommandFactory. Ownership is not passed.
	 */
	void SetLogger(CommandLogger* logger);
	/**
	 * Registers the command factory with a command replayer.
	 */
	void RegisterCommandFactory(const char* name,
			CommandFactory& factory);
	/**
	 * Makes a command from a string.
	 */
	Command& MakeCommand(const char*);
	/**
	 * Gets a command factory that produces the command specified. This command
	 * factory should not be used after any subsequent call to GetCommandFactory()
	 * with the same CommandReplayer.
	 * @param commandName The name of the command that will be constructed.
	 * @return Command factory that constructs items of the specified type, as
	 * well as logging the construction to a CommandAndDescriptionFactory::Logger.
	 * Returns 0 if there is no command factory matching the name given.
	 */
	const CommandFactory* GetCommandFactory(
			const char* commandName);
};

#endif /* REPLAY_H_ */
