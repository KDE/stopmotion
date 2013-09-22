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
#ifndef SCENE_H
#define SCENE_H

#include "src/config.h"
#include "frame.h"
#include "src/presentation/frontends/frontend.h"

class FileNameVisitor;

/**
 * Class representing the scenes in the animation
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Scene
{
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
	 *Temporary functions to assist in an orderly transfer of functionality from
	 *Animation, without having to break the project for big time-periods.
	 */
	vector<Frame*>& getFrames();

	/**
	 * Retrieves the size of the scene (the number of frames in it).
	 * @return the number of frames in the scene.
	 */
	unsigned int getSize();

	/**
	 * Retrieves the frame at position frameNumber in the scene.
	 * @param frameNumber the number of the frame to retrieve.
	 * @return the frame at position frameNumber.
	 */
	Frame* getFrame(unsigned int frameNumber);

	/**
	 * Removes a frame from the scene.
	 * @param frame The frame to remove.
	 * @return The removed frame. Ownership is returned.
	 */
	Frame* removeFrame(unsigned int frame);

	/**
	 * Moves the frames at the positions from fromFrame to toFrame (inclusive)
	 * to the position movePosition inside the scene.
	 * @param fromFrame the first frame to move.
	 * @param toFrame the lst frame to move.
	 * @param movePosition the position to move the frames to.
	 */
	void moveFrames(unsigned int fromFrame, unsigned int toFrame,
			unsigned int movePosition);

	/**
	 * Cleans frames from the scene without moving them around. Used when the user
	 * aborts while adding frames.
	 * @param fromFrame the first frame to remove.
	 * @param toFrame the last frame to remove.
	 */
	void cleanFrames(unsigned int fromFrame, unsigned int toFrame);

	/**
	 * Adds a frame at @c index. Will not fail if {@ref preallocateFrames}
	 * has been called with the appropriate number beforehand.
	 * @param f The frame to add.
	 * @param index The index at which to add the frame. Must be between
	 * 0 and @code{.cpp} getSize() @endcode inclusive.
	 */
	void addFrame(Frame* f, int index);

	/**
	 * Reserves space for @c count more frames to be added without the risk of
	 * an exception being thrown.
	 * @param count The number of frames that need to be added.
	 */
	void preallocateFrames(int count);

	/**
	 * Adds an already saved frame.
	 * @param f the frame to add
	 */
	void addSavedFrame(Frame *f);

	/**
	 * Adds a sound located at the path ``sound'' to the frame at location frameNumber
	 * in the scene.
	 * @param frameNumber the frame to add the sound to.
	 * @param sound the path to the sound file containing the sound.
	 * @return zero on success, less than zero on failure;
	 * -1 = file is not readable
	 * -2 = not a valid audio file
	 */
	int addSound(unsigned int frameNumber, const char *sound);

	/**
	 * Removes the the sound with the number soundNumber from the frame at location
	 * frameNumber.
	 * @param frameNumber the frame to remove the sound from.
	 * @param soundNumber the index of the sound to remove from the frame at index
	 * frameNumber.
	 */
	void removeSound(unsigned int frameNumber, unsigned int soundNumber);

	/**
	 * Sets the name of the sound at index soundNumber in the frame at location
	 * frameNumber to soundName.
	 * @param frameNumber the frame containing the sound to change the name of.
	 * @param soundNumber the sound to change the name of.
	 * @param soundName the new name for the sound.
	 */
	void setSoundName(unsigned int frameNumber, unsigned int soundNumber,
			const char* soundName);

	/**
	 * Has v visit all the files referenced (images and sounds)
	 */
	void Accept(FileNameVisitor& v) const;

private:
	typedef vector<Frame*> frameVector;
	frameVector frames;
};

#endif
