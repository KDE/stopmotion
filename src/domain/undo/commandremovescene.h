/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDREMOVESCENE_H
#define COMMANDREMOVESCENE_H

#include "command.h"

#include <cstdint>

class AnimationImpl;
class ErrorHandler;

class UndoRemoveScene : public Command {
	AnimationImpl& sv;
	int32_t sc;
public:
	UndoRemoveScene(AnimationImpl& model, int32_t sceneNumber);
	~UndoRemoveScene() override;
	Command* execute() override;
};

class UndoRemoveSceneFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	UndoRemoveSceneFactory(AnimationImpl& model);
	~UndoRemoveSceneFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
