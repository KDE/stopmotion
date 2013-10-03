/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FRAME_H
#define FRAME_H

#include "src/config.h"
#include "src/technical/audio/audioformat.h"
#include "src/technical/audio/audiodriver.h"
#include "workspacefile.h"

#include <vector>
#include <string>
using namespace std;

class FileNameVisitor;

class Sound {
	AudioFormat* af;
	const char* name;
public:
	Sound();
	~Sound();
	/**
	 * Opens an audio file. See {@ref AudioFormat::setFilename} for
	 * exceptions that might be thrown.
	 * @param filename The filename to open. Ownership is not passed.
	 * @todo We need a way of mocking this for testing.
	 */
	void open(TemporaryWorkspaceFile& filename);
	/**
	 * Sets or resets the (human-readable) name of this sound.
	 * @param name The new name or NULL for no name. Ownership is passed.
	 * @return The old name or NULL for no name. Ownership is returned.
	 */
	const char* setName(const char* name);
	/**
	 * Sets the (human-readable) name of this sound. May only be used when
	 * there is no name already set for the sound.
	 * @param n The name to set.
	 */
	void setName(std::string& n);
	AudioFormat* getAudio();
	const char* getName() const;
};

/**
 * Class representing the frames in the animation
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Frame {
public:
	/**
	 * Creates a frame with the specified file for its picture.
	 * @param file The picture for this frame.
	 */
	Frame(TemporaryWorkspaceFile& file);

	/**
	 * Cleans up after the frame
	 */
	~Frame();

	/**
	 * Adds the sound in the file filename to this frame.
	 * @param filename the name of the file where the sound is.
	 * @return zero on success, less than zero on failure;
	 * -1 = file is not readable
	 * -2 = not a valid audio file
	 */
	int addSound(TemporaryWorkspaceFile& filename);

	/**
	 * Adds a sound.
	 * @param sound Ownership is passed. May not be null.
	 * @param index Must be between 0 and @code{.cpp} getNumberOfSounds() - 1
	 * @endcode
	 * @note This is guaranteed not to fail for @c n calls after a call to
	 * @code{.cpp} preallocateSounds(n) @endcode
	 */
	void addSound(Sound* sound, int index);

	/**
	 * Allocates space for @c extra more calls to @ref addSound
	 * @param extra Number of slots to reserve.
	 */
	void preallocateSounds(int extra);

	/**
	 * Removes sound number soundNumber from this frame.
	 * @param soundNumber The index of the sound to remove. Must be between
	 * 0 and @code{.cpp} getNumberOfSounds() - 1 @encode
	 * @return The sound that was removed. Ownership is returned.
	 */
	Sound* removeSound(int index);

	/**
	 * Returns the sound.
	 * @param index Which sound to return.
	 */
	Sound* getSound(int index);

	/**
	 * Returns the sound.
	 * @param index Which sound to return.
	 */
	const Sound* getSound(int index) const;

	/**
	 * Returns the number of sounds in this frame.
	 * @return the number of sounds in this frame.
	 */
	int getNumberOfSounds() const;

	/**
	 * Sets the name of the sound at index soundNumber in this frame to
	 * soundName
	 * @param soundNumber the number of the sound to change the name of.
	 * @param soundName the new name of the sound. Ownership is passed; must
	 * have been allocated with new char[].
	 * @return The old name for this sound. Ownership is returned; must be
	 * freed with delete[].
	 */
	const char* setSoundName(unsigned int soundNumber, const char* soundName);

	/**
	 * Retrieves the name of the sound at index soundNumber in this frame.
	 * @param soundNumber the sound to return.
	 * @return the sound at index soundNumber in this frame. Ownership is
	 * not returned.
	 */
	const char* getSoundName(unsigned int soundNumber) const;

	/**
	 * Retrieves the absolute path to the picture of this frame.
	 * @return the absolute path to the picture of this frame.
	 */
	const char* getImagePath() const;

	/**
	 * Retrieves the base name of the picture of this frame.
	 * @return The picture file's silename and extension without any path.
	 */
	const char* getBasename() const;

	/**
	 * Replaces the image path.
	 * @param [in, out] otherImage The new image to set. On return, this will
	 * hold the old image.
	 */
	void replaceImage(WorkspaceFile& otherImage);

	/**
	 * Plays the sounds belonging to this frame.
	 */
	void playSounds(AudioDriver *driver) const;

	/**
	 * Makes v visit all the files referenced (image and sounds)
	 */
	void accept(FileNameVisitor& v) const;

private:

	WorkspaceFile imagePath;

	typedef vector<Sound*> SoundVector;

	/** Contains the sounds belonging to this frame. */
	SoundVector sounds;
};

#endif
