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

#ifndef COMMAND_FACTORY_H_
#define COMMAND_FACTORY_H_

class Command;
class Parameters;

/**
 * A command factory creates a particular type of command.
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

#endif
