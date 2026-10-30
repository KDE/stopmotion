/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandsetimage.h"

#include <cstdint>
#include <string>

#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/workspacefile.h"
#include "src/domain/undo/command.h"

class ErrorHandler;

CommandSetImage::CommandSetImage(AnimationImpl& model, int32_t scene,
		int32_t frame, WorkspaceFile& w)
		: sv(model), sc(scene), fr(frame), image(w) {
}

CommandSetImage::~CommandSetImage() {
}

Command* CommandSetImage::execute() {
	sv.replaceImage(sc, fr, image);
	return this;
}

CommandSetImageFactory::CommandSetImageFactory(AnimationImpl& model) : sv(model) {
}

CommandSetImageFactory::~CommandSetImageFactory() {
}

Command* CommandSetImageFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return nullptr;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(sc);
	if (frameCount == 0)
		return nullptr;
	int32_t fr = ps.getInteger(0, frameCount - 1);
	std::string path;
	ps.getString(path, "?*.jpg");
	WorkspaceFile twf(path.c_str());
	return new CommandSetImage(sv, sc, fr, twf);
}
