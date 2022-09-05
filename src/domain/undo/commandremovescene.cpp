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

#include "commandremovescene.h"

#include "command.h"
#include "commandaddscene.h"
#include "src/domain/animation/animationimpl.h"

#include <memory>

class ErrorHandler;

UndoRemoveScene::UndoRemoveScene(AnimationImpl& model, int32_t sceneNumber)
		: sv(model), sc(sceneNumber) {
}

UndoRemoveScene::~UndoRemoveScene() {
}

Command* UndoRemoveScene::execute() {
	std::unique_ptr<CommandAddScene> inv(new CommandAddScene(sv, sc));
	inv->setScene(sv.removeScene(sc));
	delete this;
	return inv.release();
}

UndoRemoveSceneFactory::UndoRemoveSceneFactory(AnimationImpl& model)
		: sv(model) {
}

UndoRemoveSceneFactory::~UndoRemoveSceneFactory() {
}

Command* UndoRemoveSceneFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return 0;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	return new UndoRemoveScene(sv, sc);
}
