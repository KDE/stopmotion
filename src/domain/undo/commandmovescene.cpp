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

#include "commandmovescene.h"

#include "command.h"
#include "src/domain/animation/animationimpl.h"

class ErrorHandler;

CommandMoveScene::CommandMoveScene(AnimationImpl& model, int sceneNumber,
		int movePosition) : sv(model), from(sceneNumber), to(movePosition) {
}

CommandMoveScene::~CommandMoveScene() {
}

Command* CommandMoveScene::execute() {
	sv.moveScene(from, to);
	int32_t t = from;
	if (from < to) {
		from = to - 1;
		to = t;
	} else {
		from = to;
		to = t + 1;
	}
	return this;
}

CommandMoveSceneFactory::CommandMoveSceneFactory(AnimationImpl& model) : sv(model) {
}

CommandMoveSceneFactory::~CommandMoveSceneFactory() {
}

Command* CommandMoveSceneFactory::create(Parameters& ps, ErrorHandler&) {
	int max = sv.sceneCount() - 1;
	if (max < 0)
		return 0;
	int32_t from = ps.getInteger(0, max);
	int32_t to = ps.getInteger(0, max + 1);
	return new CommandMoveScene(sv, from, to);
}
