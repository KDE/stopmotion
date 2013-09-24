/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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

#include "undoremovesound.h"
#include "undoaddsound.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/scenevector.h"
#include <memory>

UndoRemoveSound::UndoRemoveSound(SceneVector& model, int32_t scene,
		int32_t frame, int32_t soundNumber)
	: sv(model), sc(scene), fr(frame), index(soundNumber) {
}

UndoRemoveSound::~UndoRemoveSound() {
}

Command* UndoRemoveSound::execute() {
	Frame* frame = sv.getScene(sc)->getFrame(fr);
	Frame::Sound* sound = frame->getSound(index);
	std::auto_ptr<Command*> inv(
			new UndoAddSound(sv, sc, fr, index, sound));
	frame->removeSound(index);
	delete this;
	return inv.release();
}

UndoRemoveSoundFactory::UndoRemoveSoundFactory(SceneVector& model)
	: sv(model) {
}

UndoRemoveSoundFactory::~UndoRemoveSoundFactory() {
}

Command* UndoRemoveSoundFactory::create(Parameters& ps) {
	int32_t sc = ps.getInteger(0, sv.sceneCount());
	int32_t fr = ps.getInteger(0, sv.frameCount(sc));
	int32_t index = ps.getInteger(0, sv.getScene(sc)->getFrame(fr)->
			getNumberOfSounds());
	return new UndoRemoveSound(sv, sc, fr, index);
}
