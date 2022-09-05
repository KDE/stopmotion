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

#include "commandaddsound.h"

#include <assert.h>
#include <memory>
#include <string>

#include "commandremovesound.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/sound.h"
#include "src/domain/animation/workspacefile.h"
#include "src/domain/filenamevisitor.h"
#include "src/domain/undo/command.h"

class ErrorHandler;

CommandAddSound::CommandAddSound(AnimationImpl& model, int32_t scene, int32_t frame,
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
	std::unique_ptr<CommandRemoveSound>
			inv(new CommandRemoveSound(sv, sc, fr, index));
	sv.addSound(sc, fr, index, snd);
	snd = 0;
	delete this;
	return inv.release();
};

void CommandAddSound::accept(FileNameVisitor& v) const {
	v.visitSound(snd->getSoundPath());
}

CommandAddSoundFactory::CommandAddSoundFactory(AnimationImpl& model) : sv(model) {
}

CommandAddSoundFactory::~CommandAddSoundFactory() {
}

Command* CommandAddSoundFactory::create(Parameters& ps, ErrorHandler& e) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return 0;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(sc);
	if (frameCount == 0)
		return 0;
	int32_t fr = ps.getInteger(0, frameCount - 1);
	int32_t index = ps.getInteger(0, sv.soundCount(sc, fr));
	std::string filename;
	ps.getString(filename, "?*.test-sound");
	std::string humanName;
	ps.getString(humanName, "sound ?*");
	std::unique_ptr<Sound> sound(new Sound());
	sound->setName(humanName);
	std::unique_ptr<CommandAddSound> r(new CommandAddSound(sv, sc, fr, index));
	Sound* soundCopy = sound.get();
	r->setSound(sound.release());
	WorkspaceFile wf(filename.c_str());
	soundCopy->open(wf, e);
	return r.release();
}
