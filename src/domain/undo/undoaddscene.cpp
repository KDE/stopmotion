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

#include "undoaddscene.h"
#include "undoremovescene.h"
#include "src/domain/animation/scene.h"

UndoAddScene::UndoAddScene(SceneVector& model, int32_t sn, Scene* scene)
		: sv(model), index(sn), sc(scene) {
}

UndoAddScene::~UndoAddScene() {
	delete sc;
}

Command* UndoAddScene::execute() {
	std::auto_ptr<UndoRemoveScene> inv(new UndoRemoveScene(index));
	sv.addScene(index, sc);
	return inv.release();
}

UndoAddSceneFactory::UndoAddSceneFactory(SceneVector& model) : sv(model) {
}

UndoAddSceneFactory::~UndoAddSceneFactory() {
}

Command* UndoAddSceneFactory::create(Parameters& ps) {
	int32_t index = ps.getInteger(0, sv.sceneCount());
	std::auto_ptr<Scene> sc(new Scene());
	Command* r = new UndoAddScene(sv, index, sc);
	sc.release();
	return r;
}
