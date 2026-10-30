/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandaddscene.h"

#include <cassert>
#include <memory>

#include "commandremovescene.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/undo/command.h"

class ErrorHandler;

CommandAddScene::CommandAddScene(AnimationImpl& model, int32_t sn)
		: sv(model), index(sn), sc(nullptr) {
}

CommandAddScene::~CommandAddScene() {
	delete sc;
}

void CommandAddScene::setScene(Scene* s) {
	assert(!sc);
	sc = s;
}

Command* CommandAddScene::execute() {
	std::unique_ptr<UndoRemoveScene> inv(new UndoRemoveScene(sv, index));
	sv.addScene(index, sc);
	sc = nullptr;
	delete this;
	return inv.release();
}

CommandAddSceneFactory::CommandAddSceneFactory(AnimationImpl& model) : sv(model) {
}

CommandAddSceneFactory::~CommandAddSceneFactory() {
}

Command* CommandAddSceneFactory::create(Parameters& ps, ErrorHandler&) {
	int32_t index = ps.getInteger(0, sv.sceneCount());
	std::unique_ptr<Scene> sc(new Scene());
	CommandAddScene* r = new CommandAddScene(sv, index);
	r->setScene(sc.release());
	return r;
}
