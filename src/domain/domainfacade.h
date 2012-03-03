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
#ifndef DOMAINFACADE_H
#define DOMAINFACADE_H

#include "src/config.h"
#include "animation/animationmodel.h"
#include "src/domain/undo/undohistory.h"
#include "src/presentation/frontends/frontend.h"
#include "src/technical/video/videoencoder.h"
#include "src/technical/util.h"

#include <vector>
using namespace std;


/**
 * Singleton facade for the domain level. All requests into the domain layer
 * has to pass through this facade. The only exception is observer requests
 * for display data.
 *
 * DomainFacade is implemented with lazy allocation.
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class DomainFacade
{
public:
	~DomainFacade();
	
	/**
	 * Static function to retrieve the factory instance object from the singleton 
	 * class.
	 * @return the factory for sending requests into the domain.
	 */
	static DomainFacade* getFacade();
	
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
	 * Registers the GUI frontend which is used to display and update
	 * a progress bar when running time consuming operations.
	 * @param frontend the GUI frontend
	 */
	void registerFrontend(Frontend *frontend);
	
	/**
	 * Retrieves the registered frontend.
	 * @return the frontend if it is a valid frontend pointer, NULL otherwise.
	 */
	Frontend* getFrontend();
	
	/**
	 * Function to change the currently active frame. (Working frame).
	 * @param frameNumber the number of the new active frame.
	 */
	void setActiveFrame(int frameNumber);   
	
	/**
	 * Returns the number of the currently active frame in the model.
	 * @return 
	 */
	int getActiveFrameNumber();
	 
	/**
	 * Adds the frames in the vector to the animation model and sets up the undo 
	 * command object
	 * @param frameNames a vector containing the frames to be added to the animation.
	 */
	void addFrames(const vector<char*>& frameNames);
	
	/**
	 * Removes the frame between (inclusive) fromFrame and toFrame from
	 * the animation model.
	 * @param fromFrame the first frame to remove.
	 * @param toFrame the last frame to remove.
	 */
	void removeFrames(unsigned int fromFrame, unsigned int toFrame);
	
	/**
	 * Moves the frames in the frameNumbers container to toPosition.
	 * @param fromFrame the first frame in the selection.
	 * @param toFrame the last frame in the selection.
	 * @param movePosition the position to move the selected frames
	 */
	void moveFrames(unsigned int fromFrame, unsigned int toFrame, 
			unsigned int movePosition);
	
	/**
	 * Adds a sound the given frame number. An error message will be 
	 * sent to the frontend if somethings goes wrong with the adding.
	 * @param frameNumber the number of the frame to add the sound to
	 * @param filename the path to the file with the sound
	 * @return zero on success, less than zero on failure
	 */
	int addSound(unsigned int frameNumber, const char* filename);
	
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
	 * Plays the frame with the number frameNumber
	 * @param frameNumber the number of the frame to play
	 */
	void playFrame(int frameNumber);
	
	/**
	 * Opens a project.
	 */
	bool openProject(const char *filename);
	
	/**
	 * Saves the active project to a XML-file which is written to disk.
	 */
	bool saveProject(const char *filename);
	
	/**
	 * Creates a new project.
	 */
	bool newProject();
	
	/**
	 * Checks if there are unsaved changes in the model.
	 * @return true if there are unsaved changes, false otherwise.
	 */
	bool isUnsavedChanges();
	
	/**
	 * Retrieves a given frame.
	 * @param frameNumber the number of the frame to retrieve.
	 * @param sceneNumber the number of the scene to retrieve the frame from.
	 * @return the frame at location frameNumber in the scene at location sceneNumber.
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
	 * Retrieves the size of the model
	 * @return the size of the model
	 */
	unsigned int getModelSize();
	
	/**
	 * Function for retrieving number of frames in a given scene.
	 * @param sceneNumber the scene number
	 * @return number of frames
	 */
	unsigned int getSceneSize(int sceneNumber);
	
	/**
	 * Function for retrieving number of scenes.
	 * @return number of scenes
	 */
	unsigned int getNumberOfScenes();
	
	/**
	 * Retrieves the project file.
	 * @return the project file if it's setted, NULL otherwise.
	 */
	const char* getProjectFile();
	 
	/**
	 * Undoes the last undoable operation on the model.
	 */
	bool undo();
	
	/**
	 * Redoes the last undo operation.
	 */
	bool redo();
	
	/**
	 * Clears the undo history.
	 */
	void clearHistory();
	
	/**
	 * Sets a new active scene (the scene to be worked on now).
	 * @param sceneNumber the new active scene.
	 */
	void setActiveScene(int sceneNumber);
	
	/**
	 * Returns the number of the currently active scene.
	 * @return the number of the active scene.
	 */
	int getActiveSceneNumber();
	
	/**
	 * Creates a new scene in the animation project.
	 * @param index the place the new scene should be added to.
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
	 * Initializes the audio device so it is ready to play sounds. It will sends
	 * an error message to the frontend if somethings goes wrong, and audio will
	 * be disabled until the problem is fixed.
	 * @return true on success, false otherwise
	 */
	bool initAudioDevice();
	
	/**
	 * Shutdowns the audio device so other programs can use it.
	 */
	void shutdownAudioDevice();
	
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
	
	/**
	 * Tells the domain that the file alteredFile has been changed outside 
	 * the program and that it should be updated.
	 * @param alteredFile the file which has been altered.
	 */
	void animationChanged(const char *alteredFile);

	/** 
	 * Returns available grabber devices.
	 * @return vector containing available devices.
	 */
	const vector<GrabberDevice> getGrabberDevices();	
	 
protected:
	/**
	 * The constructor. It is protected so that it will be impossible for other classes,
	 * which don't inherit from it to instanciate the singleton. 
	 */
	DomainFacade();
	
private:
	/**The singleton instance of this class.*/
	static DomainFacade *domainFacade;
	
	/**The datamodel in the program*/
	AnimationModel *animationModel;
	
	/**Container where one can register and retrieve undo objects for undo and
	 remove operations*/
	UndoHistory *undoHistory;
};

#endif
