/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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
#ifndef ANIMATION_H
#define ANIMATION_H

#include "src/config.h"
#include "animationmodel.h"
#include "scene.h"
// #include "externalchangemonitor.h"
#include "src/technical/projectserializer.h"
#include "src/technical/audio/ossdriver.h"

#include "frame.h"

#include <vector>
#include <libxml/tree.h>

using namespace std;


/**
 * Implementation of the animationmodel containing the data about the animation.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Animation : public AnimationModel
{
public:
	/**
	 * Initializes the variables of the animation to starting values.
	 */
	Animation();
	
	/**
	 * Cleans up the animation.
	 */
	~Animation();

	/**
	 * Inserts a new frame into the animationmodel.
	 * @param frameNames a vector containing the names to be added in the model
	 * @param index the place to add the frames in the animation.
	 * @return a vector containing paths to the images which has been
	 * copied to a temporary directory
	 */
	const vector<char*> addFrames(const vector<char*>& frameNames, 
			unsigned int index);
	
	/**
	 * Removes the frames between (inclusive) fromFrame and toFrame from 
	 * the animation.
	 * @param fromFrame the first frame to remove.
	 * @param toFrame the last frame to remove.
	 * @return a vector containing paths to the images in the given range,
	 * which are moved to a trash directory.
	 */
	const vector<char*> removeFrames(unsigned int fromFrame, 
			const unsigned int toFrame);
	
	/**
	 * Move the frames from fromFrame (inclusive) to toFrame to toPosition.
	 * @param fromFrame the first frame to move.
	 * @param toFrame the last frame to move.
	 * @param movePosition the posititon to move the frames to.
	 */
	void moveFrames(unsigned int fromFrame, unsigned int toFrame, 
			unsigned int movePosition);
	
	/**
	 * Adds the sound from the file "sound" to frame
	 * at position frameNumber.
	 * @param frameNumber the number of the frame to add the sound to
	 * @param filename the path to the file with the sound
	 * @return zero on success, less than zero on failure;
	 * -1 = file is not readable
	 * -2 = not a valid audio file
	 */
	int addSound(unsigned int frameNumber, const char *filename);
	
	/**
	 * Removes the sound with index soundNumber from the frame with index
	 * frameNumber.
	 * @param frameNumber the index of the frame to remove a sound from.
	 * @param soundNumber the index of the sound to remove from the frame.
	 */
	void removeSound(unsigned int frameNumber, unsigned int soundNumber);
	
	/**
	 * Sets the name of the sound with index soundNumber in the frame with
	 * index frameNumber to soundName.
	 * @param frameNumber the index of the frame the sound is in.
	 * @param soundNumber the index to the sound to change the name of.
	 * @param soundName the new name of the sound.
	 */
	void setSoundName(unsigned int frameNumber, unsigned int soundNumber,
			char* soundName);
	
	/**
	 * Returns the frame with the number frameNumber.
	 * @param frameNumber the number of the frame to retrieve
	 * @param sceneNumber the number of the scene to retrieve the frame from
	 * @return the frame with the number frameNumber.
	 */
	Frame* getFrame(unsigned int frameNumber, unsigned int sceneNumber);
	
	/**
	 * Overloaded function for convenience. Assumes the scene to retrieve there
	 * frame from is the active scene.
	 * @param frameNumber the number of the frame to retrieve.
	 * @return the frame at location frameNumber in the active scene. 
	 */
	Frame* getFrame(unsigned int frameNumber);
	
	/**
	 * Returns the size of the model.
	 * @return the size of the model.
	 */
	unsigned int getModelSize();
	
	/**
	 * Retrieves the size of the scene at index sceneNumber.
	 * @param sceneNumber the index of the scene to retrieve the size of.
	 * @return the size of the scene.
	 */
	unsigned int getSceneSize(int sceneNumber);
	
	/**
	 * Retrieves the number of scenes in the animation.
	 * @return the number of scenes in the animation.
	 */
	unsigned int getNumberOfScenes();
	
	/**
	 *Function for changing the currently active frame.
	 *@param frameNumber the number of the new active frame.
	 */
	void setActiveFrame(int frameNumber);

	/**
	  * Plays the frame with number frameNumber.
	  * @param frameNumber the number of the frame to play.
	  */
	void playFrame(int frameNumber);
	  
	/**
	 * Returns the number of the currently active frame
	 * in the model.
	 * @return the number of the currently active frame.
	 */
	int getActiveFrameNumber();
	
	/**
	 * Clears the model.
	 */
	void clear();
	
	/**
	 * Retrieves the project directory
	 * @return the project directory if it's setted, NULL otherwise.
	 */
	const char *getProjectFile();
	
	/**
	 * Opens a project.
	 * @param filename the project file to open (ends with .sto)
	 * @return true on success, false otherwise
	 */
	bool openProject(const char *filename);
	
	/**
	 * Saves the active project to a XML-file which is written to disk.
	 * @param filename the filename to store the project files within.
	 * @return true on success, false otherwise
	 */
	bool saveProject(const char *filename);
	
	/**
	 * Creates a new project.
	 * @return true on success, false otherwise
	 */
	bool newProject();
	
	/**
	 * Checks if there are unsaved changes in the model.
	 * @return true if there are unsaved changes, false otherwise.
	 */
	bool isUnsavedChanges();
	
	/**
	 * Sets the scene with at position sceneNumber as the active scene.
	 * @param sceneNumber the number of the new active scene.
	 */
	void setActiveScene(int sceneNumber);
	
	/**
	 * Returns the number of the currently active scene.
	 * @return the number of the active scene.
	 */
	int getActiveSceneNumber();
	
	/**
	 * Create and adds a new scene to the animation at position ``index''.
	 * @param index the position to add the new scene.
	 */
	void newScene(int index);
	
	/**
	 * Removes the scene at the location sceneNumber from the animation.
	 * @param sceneNumber the scene to be removed from the animation.
	 */
	void removeScene(int sceneNumber);
	
	/**
	 * Moves the scene at position sceneNumber to the position movePosition.
	 * @param sceneNumber the number of the scene to move.
	 * @param movePosition the position to move the scene to.
	 */
	void moveScene(int sceneNumber, int movePosition);
	
	/**
	 * Initializes the audio device so it is ready to play sounds.
	 * @return true on success, false otherwise
	 */
	bool initAudioDevice();
	
	/**
	 * Shutdowns the the audio device so that other programs can use it.
	 */
	void shutdownAudioDevice();
	
	/**
	 * Function to call when a file has been altered and the gui
	 * should be updated.
	 * @param alteredFile the file which has been altered.
	 */
	void animationChanged(const char *alteredFile);
	
	/**
	 * Exports the current project to a video file as specified by the video encoder.
	 * @param encoder the video encoder to use for export to video
	 * @return true on success, false otherwise
	 */
	bool exportToVideo(VideoEncoder *encoder);
	
	/**
	 * Exports the current project to a Cinerella project.
	 * @param file the Cinerella project file
	 * @return true on success, false otherwise
	 */
	bool exportToCinerella(const char *file);
	
private:
	/** All of the scenes in the animation. */
	vector<Scene*> scenes;
	
	/** Serializer to be used on saving and loading of the project. */
	ProjectSerializer *serializer;
	
	/** Class that listens for changes in the disk representation of there
	    animation and notifies this class when something is changed        */
// 	ExternalChangeMonitor *externalChangeMonitor;
	
	/** Audio driver that can play sound. */
	AudioDriver *audioDriver;
	
	/** Index of the active frame. */
	int activeFrame;
	
	/** Index of the active scene. */
	int activeScene;
	
	/** Number of sounds added to the model. */
	int numSounds;
	
	/** Variable for checking if there are unsaved changes in the model */
	bool isChangesSaved;
	
	/** Variable for checking if the audio device is successfully initialized. */
	bool isAudioDriverInitialized;
	
	/**
	 * Loads frames into the model. This acts excatly like the addframes
	 * function except that it does not moves the frames to a temporary directory.
	 * It also registers the frame to be a valid project frame.
	 */
	void loadSavedScenes();
	
	/**
	 * Sets the scene at location sceneNumber as the active scene.
	 * @param sceneNumber the new active scene
	 */
	void activateScene(int sceneNumber);
};

#endif
