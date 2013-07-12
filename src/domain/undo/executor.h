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

#endif /* EXECUTOR_H_ */
