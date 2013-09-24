/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.              *
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

#ifndef UNDORENAMESOUND_H_
#define UNDORENAMESOUND_H_

#include "command.h"

class SceneVector;

class UndoRenameSound : public Command {
	SceneVector& sv;
	int32_t sc;
	int32_t fr;
	int32_t index;
	const char* name;
public:
	/**
	 * @param newName The new name. Ownership is passed; must have been
	 * allocated with {@c new char[]}.
	 */
	UndoRenameSound(SceneVector& model, int32_t scene, int32_t frame,
			int32_t soundNumber, const char* newName);
	~UndoRenameSound();
	/**
	 * Sets the name to be set.
	 * @param newName Ownership is not passed.
	 */
	void setName(const char* newName);
	Command* execute();
};

class UndoRenameSoundFactory : public CommandFactory {
	SceneVector& sv;
public:
	UndoRenameSoundFactory(SceneVector& model);
	~UndoRenameSoundFactory();
	Command* create(Parameters& ps);
};

#endif /* UNDORENAMESOUND_H_ */
