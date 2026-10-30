/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef COMMANDADD_H
#define COMMANDADD_H

#include "command.h"

#include <cstdint>
#include <string>
#include <vector>

class ErrorHandler;
class FileNameVisitor;
class Frame;
class AnimationImpl;
class TemporaryWorkspaceFile;

class CommandAdd : public Command {
public:
	/**
	 * @param count The number of frames to reserve; {@ref addFrame} can
	 * subsequently be called this many times without throwing an exception.
	 */
	CommandAdd(AnimationImpl& model, int toScene, int toFrame, int count);
	~CommandAdd() override;
	/**
	 * Adds a frame to the add command.
	 * @param frame Ownership is passed.
	 */
	void addFrame(Frame* frame);
	Command* execute() override;
	void accept(FileNameVisitor& v) const override;
private:
	AnimationImpl& sv;
	std::vector<Frame*> frames;
	int scene;
	int frame;
};

class CommandAddFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandAddFactory(AnimationImpl& model);
	~CommandAddFactory() override;
	Command* create(::Parameters& ps, ErrorHandler& e) override;
	class Parameters : public ::Parameters {
		int32_t sc;
		int32_t fr;
		int32_t frameCount;
		TemporaryWorkspaceFile* twfs;
		int32_t twfCount;
		int32_t parameterCount;
	public:
		Parameters(int scene, int frame, int count);
		~Parameters() override;
		/**
		 * Add a frame.
		 * @filename The filename of the image. Ownership is not passed.
		 * @return The full path of the workspace file created.
		 */
		const char* addFrame(const char* filename);
		int32_t getInteger(int32_t min, int32_t max) override;
		int32_t getHowMany() override;
		void getString(std::string& out, const char* pattern) override;
		/**
		 * If this function is not called before destruction, copied files
		 * held by this function are deleted.
		 */
		void retainFiles();
	};
};

#endif
