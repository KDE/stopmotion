/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandaddsound.h"

#include <cassert>
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
	: sv(model), sc(scene), fr(frame), index(soundNumber) {
}

CommandAddSound::~CommandAddSound() {
}

void CommandAddSound::setSound(std::unique_ptr<Sound> sound) {
	assert(!snd);
	snd = std::move(sound);
}

Command* CommandAddSound::execute() {
	std::unique_ptr<CommandRemoveSound>
			inv(new CommandRemoveSound(sv, sc, fr, index));
	sv.addSound(sc, fr, index, std::move(snd));
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
		return nullptr;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(sc);
	if (frameCount == 0)
		return nullptr;
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
	r->setSound(std::move(sound));
	WorkspaceFile wf(filename.c_str());
	soundCopy->open(wf, e);
	return r.release();
}
