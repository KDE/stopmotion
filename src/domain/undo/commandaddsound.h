/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDADDSOUND_H_
#define COMMANDADDSOUND_H_

#include "command.h"

#include <cstdint>

class AnimationImpl;
class ErrorHandler;
class FileNameVisitor;
class Sound;

class CommandAddSound : public Command {
	AnimationImpl& sv;
	int32_t sc;
	int32_t fr;
	int32_t index;
	Sound* snd;
public:
	CommandAddSound(AnimationImpl& model, int32_t scene, int32_t frame,
			int32_t soundNumber);
	~CommandAddSound() override;
	void setSound(Sound* sound);
	Command* execute() override;
	void accept(FileNameVisitor& v) const override;
};

class CommandAddSoundFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandAddSoundFactory(AnimationImpl& model);
	~CommandAddSoundFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
