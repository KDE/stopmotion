/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDMOVESCENE_H
#define COMMANDMOVESCENE_H

#include "command.h"

#include <cstdint>

class AnimationImpl;
class ErrorHandler;

class CommandMoveScene : public Command {
	AnimationImpl& sv;
	int32_t from;
	int32_t to;
public:
	CommandMoveScene(AnimationImpl& model, int sceneNumber, int movePosition);
	~CommandMoveScene() override;
	Command* execute() override;
};

class CommandMoveSceneFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandMoveSceneFactory(AnimationImpl& model);
	~CommandMoveSceneFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
