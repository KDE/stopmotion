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

class Command;

/**
 * Produces one sort of command.
 * @author Tim Band
 */
class CommandFactory {
	CommandFactory(const CommandFactory&); // no copy
	CommandFactory& operator=(const CommandFactory&); // no copy
public:
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
	 * Make a command from an integer and a string
	 */
	virtual Command& Make(int32_t, const char*) const;
};

/**
 * Command factory that makes a specific sort of command and, simultaneously,
 * makes the string that would produce the same thing when passed to
 * MakeCommand().
 */
class CommandAndDescriptionFactory : public CommandFactory {
public:
	/**
	 * Callback type for receiving the command's string
	 */
	typedef void SetName_t(const char*);
	CommandAndDescriptionFactory();
	~CommandAndDescriptionFactory();
	virtual void SetNameCallback(SetName_t* fn) = 0;
};

/**
 * General factory for commands. Either constructs a command from a line of
 * text describing the command and its parameters, or produces such a line of
 * text.
 * The specific commands are produced by CommandFactorys registers with the
 * CommandReplayer with RegisterCommandFactory().
 * @author Tim Band
 */
class CommandReplayer;

/**
 * Contructs a new command replayer.
 * @author Tim Band
 */
CommandReplayer* MakeCommandReplayer();

/**
 * Registers the command factory with a command replayer.
 * @author Tim Band
 */
void RegisterCommandFactory(CommandReplayer& replayer,
		const char* name, CommandFactory& factory);

/**
 * Makes a command from a string.
 * @author Tim Band
 */
Command& MakeCommand(CommandReplayer&, const char*);

/**
 * Gets a command factory that produces the command specified. This command
 * factory should not be used after any subsequent call to GetCommandFactory()
 * with the same CommandReplayer.
 * @param commandName The name of the command that will be constructed.
 * @author Tim Band
 */
const CommandAndDescriptionFactory& GetCommandFactory(CommandReplayer&,
		const char* commandName);

#endif /* REPLAY_H_ */
