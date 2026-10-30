/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandmovescene.h"

#include "command.h"
#include "src/domain/animation/animationimpl.h"

class ErrorHandler;

CommandMoveScene::CommandMoveScene(AnimationImpl& model, int sceneNumber,
		int movePosition) : sv(model), from(sceneNumber), to(movePosition) {
}

CommandMoveScene::~CommandMoveScene() {
}

Command* CommandMoveScene::execute() {
	sv.moveScene(from, to);
	int32_t t = from;
	if (from < to) {
		from = to - 1;
		to = t;
	} else {
		from = to;
		to = t + 1;
	}
	return this;
}

CommandMoveSceneFactory::CommandMoveSceneFactory(AnimationImpl& model) : sv(model) {
}

CommandMoveSceneFactory::~CommandMoveSceneFactory() {
}

Command* CommandMoveSceneFactory::create(Parameters& ps, ErrorHandler&) {
	int max = sv.sceneCount() - 1;
	if (max < 0)
		return nullptr;
	int32_t from = ps.getInteger(0, max);
	int32_t to = ps.getInteger(0, max + 1);
	return new CommandMoveScene(sv, from, to);
}
