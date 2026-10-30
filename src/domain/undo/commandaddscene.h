/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef COMMANDADDSCENE_H
#define COMMANDADDSCENE_H

#include "command.h"

#include <cstdint>

class AnimationImpl;
class ErrorHandler;
class Scene;

class CommandAddScene : public Command {
	AnimationImpl& sv;
	int32_t index;
	Scene* sc;
public:
	/**
	 * @param scene Ownership is passed.
	 */
	CommandAddScene(AnimationImpl& model, int32_t sceneNumber);
	~CommandAddScene() override;
	void setScene(Scene* s);
	Command* execute() override;
};

/**
 * This factory can only create empty scenes.
 */
class CommandAddSceneFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandAddSceneFactory(AnimationImpl& model);
	~CommandAddSceneFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
