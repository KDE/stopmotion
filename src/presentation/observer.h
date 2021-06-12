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

/**
 * The observer interface. Any object wanting to be notified when something
 * changes in the animationmodel has to implement from this class.
 *
 * The observers implemented with this class also has to be attached to the
 * animationmodel.
 *
 * The observer is implemented with strong use of the push model. Although this
 * decreases the flexibility we gain a lot in efficiency which is more
 * important to us.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Observer {
public:
	virtual ~Observer() {}

	/**
	 * Abstract function for receiving notification about new frames added to the model.
	 *
	 * @param frames the frames which has been added to the model.
	 * @param scene The scene to which the frames have been added.
	 * @param index The frame number within the scene to which the frames have
	 * been added.
	 * @param frontend the GUI frontend which is used to displaying progress on timeconsuming operations
	 */
	virtual void updateAdd(int scene, int index, int numFrames) = 0;

	/**
	 * Abstract function for receiving notification about frames removed from the model.
	 * @param fromFrame the first frame of those removed
	 * @param toFrame the last frame of those removed
	 */
	virtual void updateRemove(int scene, int fromFrame, int toFrame) = 0;

	/**
	 * Abstract function for receiving notification when frames are moved in the model.
	 * @param fromScene The scene that the frames were moved from.
	 * @param fromFrame Index of the first selected frame within {@a fromScene}
	 * @param count The number of frames moved.
	 * @param toScene The scene to which the frames were moved.
	 * @param toFrame The position within scene {@a toScene} that the frames
	 * were moved to.
	 */
	virtual void updateMove(int fromScene, int fromFrame, int count,
			int toScene, int toFrame) = 0;

	/**
	 * Abstract function for receiving notification when the model is erased.
	 */
	virtual void updateClear() = 0;

	/**
	 * Abstract function for receiving notification when a new scene is
	 * created at location index.
	 * @param index The location at which the new scene is created.
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
	 * @param sceneNumber the scene that has been moved.
	 * @param movePosition the position the scene has been moved to.
	 */
	virtual void updateMoveScene(int sceneNumber, int movePosition) = 0;

	/**
	 * Abstract function for receiving notification when the disk representation of the
	 * animation is changed by other programs.
	 * @param sceneNumber The scene to which the changed frame belongs.
	 * @param frameNumber The index of the frame that has been changed.
	 */
	virtual void updateAnimationChanged(int activeScene, int frameNumber) = 0;

	/**
	 * Abstract function for receiving notifications when the number, order or
	 * names of the sounds attached to a frame change.
	 * @param sceneNumber The scene to which the frame belongs.
	 * @param frameNumber The frame within scene number @a sceneNumber to which
	 * the changed sounds belong.
	 */
	virtual void updateSoundChanged(int sceneNumber, int frameNumber) = 0;

	/**
	 * Resynchronizes the observer with the state of the animation. This is
	 * called whenever the animation is called with out-of-range arguments.
	 */
	virtual void resync() = 0;
};

#endif
