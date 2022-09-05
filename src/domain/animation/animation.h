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
#ifndef ANIMATION_H
#define ANIMATION_H

#include "src/config.h"

#include <stdint.h>
#include <stdio.h>
#include <exception>
#include <vector>


using namespace std;

class Executor;
class FileNameVisitor;
class Frontend;
class Observer;
class ObserverNotifier;
class VideoEncoder;
class ProjectSerializer;
class AudioDriver;
class FileCommandLogger;
class Scene;
class StringIterator;
class UiException;
class UndoRedoObserver;

class FailedToInitializeCommandLogger : public std::exception {
public:
	FailedToInitializeCommandLogger();
	const char* what() const throw();
};

/**
 * Represents the animation. Is responsible for the undo system and a bunch of
 * other minor stuff it really shouldn't be doing.
 */
class Animation {
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
	 * Registers the GUI frontend which is used to displaying and updating
	 * progress when running time-consuming operations.
	 * @param frontend the GUI frontend
	 */
	void registerFrontend(Frontend *frontend);

	/**
	 * Retrieves the registered frontend.
	 * @return the frontend if it is a valid frontend pointer, NULL otherwise.
	 */
	Frontend* getFrontend();

	/**
	 * Attempts to re-synch the UI after an error.
	 */
	void resync(std::exception& e);
	/**
	 * Attempts to re-synch the UI after a non-critical error.
	 */
	void resync(UiException& e);

	/**
	 * Inserts new frames into the animation model.
	 * @param scene The scene to add the frames to.
	 * @param frame The frame index to add the frames to in the animation.
	 * @param frameNames a vector containing the names to be added in the model
	 */
	void addFrames(int scene, int frame, StringIterator& frameNames);

	/**
	 * Removes frames from a scene in the animation.
	 * @param scene The scene from which to remove the frames.
	 * @param frame The first frame to remove.
	 * @param count The number of frames to remove.
	 */
	void removeFrames(int32_t scene, int32_t frame, int32_t count);

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
	void moveFrames(int32_t fromScene, int32_t fromFrame,
			int32_t count, int32_t toScene, int32_t toFrame);

	/**
	 * Returns the path to the image file for the frame specified.
	 * @param scene The scene to which the frame belongs.
	 * @param frame The frame within scene {@a scene} to query.
	 * @return The image path of frame number {@a frame} of scene {@a scene}.
	 * Ownership is not returned.
	 */
	const char* getImagePath(int scene, int frame) const;

	/**
	 * Replaces the image for the specified frame.
	 * @param scene The scene to which the frame belongs.
	 * @param frame The frame number of the frame within the scene.
	 * @param newImagePath The full path to the new image for this frame.
	 */
	void setImagePath(int32_t sceneNumber, int32_t frameNumber,
			const char* newImagePath);

	/**
	 * Duplicates the image file for the specified frame, setting the frames
	 * image to the duplicate.
	 * @param sceneNumber The scene to which the frame belongs.
	 * @param frameNumber The frame within the scene.
	 */
	void duplicateImage(int32_t sceneNumber, int32_t frameNumber);

	/**
	 * Adds the sound from the file "sound" to frame
	 * at position frameNumber.
	 * @param scene The scene containing the frame to add the sound to.
	 * @param frameNumber The number of the frame to add the sound to.
	 * @param filename The path to the file with the sound
	 */
	void addSound(int32_t scene, int32_t frameNumber, const char *filename);

	/**
	 * Removes the sound with index soundNumber from the frame with index
	 * frameNumber.
	 * @param sceneNumber The index of the scene containing the frame
	 * from which the sound is to be removed.
	 * @param frameNumber the index of the frame to remove a sound from.
	 * @param soundNumber the index of the sound to remove from the frame.
	 */
	void removeSound(int32_t sceneNumber, int32_t frameNumber,
			int32_t soundNumber);

	/**
	 * Returns the human-readable name of the sound specified.
	 * @param scene The scene to which the frame belongs.
	 * @param frame The frame to which the sound belongs.
	 * @param soundNumber The index of the sound.
	 * @return The name of sound indexed {@a soundNumber} of frame {@a frame}
	 * of scene {@a scene}. Ownership is not returned.
	 */
	const char* getSoundName(int scene, int frame, int soundNumber) const;

	/**
	 * Sets the name of the sound with index {@a soundNumber} in the frame with
	 * index {@a frameNumber} to {@a soundName}.
	 * @param sceneNumber The index of the scene containing the frame
	 * whose sound will be renamed.
	 * @param frameNumber the index of the frame the sound is in.
	 * @param soundNumber the index to the sound to change the name of.
	 * @param soundName the new name of the sound.
	 */
	void setSoundName(int32_t sceneNumber, int32_t frameNumber,
			int32_t soundNumber, const char* soundName);

