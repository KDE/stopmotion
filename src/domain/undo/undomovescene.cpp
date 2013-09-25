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

#include "undomovescene.h"

UndoMoveScene::UndoMoveScene(SceneVector& model, int sceneNumber,
		int movePosition) : sv(model), from(sceneNumber), to(movePosition) {
}

UndoMoveScene::~UndoMoveScene() {
}

Command* UndoMoveScene::execute() {
	sv.moveScene(from, to);
	int32_t t = from;
	from = to;
	to = t;
	return this;
}

UndoMoveSceneFactory::UndoMoveSceneFactory(SceneVector& model) : sv(model) {
}

UndoMoveSceneFactory::~UndoMoveSceneFactory() {
}

Command* UndoMoveSceneFactory::create(Parameters& ps) {
	int max = sv.sceneCount() - 1;
	int32_t from = ps.getInteger(0, max);
	int32_t to = ps.getInteger(0, max);
	return new UndoMoveScene(sv, from, to);
}
