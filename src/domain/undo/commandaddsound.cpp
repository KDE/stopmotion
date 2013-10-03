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

#include "commandaddsound.h"
#include "commandremovesound.h"
#include "scenevector.h"
#include "src/domain/animation/frame.h"
#include "src/domain/filenamevisitor.h"

#include <memory>

CommandAddSound::CommandAddSound(SceneVector& model, int32_t scene, int32_t frame,
		int32_t soundNumber)
	: sv(model), sc(scene), fr(frame), index(soundNumber),
	  snd(0) {
}

CommandAddSound::~CommandAddSound() {
	delete snd;
}

void CommandAddSound::setSound(Sound* sound) {
	assert(!snd);
	snd = sound;
}

Command* CommandAddSound::execute() {
	std::auto_ptr<CommandRemoveSound>
			inv(new CommandRemoveSound(sv, sc, fr, index));
	sv.addSound(sc, fr, index, snd);
	delete this;
	return inv.release();
};

void CommandAddSound::accept(FileNameVisitor& v) const {
	v.visitSound(snd->getAudio()->getSoundPath());
}

CommandAddSoundFactory::CommandAddSoundFactory(SceneVector& model) : sv(model) {
}

CommandAddSoundFactory::~CommandAddSoundFactory() {
}

Command* CommandAddSoundFactory::create(Parameters& ps) {
	int32_t sc = ps.getInteger(0, sv.sceneCount() - 1);
	int32_t fr = ps.getInteger(0, sv.frameCount(sc));
	int32_t index = ps.getInteger(0, sv.soundCount(sc, fr));
	std::string filename;
	ps.getString(filename);
	std::string humanName;
	ps.getString(humanName);
	std::auto_ptr<Sound> sound(new Sound());
	sound->setName(humanName);
	std::auto_ptr<CommandAddSound> r(new CommandAddSound(sv, sc, fr, index));
	r->setSound(sound.release());
	TemporaryWorkspaceFile twf(filename.c_str());
	sound->open(twf);
	return r.release();
}
