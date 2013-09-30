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

#ifndef COMMANDMOVESCENE_H
#define COMMANDMOVESCENE_H

#include "command.h"

class SceneVector;

class CommandMoveScene : public Command {
	SceneVector& sv;
	int32_t from;
	int32_t to;
public:
	CommandMoveScene(SceneVector& model, int sceneNumber, int movePosition);
	~CommandMoveScene();
	Command* execute();
};

class CommandMoveSceneFactory : public CommandFactory {
	SceneVector& sv;
public:
	CommandMoveSceneFactory(SceneVector& model);
	~CommandMoveSceneFactory();
	Command* create(Parameters& ps);
};

#endif
