/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.                   *
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
	 * For receiving the serialized command. This function is allowed to
	 * throw an exception.
	 * @par
	 * @ref CommandComplete needs to be called later, when the command has been
	 * successfully executed.
	 * @param text The text of the command to be written to the log. Should
	 * not contain any nulls or line delimiters.
	 */
	virtual void writeCommand(const char* text) = 0;
	/**
	 * Indicates that the command has been successfully executed.
	 */
	virtual void commandComplete() = 0;
};

#endif /* COMMANDLOGGER_H_ */
