/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandrenamesound.h"

#include <string>

#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/scene.h"
#include "src/domain/undo/command.h"


CommandRenameSound::CommandRenameSound(AnimationImpl& model, int32_t scene,
		int32_t frame, int32_t soundNumber, std::string newName)
	: sv(model), sc(scene), fr(frame), index(soundNumber),
	  name(std::move(newName)) {
}

CommandRenameSound::~CommandRenameSound() {
}

Command* CommandRenameSound::execute() {
	// the inverse command sets the old name again
	name = sv.setSoundName(sc, fr, index, std::move(name));
	return this;
}

CommandRenameSoundFactory::CommandRenameSoundFactory(AnimationImpl& model) :sv(model) {
}

CommandRenameSoundFactory::~CommandRenameSoundFactory() {
}

Command* CommandRenameSoundFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return nullptr;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(sc);
	if (frameCount == 0)
		return nullptr;
	int32_t fr = ps.getInteger(0, frameCount - 1);
	int soundCount = sv.getScene(sc)->getFrame(fr)->soundCount();
	if (soundCount == 0)
		return nullptr;
	int32_t index = ps.getInteger(0, soundCount - 1);
	std::string name;
	ps.getString(name, "sound ?*");
	return new CommandRenameSound(sv, sc, fr, index, std::move(name));
}