	/**
	 * Returns the filename of the sound number {@a sound} within frame number
	 * {@a frame} within scene number {@a scene}.
	 * @param scene The index of the scene containing the frame.
	 * @param frame The index of the frame within scene {@a scene}
	 * @param sound The index of the sound within frame {@frame}.
	 * @return The path of the file containing the sound. Ownership is not
	 * returned.
	 */
	const char* getSoundPath(int scene, int frame, int sound) const;

	/**
	 * Play the sounds in the specified frame.
	 * @param scene The scene that the frame belongs to.
	 * @param frame The frame within that scene.
	 */
	void playSounds(int scene, int frame) const;

	/**
	 * Returns the total number of frames in the model.
	 * @return The total number of frames in all scenes of the model.
	 */
	int frameCount() const;

	/**
	 * Retrieves the size of the scene at index sceneNumber.
	 * @param sceneNumber the index of the scene to retrieve the size of.
	 * @return the size of the scene.
	 */
	int frameCount(int sceneNumber) const;

	/**
	 * Returns the number of sounds in the frame specified.
	 * @param scene The number of the scene.
	 * @param frame The number of the frame within the scene.
	 * @return The number of sounds attached to the specified frame.
	 */
	int soundCount(int scene, int frame) const;

	/**
	 * Returns the total number of sounds in the project.
	 * @return The total number of sounds in all frames in all scenes.
	 * Duplicates are counted the number of times they appear.
	 */
	int soundCount() const;

	/**
	 * Retrieves the number of scenes in the animation.
	 * @return the number of scenes in the animation.
	 */
	int sceneCount() const;

	void clearHistory();

	void clear();

	void undo();

	void redo();

	bool canUndo();
	bool canRedo();

	/**
	 * Sets the observer to receive notification when the can undo/can redo
	 * state changes.
	 */
	void setUndoRedoObserver(UndoRedoObserver* observer);

	/**
	 * Retrieves the project file.
	 * @return the project file if it has been set, NULL otherwise.
	 */
	const char *getProjectFile();

	/**
	 * Clears the animation and loads it from the dat file specified.
	 * If unsuccessful, the animation is untouched.
	 * @param filename The XML file to load.
	 * @param projectFilename The name of the project file this XML file came
	 * from or was last saved to, if known.
	 * @return {@c true} if successful.
	 */
	bool loadFromDat(const char* filename, const char* projectFilename);

	/**
	 * Opens a project.
	 * @param filename the project file to open (ends with .sto)
	 */
	void openProject(const char *filename);

	/**
	 * Saves the active project to a XML-file which is written to disk.
	 * @param filename the filename to store the project files within.
	 */
	void saveProject(const char *filename);

	/**
	 * Creates a new project.
	 * @throw FailedToInitializeCommandLogger if either the current project
	 * file could not be deleted or the command log file could not be
	 * reinitialized. The project may or may not be cleared in this case.
	 */
	void newProject();

	/**
	 * Checks if there are unsaved changes in the model.
	 * @return true if there are unsaved changes, false otherwise.
	 */
	bool isUnsavedChanges();

	/**
	 * Create and adds a new scene to the animation at position ``index''.
	 * @param index the position to add the new scene.
	 */
	void newScene(int32_t index);

	/**
	 * Removes the scene at the location sceneNumber from the animation.
	 * @param sceneNumber the scene to be removed from the animation.
	 */
	void removeScene(int32_t sceneNumber);

	/**
	 * Moves the scene at position sceneNumber to the position movePosition.
	 * @param sceneNumber the number of the scene to move.
	 * @param movePosition the position to move the scene to.
	 */
	void moveScene(int32_t sceneNumber, int32_t movePosition);

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
	 * Sets the file to be used as the command log.
	 * @param file The file, which must be opened for write. Ownership is
	 * passed.
	 */
	void setCommandLoggerFile(FILE* file);

	/**
	 * Replay the commands in the file.
	 * @param file The command log file, opened for reading and seeked to the
	 * beginning of the file (or wherever is appropriate).
	 * @throws FileException if the file could not be read in its entirety, or
	 * another exception if decoding the log failed. Some commands may have
	 * been applied.
	 */
	void replayCommandLog(FILE* file);

	/**
	 * Have v visit all the files referenced (images and sounds)
	 */
	void accept(FileNameVisitor& v) const;

private:
	void setScenes(const std::vector<Scene*>& sv);
	FILE* initializeCommandLog();

	/** All of the scenes in the animation. */
	ObserverNotifier* scenes;

	/** Undo and disaster recovery. */
	Executor* executor;
	FileCommandLogger* logger;

	/** Serializer to be used on saving and loading of the project. */
	ProjectSerializer *serializer;

	/** Audio driver that can play sound. */
	AudioDriver *audioDriver;

	/** Variable for checking if the audio device is successfully initialized. */
	bool isAudioDriverInitialized;

	/** For the reporting of errors and warnings */
	Frontend* frontend;
};

#endif
