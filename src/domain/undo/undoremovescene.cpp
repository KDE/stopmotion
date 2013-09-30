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

#include "undoremovescene.h"
#include "undoaddscene.h"
#include "src/domain/animation/scenevector.h"

#include <memory>

UndoRemoveScene::UndoRemoveScene(SceneVector& model, int32_t sceneNumber)
		: sv(model), sc(sceneNumber) {
}

UndoRemoveScene::~UndoRemoveScene() {
}

Command* UndoRemoveScene::execute() {
	std::auto_ptr<Command> inv(new UndoAddScene(sv,
			sc, sv.getScene(sc)));
	sv.removeScene(sc);
	return inv.release();
}

UndoRemoveSceneFactory::UndoRemoveSceneFactory(SceneVector& model)
		: sv(model) {
}

UndoRemoveSceneFactory::~UndoRemoveSceneFactory() {
}

Command* UndoRemoveSceneFactory::create(Parameters& ps) {
	int32_t sc = ps.getInteger(0, sv.sceneCount());
	return new UndoRemoveScene(sv, sc);
}
