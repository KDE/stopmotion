/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDREMOVE_H
#define COMMANDREMOVE_H

#include "command.h"

class AnimationImpl;
class ErrorHandler;

class CommandRemove : public Command {
	AnimationImpl& sv;
	int sc;
	int fr;
	int frameCount;
public:
	/**
	 * Constructs a command that removes frames from a scene.
	 * @param model The animation to be changed.
	 * @param scene The scene from which the frames were removed.
	 * @param fromFrame The index of the first frame to be removed.
	 * @param frameCount The number of frames to remove.
	 */
	CommandRemove(AnimationImpl& model,
			int scene, int fromFrame, int frameCount);
	~CommandRemove() override;
	Command* execute() override;
};

class CommandRemoveFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandRemoveFactory(AnimationImpl& model);
	~CommandRemoveFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
