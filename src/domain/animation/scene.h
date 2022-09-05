/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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
#ifndef SCENE_H
#define SCENE_H

#include "src/config.h"

#include <exception>
#include <vector>

class FileNameVisitor;
class Frame;
class Sound;
class WorkspaceFile;

class FrameOutOfRangeException : public std::exception {
public:
	FrameOutOfRangeException();
    const char* what() const throw();
};

/**
 * Class representing the scenes in the animation
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Scene {
public:
	/**
	 * Initializes the scene
	 */
	 Scene();

	/**
	 * Cleans up the scene and delete the frames.
	 */
	 ~Scene();

	/**
	 * Retrieves the size of the scene (the number of frames in it).
	 * @return the number of frames in the scene.
	 */
	int getSize() const;

	/**
	 * Retrieves the frame at position frameNumber in the scene.
	 * @param frameNumber the number of the frame to retrieve.
	 * @return the frame at position frameNumber.
	 */
	const Frame* getFrame(int frameNumber) const;

	/**
	 * Removes a frame from the scene.
	 * @param frame The frame to remove.
	 * @return The removed frame. Ownership is returned.
	 */
	Frame* removeFrame(int frame);

	/**
	 * Removes frames from the animation.
	 * @param frame The index from which to begin removing.
	 * @param count The number of frames to remove.
	 * @param [out] out The removed frames.
	 */
	void removeFrames(int frame, int count,
			std::vector<Frame*>& out);

	/**
	 * Moves the frames at the positions from fromFrame to toFrame (inclusive)
	 * to the position movePosition inside the scene.
	 * @param fromFrame the first frame to move.
	 * @param toFrame the lst frame to move.
	 * @param movePosition the position to move the frames to.
	 */
	void moveFrames(int fromFrame, int toFrame, int movePosition);

	/**
	 * Cleans frames from the scene without moving them around. Used when the user
	 * aborts while adding frames.
	 * @param fromFrame the first frame to remove.
	 * @param toFrame the last frame to remove.
	 */
	void cleanFrames(int fromFrame, int toFrame);

	/**
	 * Adds a frame at @a index. Will not fail if {@ref preallocateFrames}
	 * has been called with the appropriate number beforehand.
	 * @param f The frame to add.
	 * @param index The index at which to add the frame. Must be between
	 * 0 and @code{.cpp} getSize() @endcode inclusive.
	 */
	void addFrame(Frame* f, int index);

	/**
	 * Adds frames to the scene.
	 * @param where Frame index to add the new frames.
	 * @param fs The frames to add.
	 */
	void addFrames(int where, const std::vector<Frame*>& fs);

	/**
	 * Reserves space for @a count more frames to be added without the risk of
	 * an exception being thrown.
	 * @param count The number of frames that need to be added.
	 */
	void preallocateFrames(int count);

	/**
	 * Replaces the image of the frame at index {@a frameNumber}.
	 * @param frameNumber The index of the frame to alter.
	 * @param [in,out] The image to swap with. On exit, the frame at index
	 * {@a frameNumber} will have the image formerly held by
	 * {@a otherImage} and {@a otherImage} will have the image formerly held
	 * by the frame.
	 */
	void replaceImage(int frameNumber, WorkspaceFile& otherImage);

	/**
	 * Adds an already saved frame.
	 * @param f the frame to add
	 */
	void addSavedFrame(Frame *f);

	/**
	 * Adds the sound in the file filename to the end of the sounds in the
	 * frame with index {@a frameNumber}, giving it an arbitrary name.
	 * @param file The file that holds the sound.
	 * @return zero on success, less than zero on failure;
	 * -1 = file is not readable
	 * -2 = not a valid audio file
	 */
	int newSound(int frameNumber, WorkspaceFile& file);

	/**
	 * Adds a sound to the frame specified.
	 * @param frameNumber Index of the frame to add a sound to.
	 * @param soundNumber Index that the sound is to have.
	 * @param sound The sound to add.
	 */
	void addSound(int frameNumber, int soundNumber, Sound* sound);

	/**
	 * Removes a sound from the specified frame.
	 * @param frameNumber The frame from which to remove the sound.
	 * @param index Which sound to remove.
	 * @return The removed sound. Ownership is passed.
	 */
	Sound* removeSound(int frameNumber, int index);

	/**
	 * Gets a sound from a frame.
	 * @param frameNumber The index of the frame.
	 * @param index The index of the sound.
	 * @return The sound. Ownership is not passed.
	 */
	const Sound* getSound(int frameNumber, int index) const;

	/**
	 * Returns the number of sounds in the specified frame.
	 * @param frameNumber Index of the frame.
	 * @return the number of sounds in frame {@a frameNumber}.
	 */
	int soundCount(int frameNumber) const;

	/**
	 * Returns the total number of sounds in all the frames in this scene.
	 * @return the number of sounds.
	 */
	int soundCount() const;

	/**
	 * Sets the name of the sound at index soundNumber in the specified frame
	 * to soundName
	 * @param frameNumber Index of the frame.
	 * @param soundNumber the number of the sound to change the name of.
	 * @param soundName the new name of the sound. Ownership is passed; must
	 * have been allocated with new char[].
	 * @return The old name for this sound. Ownership is returned; must be
	 * freed with delete[].
	 */
	const char* setSoundName(int frameNumber, int soundNumber,
			const char* soundName);

	/**
	 * Retrieves the name of the sound at index soundNumber in the specified
	 * frame.
	 * @param frameNumber Index of the frame.
	 * @param soundNumber the sound to return.
	 * @return the sound at index soundNumber in this frame. Ownership is
	 * not returned.
	 */
	const char* getSoundName(int frameNumber, int soundNumber) const;

	/**
	 * Have v visit all the files referenced (images and sounds)
	 */
	void accept(FileNameVisitor& v) const;

private:
	typedef std::vector<Frame*> FrameVector;
	FrameVector frames;
};

#endif
