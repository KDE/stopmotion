/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <string>

class AudioFormat;
class AudioDriver;
class WorkspaceFile;
class ErrorHandler;

class Sound {
	AudioFormat* af;
	const char* name;
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
	 * Sets or resets the (human-readable) name of this sound.
	 * @param name The new name or NULL for no name. Name passed must have been
	 * allocated with new[]. Ownership is passed.
	 * @return The old name or NULL for no name. Ownership is returned.
	 */
	const char* setName(const char* name);
	/**
	 * Sets the (human-readable) name of this sound. May only be used when
	 * there is no name already set for the sound.
	 * @param n The name to set.
	 */
	void setName(std::string& n);
	const char* getName() const;
	const char* getSoundPath() const;
	const char* getBasename() const;
	void addToDriver(AudioDriver& ad) const;
};

#endif
