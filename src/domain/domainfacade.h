/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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
#ifndef DOMAINFACADE_H
#define DOMAINFACADE_H

#include <vector>

class Animation;
class Observer;
class Frontend;
class VideoEncoder;
class StringIterator;
class UndoRedoObserver;
struct GrabberDevice;

/**
 * Singleton facade for the domain level. All requests into the domain layer
 * has to pass through this facade. The only exception is observer requests
 * for display data.
 *
 * DomainFacade is implemented with lazy allocation.
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class DomainFacade {
public:
	~DomainFacade();

	/**
	 * Static function to retrieve the factory instance object from the singleton
	 * class.
	 * @return the factory for sending requests into the domain.
	 */
	static DomainFacade* getFacade();

	/**
	 * Attaches a new observer to the model. The observer will be notified when
	 * something is changed in the model.
	 * @param o the observer to be attached to the model.
	 */
	void attach(Observer *o);

	/**
	 * Detaches an observer from the model. The observer will no longer be notified
	 * when something is changed in the model.
	 * @param o the observer to be detached from the model.
	 */
	void detach(Observer *o);

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
	 * Adds the frames in the vector to the animation model.
	 * @param scene The index of the scene to add frames to.
	 * @param frame Where in the scene to add the frames.
	 * @param frameNames A vector containing the names of the filenames of the
	 * images to be added as frames to the animation.
	 */
	void addFrames(int scene, int frame,
			StringIterator& frameNames);

	/**
	 * Removes frames from a scene in the animation.
	 * @param scene The scene from which to remove the frames.
	 * @param frame The first frame to remove.
	 * @param count The number of frames to remove.
	 */
	void removeFrames(int scene, int frame, int count);

	/**
	 * Moves frames from one position in the animation to another.
	 * @param fromScene The scene from which to move the frames.
	 * @param fromFrame The first frame within that scene to move.
	 * @param count The number of frames to move; all frames moved must be
	 * within the same scene.
	 * @param fromScene The scene to move the frames to.
	 * @param toFrame The position within the scene {@a toScene} to which
	 * the frames should be moved.
	 */
	void moveFrames(int fromScene, int fromFrame,
			int count, int toScene, int toFrame);

	/**
	 * Duplicates the image file of the specified frame, setting the image to
	 * the duplicate. This is used when the image is to be edited so that the
	 * edit can be undone and so that shared images are unshared.
	 * @param scene The index of the scene to which the frame belongs.
	 * @param frame The index of the frame within the scene.
	 */
	void duplicateImage(int scene, int frame);

	/**
	 * Returns the image file for the frame specified.
	 * @param scene The scene to which the frame belongs.
	 * @param frame The frame number to examine.
	 * @return The path to the image file for this frame. Ownership is not
	 * returned.
	 */
	const char* getImagePath(int scene, int frame);

	/**
	 * Adds a sound the given frame number. An error message will be
	 * sent to the frontend if somethings goes wrong with the adding.
	 * @param sceneNumber The scene containing the frame to which a sound
	 * will be added.
	 * @param frameNumber the number of the frame to add the sound to
	 * @param filename the path to the file with the sound
	 */
	void addSound(int sceneNumber, int frameNumber, const char* filename);

	/**
	 * Removes the sound with index soundNumber from the frame with index
	 * frameNumber.
	 * @param sceneNumber The scene containing the frame from which a sound
	 * is to be removed.
	 * @param frameNumber the index of the frame to remove a sound from.
	 * @param soundNumber the index of the sound to remove from the frame.
	 */
	void removeSound(int sceneNumber, int frameNumber, int soundNumber);

	/**
	 * Sets the name of the sound with index soundNumber in the frame with
	 * index frameNumber to soundName.
	 * @param sceneNumber The scene containing the frame containing the sound
	 * to be renamed.
	 * @param frameNumber the index of the frame the sound is in.
	 * @param soundNumber the index to the sound to change the name of.
	 * @param soundName the new name of the sound.
	 */
	void setSoundName(int sceneNumber, int frameNumber, int soundNumber,
			const char* soundName);

	/**
	 * Returns the name of a sound attached to a frame in the active scene.
	 * @param sceneNumber The scene containing the frame to query.
	 * @param frameNumber The frame within the scene {@a sceneNumber}.
	 * @param The number of the sound whose name is to be returned.
	 * @return The sound name. Ownership is not returned.
	 */
	const char* getSoundName(int sceneNumber, int frameNumber,
			int soundNumber) const;

	/**
	 * Returns the number of sounds attached to the frame specified.
	 * @param scene The scene that the frame belongs to.
	 * @param frame The frame within that scene.
	 * @return The number of sounds attached to frame number {@a frame} of
	 * scene number {@a scene}.
	 */
	int soundCount(int scene, int frame) const;

	/**
	 * Play the sounds in the specified frame.
	 * @param scene The scene that the frame belongs to.
	 * @param frame The frame within that scene.
	 */
	void playSounds(int scene, int frame) const;

	/**
	 * Loads the project from the {@c .dat} XML file specified. Used for
	 * recovering from the {@c new.dat} and {@c current.dat} files.
	 * @param datFilename The XML file to load.
	 * @param projectFilename The name of the {@c .sto} file that this XML file
	 * came from or was last saved into, if known. Null if this is unknown. It
	 * does not matter if this file still exists or not; it will not be
	 * accessed in this operation, it is simply to set the default name to save
	 * to and main window title.
	 * @return {@c true} if successful.
	 */
	bool loadProject(const char* datFilename, const char* projectFilename);

	/**
	 * Replay commands from the log.
	 * @param filename Filename of the log.
	 * @return {@c true} if successful.
	 */
	bool replayCommandLog(const char* filename);

	/**
	 * Opens a project.
	 * @param filename The file to load
	 */
	void openProject(const char *filename);

	/**
	 * Saves the active project to a XML-file which is written to disk.
	 */
	void saveProject(const char *filename);

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
	 * Retrieves the size of the model
	 * @return the size of the model
	 */
	int getModelSize() const;

	/**
	 * Function for retrieving number of frames in a given scene.
	 * @param sceneNumber the scene number
	 * @return number of frames
	 */
	int getSceneSize(int sceneNumber) const;

	/**
	 * Function for retrieving number of scenes.
	 * @return number of scenes
	 */
	int getNumberOfScenes() const;

	/**
	 * Returns the number of sounds attached to a frame
	 * @param scene The index of the scene containing the frame.
	 * @param frame The index within the scene of the frame.
	 * @return The number of sounds belonging to the specified frame.
	 */
	int getNumberOfSounds(int scene, int frame) const;

	/**
	 * Retrieves the project file.
	 * @return The project file if it has been set, NULL otherwise.
	 */
	const char* getProjectFile();

	/**
	 * Sets the preferences in the preferences file for current,
	 * most recent, second most recent and third most recent project filenames.
	 */
	void setMostRecentProject();

	/**
	 * Undoes the last undoable operation on the model.
	 */
	void undo();

	/**
	 * Redoes the last undo operation.
	 */
	void redo();

	/**
	 * Detects if undo is possible.
	 */
	bool canUndo();

	/**
	 * Detects if redo is possible.
	 */
	bool canRedo();

	/**
	 * Clears the undo history.
	 */
	void clearHistory();

	/**
	 * Sets the observer to receive notification of when the can undo/can redo
	 * state changes.
	 */
	void setUndoRedoObserver(UndoRedoObserver* observer);

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
	 * @param encoder The encoder to be used for video export
	 * @param playbackSpeed The frame rate (in frames per second) at which
	 * the video should be encoded, assuming the video encoder supports
	 * this adjustment.
	 * @return true on success, false otherwise
	 */
	bool exportToVideo(VideoEncoder *encoder, int playbackSpeed);

	/**
	 * Exports the current project to a Cinerella project.
	 * @param file the Cinerella project file
	 * @return true on success, false otherwise
	 */
	bool exportToCinerella(const char *file);

	/**
	 * Returns available grabber devices.
	 * @return vector containing available devices.
	 */
	const std::vector<GrabberDevice> getGrabberDevices();

	/**
	 * Sets up the command logger file. If unsuccessful, a message will have
	 * been displayed to the user.
	 */
	void initializeCommandLoggerFile();

protected:
	/**
	 * The constructor. It is protected so that it will be impossible for other classes,
	 * classes that don't inherit from it to instantiate the singleton.
	 */
	DomainFacade();

private:
	/**The singleton instance of this class.*/
	static DomainFacade *domainFacade;

	/**The datamodel in the program*/
	Animation *animationModel;
};

#endif
