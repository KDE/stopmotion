/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.                   *
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
#ifndef UNDOREMOVE_H
#define UNDOREMOVE_H

#include "command.h"

class UndoRemove : public Command {
	SceneVector& sv;
	int sc;
	int fr;
	int frameCount;
public:
	/**
	 * Constructs a command that removes frames from a scene.
	 * @param model The animation to be changed.
	 * @param scene The scene from which the frames were removed.
	 * @param fromFrame The index of the first frame to be removed.
	 * @param frameCount The number of frames to remove.
	 */
	UndoRemove(SceneVector& model,
			int scene, int fromFrame, int frameCount);
	~UndoRemove();
	Command* execute();
};

class UndoRemoveFactory : public CommandFactory {
	SceneVector& sv;
public:
	UndoRemoveFactory(SceneVector& model);
	~UndoRemoveFactory();
	Command* create(Parameters& ps);
};

#endif
