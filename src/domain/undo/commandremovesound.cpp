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

#include "commandremovesound.h"

#include <memory>

#include "commandaddsound.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/undo/command.h"

class Sound;

CommandRemoveSound::CommandRemoveSound(AnimationImpl& model, int32_t scene,
		int32_t frame, int32_t soundNumber)
	: sv(model), sc(scene), fr(frame), index(soundNumber) {
}

CommandRemoveSound::~CommandRemoveSound() {
}

Command* CommandRemoveSound::execute() {
	std::unique_ptr<CommandAddSound> inv(
			new CommandAddSound(sv, sc, fr, index));
	Sound* s = sv.removeSound(sc, fr, index);
	inv->setSound(s);
	delete this;
	return inv.release();
}

UndoRemoveSoundFactory::UndoRemoveSoundFactory(AnimationImpl& model)
	: sv(model) {
}

UndoRemoveSoundFactory::~UndoRemoveSoundFactory() {
}

Command* UndoRemoveSoundFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return 0;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(sc);
	if (frameCount == 0)
		return 0;
	int32_t fr = ps.getInteger(0, frameCount - 1);
	int soundCount = sv.getScene(sc)->getFrame(fr)->soundCount();
	if (soundCount == 0)
		return 0;
	int32_t index = ps.getInteger(0, soundCount - 1);
	return new CommandRemoveSound(sv, sc, fr, index);
}
