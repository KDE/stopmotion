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
#ifndef OBSERVER_H
#define OBSERVER_H

#include "src/config.h"
#include "src/domain/animation/frame.h"
#include "src/presentation/frontends/frontend.h"

#include <vector>

using namespace std;

class FrameIterator {
public:
	virtual ~FrameIterator() = 0;
	/**
	 * Returns the number of frames remaining in the iteration.
	 */
	virtual int count() const = 0;
	/**
	 * Returns true if and only if we have advanced beyond the limits of this
	 * iteration. Neither {@ref get} nor {@ref next} should be called if
	 * {@c isAtEnd} has returned true.
	 */
	virtual bool isAtEnd() const = 0;
	/**
	 * Returns the current frame's name
	 * @return The name; ownership is not returned.
	 */
	virtual const char* getName() = 0;
	/**
	 * Advances the iteration on to the next frame.
	 */
	virtual void next() = 0;
};

/**
 * The observer interface. All classes who wants to be notified when something
 * changes in the animationmodel has to implement from this class.
 *
 * The observers implemented with this class also has to be attatched to the
 * animationmodel.
 *
 * The observer is implemented with strong use of the push model. Although this
 * decreases the flexibility we gain alot in efficiency which is more
 * important to us.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Observer
{
public:
	virtual ~Observer() {}

	/**
	 * Abstract function for receiving notification about new frames added to the model.
	 *
	 * @param frames the frames which has been added to the model.
	 * @param index the place the frame has been added
	 * @param frontend the GUI frontend which is used to displaying progress on timeconsuming operations
	 */
	virtual void updateAdd(FrameIterator& frames, unsigned int index,
			Frontend *frontend) = 0;

	/**
	 * Abstract function for receiving notification about frames removed from the model.
	 * @param fromFrame the first frame of those removed
	 * @param toFrame the last frame of those removed
	 */
	virtual void updateRemove(unsigned int fromFrame, unsigned int toFrame) = 0;

	/**
	 * Abstract function for receiving notification when frames are moved in the model.
	 * @param fromFrame index of the first selected frame
	 * @param toFrame index of the last selected frame
	 * @param movePosition index to where the selection should be move to
	 */
	virtual void updateMove(unsigned int fromFrame, unsigned int toFrame, unsigned int movePosition) = 0;

	/**
	 * Abstract function for receiving notification when a new frame are selected.
	 * @param frameNumber the new active frame.
	 */
	virtual void updateNewActiveFrame(int frameNumber) = 0;

	/**
	 * Abstract function for receiving notification when the model is erased.
	 */
	virtual void updateClear() = 0;

	/**
	 * Abstract function for receiving notification when a frame is to be played.
	 * @param frameNumber the frame to be played
	 */
	virtual void updatePlayFrame(int frameNumber) = 0;

	/**
	 * Abstract function for receiving notification when a new scene is created
	 * at location index.
	 * @param index the location where the new scene is created.
	 */
	virtual void updateNewScene(int index) = 0;

	/**
	 * Abstract function for receiving notification when a scene is removed from
	 * the model.
	 * @param sceneNumber the scene which has been removed from the model.
	 */
	virtual void updateRemoveScene(int sceneNumber) = 0;

	/**
	 * Abstract function for receiving notification when a scene in the animation
	 * has been moved.
	 * @param sceneNumber the scene which have been moved.
	 * @param movePosition the position the scene has been moved to.
	 */
	virtual void updateMoveScene(int sceneNumber, int movePosition) = 0;

	/**
	 * Abstract function for receiving notification when a new scene is set as the
	 * current "active" scene.
	 * @param sceneNumber the new active scene.
	 * @param framePaths an iterator returning the paths to the frames.
	 * @param frontend the frontend for processing events while adding frames.
	 */
	virtual void updateNewActiveScene(int sceneNumber, FrameIterator& framePaths,
		Frontend *frontend) = 0;

	/**
	 * Abstract function for receiving notification when the disk representation of the
	 * animation is changed by other programs.
	 * @param frameNumber the index of the frame which has been changed. (active scene
	 * is assumed).
	 */
	virtual void updateAnimationChanged(int activeScene, int frameNumber) = 0;
};

#endif
