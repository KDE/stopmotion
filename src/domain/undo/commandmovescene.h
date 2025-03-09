/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#ifndef COMMANDMOVESCENE_H
#define COMMANDMOVESCENE_H

#include "command.h"

#include <stdint.h>

class AnimationImpl;
class ErrorHandler;

class CommandMoveScene : public Command {
	AnimationImpl& sv;
	int32_t from;
	int32_t to;
public:
	CommandMoveScene(AnimationImpl& model, int sceneNumber, int movePosition);
	~CommandMoveScene() override;
	Command* execute() override;
};

class CommandMoveSceneFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandMoveSceneFactory(AnimationImpl& model);
	~CommandMoveSceneFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
