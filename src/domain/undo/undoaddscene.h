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
#ifndef UNDOADDSCENE_H
#define UNDOADDSCENE_H

#include "command.h"

class SceneVector;

class UndoAddScene : public Command {
	SceneVector& sv;
	int32_t index;
	Scene* sc;
public:
	/**
	 * @param scene Ownership is passed.
	 */
	UndoAddScene(SceneVector& model, int32_t sceneNumber, Scene* scene);
	~UndoAddScene();
	Command* execute();
};

/**
 * This factory can only create empty scenes.
 */
class UndoAddSceneFactory : public CommandFactory {
	SceneVector& sv;
public:
	UndoAddSceneFactory(SceneVector& model);
	~UndoAddSceneFactory();
	Command* create(Parameters& ps);
};

#endif
