/***************************************************************************
 *   Copyright (C) 2013-2017 by Linuxstopmotion contributors;              *
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


#ifndef COMMANDLOGGER_H_
#define COMMANDLOGGER_H_

/**
 * For receiving the command's string representation during calls to Make
 * methods.
 */
class CommandLogger {
protected:
	virtual ~CommandLogger() = 0;
public:
	/**
	 * Records text representing the command about to be executed.
	 * If any command, undo or redo has previously been recorded but not
	 * committed, it is discarded.
	 * @par
	 * This function is allowed to throw an exception. The written command is
	 * not to be committed to the log until the @ref commit method is called.
	 * @par
	 * @ref commit needs to be called later, when the command has been
	 * successfully executed.
	 * @param text The text of the command to be written to the log. Should
	 * not contain any nulls or line delimiters.
	 */
	virtual void writePendingCommand(const char* text) = 0;
	/**
	 * Records that an undo is about to happen. Any previous uncommitted
	 * command or undo or redo is discarded. The undo is not committed until
	 * @ref commit is called later.
	 */
	virtual void writePendingUndo() = 0;
	/**
	 * Records that a redo is about to happen. Any previous uncommitted
	 * command or undo or redo is discarded. The redo is not committed until
	 * @ref commit is called later.
	 */
	virtual void writePendingRedo() = 0;
	/**
	 * Indicates that the command, undo or redo has been successfully executed.
	 */
	virtual void commit() = 0;
	/**
	 * Writes committed commands to the file. This will only have an effect if
	 * a previous call to commit() threw an exception.
	 */
	virtual void flush() = 0;
};

#endif /* COMMANDLOGGER_H_ */
