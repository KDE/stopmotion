/*   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
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

#ifndef REPLAY_H_
#define REPLAY_H_

#include <stdint.h>

/**
 * Exception thrown by CommandFactory if a method that hasn't been overridden
 * is called. This represents an attempt to make a command with incorrect
 * parameters.
 * @author Tim Band
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

class Command;

/**
 * Produces one sort of command.
 * @author Tim Band
 */
class CommandFactory {
	CommandFactory(const CommandFactory&); // no copy
	CommandFactory& operator=(const CommandFactory&); // no copy
public:
	CommandFactory() {
	}
	virtual ~CommandFactory() = 0;
	/**
	 * Make a command with no parameters.
	 */
	virtual Command& Make() const;
	/**
	 * Make a command from a single integer
	 */
	virtual Command& Make(int32_t) const;
	/**
	 * Make a command from two integers
	 */
	virtual Command& Make(int32_t, int32_t) const;
	/**
	 * Make a command from three integers
	 */
	virtual Command& Make(int32_t, int32_t, int32_t) const;
	/**
	 * Make a command from four integers
	 */
	virtual Command& Make(int32_t, int32_t, int32_t, int32_t) const;
	/**
	 * Make a command from five integers
	 */
	virtual Command& Make(int32_t, int32_t, int32_t, int32_t, int32_t) const;
	/**
	 * Make a command from a string
	 */
	virtual Command& Make(const char*) const;
	/**
	 * Make a command from an integer and a string
	 */
	virtual Command& Make(int32_t, const char*) const;
	/**
	 * Make a command from two integers and a string
	 */
	virtual Command& Make(int32_t, int32_t, const char*) const;
	/**
	 * Make a command from three integers and a string
	 */
	virtual Command& Make(int32_t, int32_t, int32_t, const char*) const;
};

class CommandReplayerImpl;
class CommandAndDescriptionFactory;

/**
 * General factory for commands. Either constructs a command from a line of
 * text describing the command and its parameters, or produces such a line of
 * text.
 * The specific commands are produced by CommandFactorys registers with the
 * CommandReplayer with RegisterCommandFactory().
 * @author Tim Band
 */
class CommandReplayer {
	CommandReplayerImpl* pImpl;
	CommandAndDescriptionFactory* describer;
public:
	/**
	 * For receiving the command's string representation during calls to Make
	 * methods.
	 */
	class Logger {
	protected:
		virtual ~Logger() = 0;
	public:
		/**
		 * For receiving the serialized command. This function is allowed to
		 * throw an exception.
		 */
		virtual void WriteCommand(const char*) = 0;
	};
	/**
	 * Constructs a new command replayer.
	 */
	CommandReplayer();
	~CommandReplayer();
	/**
	 * Set the logger for the CommandAndDescriptionFactories returned by
	 * GetCommandFactory. Ownership is not passed.
	 */
	void SetLogger(Logger* logger);
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
