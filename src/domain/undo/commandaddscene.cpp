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

#include "commandaddscene.h"

#include <assert.h>
#include <memory>

#include "commandremovescene.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/undo/command.h"

class ErrorHandler;

CommandAddScene::CommandAddScene(AnimationImpl& model, int32_t sn)
		: sv(model), index(sn), sc(0) {
}

CommandAddScene::~CommandAddScene() {
	delete sc;
}

void CommandAddScene::setScene(Scene* s) {
	assert(!sc);
	sc = s;
}

Command* CommandAddScene::execute() {
	std::unique_ptr<UndoRemoveScene> inv(new UndoRemoveScene(sv, index));
	sv.addScene(index, sc);
	sc = 0;
	delete this;
	return inv.release();
}

CommandAddSceneFactory::CommandAddSceneFactory(AnimationImpl& model) : sv(model) {
}

CommandAddSceneFactory::~CommandAddSceneFactory() {
}

Command* CommandAddSceneFactory::create(Parameters& ps, ErrorHandler&) {
	int32_t index = ps.getInteger(0, sv.sceneCount());
	std::unique_ptr<Scene> sc(new Scene());
	CommandAddScene* r = new CommandAddScene(sv, index);
	r->setScene(sc.release());
	return r;
}
