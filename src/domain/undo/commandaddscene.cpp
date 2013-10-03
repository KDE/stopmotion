/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
#include "commandremovescene.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/scenevector.h"

#include <memory>

CommandAddScene::CommandAddScene(SceneVector& model, int32_t sn)
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
	std::auto_ptr<UndoRemoveScene> inv(new UndoRemoveScene(sv, index));
	sv.addScene(index, sc);
	return inv.release();
}

CommandAddSceneFactory::CommandAddSceneFactory(SceneVector& model) : sv(model) {
}

CommandAddSceneFactory::~CommandAddSceneFactory() {
}

Command* CommandAddSceneFactory::create(Parameters& ps) {
	int32_t index = ps.getInteger(0, sv.sceneCount());
	std::auto_ptr<Scene> sc(new Scene());
	CommandAddScene* r = new CommandAddScene(sv, index);
	r->setScene(sc.release());
	return r;
}
