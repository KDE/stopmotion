/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <memory>
#include <string>

class AudioFormat;
class AudioDriver;
class WorkspaceFile;
class ErrorHandler;

class Sound {
	std::unique_ptr<AudioFormat> af;
	std::string name;
	Sound& operator=(const Sound&) = delete;
	Sound(const Sound&) = delete;
public:
	Sound();
	~Sound();
	/**
	 * Opens an audio file. See {@ref AudioFormat::setFilename} for
	 * exceptions that might be thrown.
	 * @param file The filename to open. Ownership is not passed.
	 * @todo We need a way of mocking this for testing.
	 */
	void open(WorkspaceFile& file, ErrorHandler& e);
	/**
	 * Sets the (human-readable) name of this sound.
	 * @return The old name.
	 */
	std::string setName(std::string name);
	const char* getName() const;
	const char* getSoundPath() const;
	const char* getBasename() const;
	void addToDriver(AudioDriver& ad) const;
};

#endif
