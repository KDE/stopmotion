/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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

#include <vector>
#include <string>
using namespace std;

class FileNameVisitor;

/**
 * Class representing the frames in the animation
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Frame
{
public:
	class Sound {
		AudioFormat* af;
		const char* name;
	public:
		Sound();
		~Sound();
		/**
		 * Opens an audio file.
		 * @param filename The filename to open. Ownership is not passed.
		 * @return 0 for success, less than zero for failure. See
		 * @ref AudioFormat::setFilename for error return value meanings.
		 */
		int open(const char* filename);
		/**
		 * Sets or resets the (human-readable) name of this sound.
		 * @param name The new name or NULL for no name. Ownership is passed.
		 * @return The old name or NULL for no name. Ownership is returned.
		 */
		const char* setName(const char* name);
		AudioFormat* getAudio();
		const char* getName() const;
	};
	/** Number of files in the temporary directory. */
	static unsigned int tmpNum;
	
	/** Number of files in the trash directory. */
	static unsigned int trashNum;
	
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
	int addSound(const char *filename);

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
	 * Returns the number of sounds in this frame.
	 * @return the number of sounds in this frame.
	 */
	unsigned int getNumberOfSounds();
	
	/**
	 * Returns the sounds belonging to this frame.
	 * @return a vector containing audio objects which can be played
	 * with the implemented audio driver
	 */
	vector<AudioFormat*>& getSounds();
	
	/**
	 * Sets the name of the sound at index soundNumber in this frame to 
	 * soundName
	 * @param soundNumber the number of the sound to change the name of.
	 * @param soundName the new name of the sound.
	 */
	void setSoundName(unsigned int soundNumber, const char* soundName);
	
	/**
	 * Retrieves the name of the sound at index soundNumber in this frame.
	 * @param soundNumber the sound to return.
	 * @return the sound at index soundNumber in this frame.
	 */
	const char* getSoundName(unsigned int soundNumber);

	/**
	 * Retrieves the absolute path to the picture of this frame.
	 * @return the absolute path to the picture of this frame.
	 */
	char* getImagePath();
	 
	/**
	 * Moves sounds and images belonging to this frame into project directories.
	 * @param imageDir the image directory to move images into
	 * @param soundDir the sound directory to move sounds into
	 * @param imgNum a number describing the position of this frame relative
	 * to the other frames. E.g. 000005 if this frame is number five in the sequence
	 * of frames.
	 */
	void moveToProjectDir(const char *imageDir, const char *soundDir, unsigned int imgNum);
	 
	/**
	 * Copies the files belonging to this frame to a temporary directory.
	 */
	void copyToTemp();
	 
	/**
	 * Sets this frame as a valid project file.
	 */
	void markAsProjectFile();
	
	/**
	 * Plays the sounds belonging to this frame.
	 */
	void playSounds(AudioDriver *driver);
	
	/**
	 * Checks if this frame is in the project directory (as opposed to the
	 * temporary directory).
	 * @return true if a project frame, false otherwise
	 */
	bool isProjectFrame();

	/**
	 * Makes v visit all the files referenced (image and sounds)
	 */
	void Accept(FileNameVisitor& v) const;

private:
	/** Absolute path to a temporary directory (~/.stopmotion/tmp). */
	static char tempPath[256];
	
	/** Absolute path to a trash directory (~/.stopmotion/trash). */
	static char trashPath[256];
	
	/** Absolute path to the image file. The image can either be in a project 
	 * directory, the tmp directory or the trash directory. */
	char *imagePath; // absolute path

	typedef vector<Sound*> soundVector;

	/** Contains the sounds belonging to this frame. */
	soundVector sounds;
	
	/** True if this frame is saved to a project file. It is also true if the
	 * frame is loaded from a previously saved project. */
	bool isProjectFile;
	
	/**
	 * Moves the sounds to a sound directory.
	 * @param directory the directory to move the sounds to
	 */
	void moveToSoundDir(const char *directory);
	
	/**
	 * Moves the image to an image directory.
	 * @param directory the directory where the project files are stored
	 * @param imgNum the number of the image which is used to set a filename
	 */
	void moveToImageDir(const char *directory, unsigned int imgNum);
	
	/**
	 * Gets the id of an image. This is just the filename of the image without
	 * extension.
	 * @return an id for the image
	 */
	char* getImageId();
};

#endif
