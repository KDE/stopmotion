/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDRENAMESOUND_H_
#define COMMANDRENAMESOUND_H_

#include <cstdint>
#include <string>

#include "command.h"

class AnimationImpl;
class ErrorHandler;

class CommandRenameSound : public Command {
	AnimationImpl& sv;
	int32_t sc;
	int32_t fr;
	int32_t index;
	std::string name;
public:
	CommandRenameSound(AnimationImpl& model, int32_t scene, int32_t frame,
			int32_t soundNumber, std::string newName);
	~CommandRenameSound() override;
	Command* execute() override;
};

class CommandRenameSoundFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	explicit CommandRenameSoundFactory(AnimationImpl& model);
	~CommandRenameSoundFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
