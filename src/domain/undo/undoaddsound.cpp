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

#include "undoaddsound.h"
#include "undoremovesound.h"
#include "src/domain/animation/frame.h"

	SceneVector sv;
	int32_t sc;
	int32_t fr;
	int32_t index;
	Frame::Sound* snd;
UndoAddSound::UndoAddSound(SceneVector& model, int32_t scene, int32_t frame,
		int32_t soundNumber, Frame::Sound* sound)
	: sv(model), sc(scene), fr(frame), index(soundNumber),
	  snd(sound) {
}

UndoAddSound::~UndoAddSound() {
	delete snd;
}

Command* UndoAddSound::execute() {
	std::auto_ptr inv(new UndoRemoveSound(sc, fr, index));
	sv.addSound(sc, fr, index, snd);
	delete this;
	return inv.release();
};

void UndoAddSound::accept(FileNameVisitor& v) const {
	v.visitSound(snd->getAudio()->getSoundPath());
}

UndoAddSoundFactory::UndoAddSoundFactory(SceneVector& model) : sv(model) {
}

UndoAddSoundFactory::~UndoAddSoundFactory() {
}

Command* UndoAddSoundFactory::create(Parameters& ps) {
	int32_t sc = ps.getInteger(0, sv.sceneCount() - 1);
	int32_t fr = ps.getInteger(0, sv.frameCount(sc));
	int32_t index = ps.getInteger(0, sv.soundCount(sc, fr));
	std::string filename;
	ps.getString(filename);
	std::string humanName;
	ps.getString(humanName);
	std::auto_ptr<Frame::Sound> sound = new Frame::Sound();
	sound->setName(humanName);
	auto_ptr<Command> r(new UndoAddSound(sv, sc, fr, index, sound));
	TemporaryWorkspaceFile twf(filename);
	sound->open(twf);
	return r.release();
}
