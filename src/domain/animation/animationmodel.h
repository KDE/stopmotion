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
#ifndef ANIMATIONMODEL_H
#define ANIMATIONMODEL_H

#include "src/config.h"
#include "src/presentation/observer.h"
#include "src/presentation/frontends/frontend.h"
#include "src/technical/video/videoencoder.h"
#include "frame.h"

#include <cstring>
#include <vector>

using namespace std;


/**
 *The animation model abstract class. All implementations of the animation
 *model have to inherit from this class.
 *
 *AnimationModel serves as the Subject in our implementation of the observer
 *pattern, and takes care of notifying the observers when a change occur.
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class AnimationModel
{
public:
	AnimationModel();
	virtual ~AnimationModel();

	/**
	 * Attatches a new observer to the model. The observer will be notified when
	 * something is changed in the model.
	 * @param o the observer to be attatched to the model.
	 */
	void attatch(Observer *o);

	/**
	 * Detatches an observer from the model. The observer will no longer be notified
	 * when something is changed in the model.
	 * @param o the observer to be detatched from the model.
	 */
	void detatch(Observer *o);

	/**
	 * Registers the GUI frontend which is used to displaying and updating
	 * progress when running timeconsuming operations.
	 * @param frontend the GUI frontend
	 */
	void registerFrontend(Frontend *frontend);

	/**
	 * Retrieves the registered frontend.
	 * @return the frontend if it is a valid frontend pointer, NULL otherwise.
	 */
	Frontend* getFrontend();

	/**
	 * Notify the observers that a frame is added to the model.
	 * @param frames a vector containing the frames that are added to the model.
	 * @param index the position it is added to.
	 */
	void notifyAdd(const vector<char*>& frames, unsigned int index);

	/**
	 * Notify the observers that a selection of frames are removed from the model.
	 * @param fromFrame the first frame in the selection.
	 * @param toFrame the last frame in the selection.
	 */
	void notifyRemove(unsigned int fromFrame, unsigned int toFrame);

	/**
	 * Notify the observers that a frame in the model has changed location (moved).
	 * @param fromFrame the first frame in the selection
	 * @param toFrame the last frame in the selection
	 * @param movePosition the index to move the selected frames
	 */
	void notifyMove(unsigned int fromFrame, unsigned int toFrame, unsigned int movePosition);

	/**
	 * Notify the observers the active frame in the model has changed.
	 * @param frameNumber the number of the new active frame.
	 */
	void notifyNewActiveFrame(int frameNumber);

	/**
	 * Notify the observers that the model has been cleared.
	 */
	void notifyClear();

	/**
	 * Notify the observers that a frame is to be played.
	 * @param frameNumber the frame to be played.
	 */
	void notifyPlayFrame(int frameNumber);

	/**
	 * Notify the observers that a new scene has been added to the animation.
	 * @param index the place the new scene should be added to.
	 */
	void notifyNewScene(int index);

	/**
	 * Notify the observers that a scene has been removed from the animation.
	 * @param sceneNumber the frame which has been removed from the animation.
	 */
	void notifyRemoveScene(int sceneNumber);

	/**
	 * Notify the observers that a scene in the animation has been moved.
	 * @param sceneNumber the scene which has been moved.
	 * @param movePosition the location the scene has been moved to.
	 */
	void notifyMoveScene(int sceneNumber, int movePosition);

	/**
	 * Notify the observers that a new scene is set as the active scene.
	 * @param sceneNumber the number of the new active scene.
	 * @param framePaths the paths to the frames in the new scene.
	 * @param frontend the frontend for process handling.
	 */
	void notifyNewActiveScene(int sceneNumber, FrameIterator& framePaths,
			Frontend *frontend);

	/**
	 * Notify the observers that the disk representation of the animation has
	 * been altered.
	 * @param frameNumber the index of the frame which has been changed.
	 */
	void notifyAnimationChanged(int frameNumber);

	/**
	 * Abstract function which adds a new frame to the AnimationModel.
	 * @param frameNames a vector containing the frames to be added to the model
	 * @param index the place to add the frames in the animation.
	 * @return a vector containing paths to the images which has been
	 * copied to a temporary directory.
	 */
	virtual void addFrames(const vector<const char*>& frameNames,
			int32_t index) = 0;

	/**
	 * Abstract function to remove the frames between (inclusive) fromFrame and
	 * toFrame from the animation.
	 * @param fromFrame the first frame to remove.
	 * @param toFrame the last frame to remove.
	 * @return a vector containing paths to the images in the given range,
	 * which are moved to a trash directory.
	 */
	virtual void removeFrames(int32_t fromFrame, int32_t toFrame) = 0;

	/**
	 * Abstract function for moving the frames from fromFrame (inclusive) to
	 * toFrame to toPosition.
	 * @param fromFrame the first frame to move.
	 * @param toFrame the last frame to move.
	 * @param movePosition the posititon to move the frames to.
	 */
	virtual void moveFrames(int32_t fromFrame, int32_t toFrame,
			int32_t movePosition) = 0;

	/**
	 * Abstract function for adding the sound from the file "sound" to frame
	 * at position frameNumber.
	 * @param frameNumber the number of the frame to add the sound to
	 * @param filename the path to the file with the sound
	 * @return zero on success, less than zero on failure;
	 * -1 = file is not readable
	 * -2 = not a valid audio file
	 */
	virtual int addSound(int32_t frameNumber, const char *filename) = 0;

	/**
	 * Abstract function which removes the sound with index soundNumber from
	 * the frame with index frameNumber.
	 * @param frameNumber the index of the frame to remove a sound from.
	 * @param soundNumber the index of the sound to remove from the frame.
	 */
	virtual void removeSound(int32_t frameNumber, int32_t soundNumber) = 0;

	/**
	 * Abstract function whick sets the name of the sound with index soundNumber
	 * in the frame with index frameNumber to soundName.
	 * @param frameNumber the index of the frame the sound is in.
	 * @param soundNumber the index to the sound to change the name of.
	 * @param soundName the new name of the sound.
	 */
	virtual void setSoundName(int32_t frameNumber, int32_t soundNumber,
			const char* soundName) = 0;

	/**
	 * Abstract function which retrieves a given frame
	 *
	 * @param frameNumber the number of the frame to retrieve
	 * @param sceneNumber the scene to retrieve the frame from.
	 * @return the frame to retrieve.
	 */
	virtual Frame* getFrame(int frameNumber, int sceneNumber) = 0;

	/**
	 * Abstract overloaded function for convenience. Assumes the scene to retrieve there
	 * frame from is the active scene.
	 * @param frameNumber the number of the frame to retrieve.
	 * @return the frame at location frameNumber in the active scene.
	 */
	virtual Frame* getFrame(int frameNumber) = 0;

	/**
	 * Abstract function which returns the size of the model.
	 * @return the size of the model.
	 */
	virtual unsigned int getModelSize() = 0;

	/**
	 * Abstract function for retrieving number of frames in a given scene.
	 * @param sceneNumber the scene number
	 * @return number of frames
	 */
	virtual unsigned int getSceneSize(int sceneNumber) = 0;

	/**
	 * Abstract function for retrieving number of scenes.
	 * @return number of scenes
	 */
	virtual unsigned int getNumberOfScenes() = 0;

	/**
	 * Abstract function for changing the currently active frame.
	 * @param frameNumber the number of the new active frame.
	 */
	virtual void setActiveFrame(int frameNumber) = 0;

	/**
	 * Abstract function for playing the frame with number frameNumber.
	 * @param frameNumber the number of the frame to play.
	 */
	virtual void playFrame(int frameNumber) = 0;

	/**
	 *  Abstract function which returns the number of the currently active frame
	 * in the model.
	 * @return the number of the currently active frame.
	 */
	virtual int getActiveFrameNumber() = 0;

	/**
	 * Clears the undo history.
	 */
	virtual void clearHistory() = 0;

	/**
	 * Abstract function for clearing the model.
	 */
	virtual void clear() = 0;

	/**
	 * Undoes the last operation.
	 */
	virtual void undo() = 0;

	/**
	 * Redoes the last undone operation.
	 */
	virtual void redo() = 0;

	/**
	 * Abstract function for retrieving the project file
	 * @return the project file if it's setted, NULL otherwise.
	 */
	virtual const char* getProjectFile() = 0;

	/**
	 * Abstract function for retrieving the project path.
	 * @return the project path if it's setted, NULL otherwise.
	 */
        virtual const char* getProjectPath() = 0;

	/**
	 * Abstract function for opening a project.
	 * @return true on success, false otherwise
	 */
	virtual bool openProject(const char *filename) = 0;

	/**
	 * Abstract function which saves the active project to a XML-file
	 * which is written to disk.
	 * @return true on success, false otherwise
	 */
	virtual bool saveProject(const char *filename) = 0;

	/**
	 * Abstract funtion to create a new project.
	 * @return true on success, false otherwise
	 */
	virtual bool newProject() = 0;

	/**
	 * Abstract function to check if there are unsaved changes.
	 * @return true if there are unsaved changes, false otherwise
	 */
	virtual bool isUnsavedChanges() = 0;

	/**
	 * Abstract funtion for setting new active scene.
	 * @param sceneNumber the scene to set active
	 */
	virtual void setActiveScene(int sceneNumber) = 0;

	/**
	 * Abstract function for retrieving the number of the currently active scene.
	 * @return the number of the active scene.
	 */
	virtual int getActiveSceneNumber() = 0;

	/**
	 * Abstract function for adding a new scene at a given index in the vector
	 * containing the scenes.
	 * @param index the index to add the scene
	 */
	virtual void newScene(int index) = 0;

	/**
	 * Abstract function which removes the scene at the location sceneNumber
	 * from the animation.
	 * @param sceneNumber the scene to be removed from the animation.
	 */
	virtual void removeScene(int sceneNumber) = 0;

	/**
	 * Abstract function which moves the scene at position sceneNumber
	 * to the position movePosition.
	 * @param sceneNumber the number of the scene to move.
	 * @param movePosition the position to move the scene to.
	 */
	virtual void moveScene(int sceneNumber, int movePosition) = 0;

	/**
	 * Abstract function for initializing the audio device
	 * so it is ready to play sounds.
	 * @return true on success, false otherwise
	 */
	virtual bool initAudioDevice() = 0;

	/**
	 * Abstract function for shutting down the audio device so other
	 * programs can use it.
	 */
	virtual void shutdownAudioDevice() = 0;

	/**
	 * Abstract function which must be implemented to handle what
	 * should happen when a frame has been altered outside the application.
	 * @param alteredFile the file which has been altered.
	 */
	virtual void animationChanged(const char *alteredFile) = 0;

	/**
	 * Abstract function for exporting the current project to a video file as
	 * specified by the video encoder.
	 * @param encoder the video encoder to use for export to video
	 * @return true on success, false otherwise
	 */
	virtual bool exportToVideo(VideoEncoder *encoder) = 0;

	/**
	 * Abstract function for exporting the current project to a Cinerella project.
	 * @param file the Cinerella project file
	 * @return true on success, false otherwise
	 */
	virtual bool exportToCinerella(const char *file) = 0;

protected:
	/**
	 * Pointer to the frontend that can be used to display progress
	 * and error messages.
	 */
	Frontend *frontend;

private:
	/**
	 * The datastructure containing the observers to be notified when something is
	 * changed in the model.
	 */
	vector<Observer*> observers;
};

#endif
