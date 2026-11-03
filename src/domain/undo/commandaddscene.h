/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef COMMANDADDSCENE_H
#define COMMANDADDSCENE_H

#include "command.h"

#include <cstdint>
#include <memory>

class AnimationImpl;
class ErrorHandler;
class Scene;

class CommandAddScene : public Command {
	AnimationImpl& sv;
	int32_t index;
	std::unique_ptr<Scene> sc;
public:
	CommandAddScene(AnimationImpl& model, int32_t sceneNumber);
	~CommandAddScene() override;
	void setScene(std::unique_ptr<Scene> s);
	Command* execute() override;
};

/**
 * This factory can only create empty scenes.
 */
class CommandAddSceneFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	explicit CommandAddSceneFactory(AnimationImpl& model);
	~CommandAddSceneFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
