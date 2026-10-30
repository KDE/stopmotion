/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandremovescene.h"

#include "command.h"
#include "commandaddscene.h"
#include "src/domain/animation/animationimpl.h"

#include <memory>

class ErrorHandler;

UndoRemoveScene::UndoRemoveScene(AnimationImpl& model, int32_t sceneNumber)
		: sv(model), sc(sceneNumber) {
}

UndoRemoveScene::~UndoRemoveScene() {
}

Command* UndoRemoveScene::execute() {
	std::unique_ptr<CommandAddScene> inv(new CommandAddScene(sv, sc));
	inv->setScene(sv.removeScene(sc));
	delete this;
	return inv.release();
}

UndoRemoveSceneFactory::UndoRemoveSceneFactory(AnimationImpl& model)
		: sv(model) {
}

UndoRemoveSceneFactory::~UndoRemoveSceneFactory() {
}

Command* UndoRemoveSceneFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return nullptr;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	return new UndoRemoveScene(sv, sc);
}
