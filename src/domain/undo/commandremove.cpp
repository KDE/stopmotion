/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandremove.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "commandadd.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/undo/command.h"

class ErrorHandler;
class Frame;

CommandRemove::CommandRemove(AnimationImpl& model,
		int scene, int fromFrame, int count)
		: sv(model), sc(scene), fr(fromFrame), frameCount(count) {
}

CommandRemove::~CommandRemove() {
}

Command* CommandRemove::execute() {
	std::unique_ptr<CommandAdd> inv(new CommandAdd(sv, sc, fr, frameCount));
	std::vector<Frame*> removed;
	sv.removeFrames(sc, fr, frameCount, removed);
	for (Frame* f : removed) {
		inv->addFrame(f);
	}
	delete this;
	return inv.release();
}

CommandRemoveFactory::CommandRemoveFactory(AnimationImpl& model)
		: sv(model) {
}

CommandRemoveFactory::~CommandRemoveFactory() {
}

Command* CommandRemoveFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return nullptr;
	int32_t scene = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(scene);
	if (frameCount == 0)
		return nullptr;
	int32_t frame = ps.getInteger(0, frameCount - 1);
	int32_t count = ps.getHowMany();
	return new CommandRemove(sv, scene, frame, count);
}
