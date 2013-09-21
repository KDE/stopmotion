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
#include "undoremove.h"
#include "undoadd.h"

UndoRemove::UndoRemove(SceneVector& model,
		int scene, int fromFrame, int count)
		: sv(model), sc(scene), fr(fromFrame), frameCount(count) {
}

UndoRemove::~UndoRemove() {
}

Command* UndoRemove::execute() {
	std::auto_ptr<UndoAdd> inv(new UndoAdd(sv, sc, fr, frameCount));
	std::vector<Frame*> removed;
	sv.removeFrames(sc, fr, frameCount, removed);
	for (std::vector<Frame*>::iterator i = removed.begin();
			i != removed.end(); ++i) {
		inv->addFrame(*i);
	}
	return inv.release();
}

UndoRemoveFactory::UndoRemoveFactory(SceneVector& model)
		: sv(model) {
}

UndoRemoveFactory::~UndoRemoveFactory() {
}

Command* UndoRemoveFactory::create(Parameters& ps) {
	int32_t scene = ps.getInteger(0, sv.sceneCount() - 1);
	int32_t frame = ps.getInteger(0, sv.frameCount(scene) - 1);
	int32_t count = ps.getHowMany();
	return new UndoRemove(sv, scene, frame, count);
}
