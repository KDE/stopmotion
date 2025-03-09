/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#ifndef SCENEVECTOR_H_
#define SCENEVECTOR_H_

#include <vector>

#include "animationimpl.h"

class Scene;
class Frame;
class Sound;
class FileNameVisitor;
class WorkspaceFile;
class AudioDriver;

class SceneVector : public AnimationImpl {
	std::vector<Scene*> scenes;
	int totalSoundCount;

	Scene* getMutableScene(int);
	SceneVector(const SceneVector&);
	SceneVector& operator=(const SceneVector&);
public:
	SceneVector();
	~SceneVector() override;
	/**
	 * Clears the animation so that there are no scenes and no frames.
	 */
	void clear() override;
	/**
	 * Returns the number of scenes in the animation.
	 * @return The number of scenes.
	 */
	int sceneCount() const override;
	/**
	 * Adds a scene to the animation.
	 * @param where Where in the sequence of scenes to insert this one. Must be
	 * between 0 and {@c SceneCount()} inclusive.
	 * @param newScene The scene to be inserted; ownership is passed.
	 */
	void addScene(int where, Scene* newScene) override;
	/**
	 * Adds a fresh empty scene.
	 * @param where The scene number that this new scene will have.
	 */
	void addScene(int where) override;
	/**
	 * Preallocates memory so that AddScene can be called without the risk of
	 * throwing an exception.
	 * @param count Number of calls to @ref AddScene required without
	 * exceptions being thrown.
	 */
	void preallocateScenes(int count) override;
	/**
	 * Removes a scene from the animation, together with all its frames.
	 * @param from The scene to remove. Must be between 0 and @code{.cpp}
	 * SceneCount() - 1 @endcode inclusive.
	 * @return The removed scene. Ownership is returned.
	 */
	Scene* removeScene(int from) override;
	/**
	 * Moves a scene.
	 * @param from The scene number to move. Must be between 0 and
	 * {@c SceneCount()} inclusive.
	 * @param from The number the scene is to become. Must be between 0 and
	 * {@c SceneCount()} inclusive.
	 */
	void moveScene(int from, int to) override;
	/**
	 * Returns the scene requested, which must not be modified.
	 * @param which The number of the scene to be returned.
	 * @return The scene requested.
	 */
	const Scene* getScene(int which) const override;
	/**
	 * Returns the number of frames in scene number @a scene.
	 * @param scene The number of the scene to query.
	 * @return The number of frames in scene number @a scene.
	 */
	int frameCount(int scene) const override;
	/**
	 * Adds a frame to the animation.
	 * @param scene The number of the scene to add the frames to.
	 * @param where The number of the frame within scene number @a scene to
	 * add the frame.
	 * @param frame The frame to add. Ownership is passed.
	 */
	void addFrame(int scene, int where, Frame* frame) override;
	/**
	 * Adds frames to the animation.
	 * @param scene The scene to which the frames will be added.
	 * @param where The frame number for the first of the added frames.
	 * @param frames The frames to add.
	 * @note On failure, no frames will have been added.
	 */
	void addFrames(int scene, int where, const std::vector<Frame*>& frames) override;
	/**
	 * Preallocates enough memory for @ref AddFrames to add @a count frames
	 * to scene number @scene.
	 */
	void preallocateFrames(int scene, int count) override;
	/**
	 * Removes a frame from the animation.
	 * @param scene The scene to remove the frame from.
	 * @param frame The number of the first frame within scene @a scene
	 * to remove.
	 * @return The removed frame; ownership is returned.
	 * @throws Will not throw.
	 */
	Frame* removeFrame(int scene, int frame) override;
	/**
	 * Removes frames from the animation.
	 * @param scene The scene from which to remove the frames.
	 * @param frame The first frame to remove.
	 * @param count The number of frames to remove.
	 * @param [out] out The removed frames will be added to the end of this
	 * vector. Ownership is returned.
	 * @note Upon failure, {@a out} and the animation will both be untouched.
	 * @pre The scene must have at least @code{.cpp} frame + count @endcode
	 * frames.
	 */
	void removeFrames(int scene, int frame, int count, std::vector<Frame*>& out) override;
	/**
	 * Moves frames.
	 * @param fromScene The number of the scene from which to move frames.
	 * @param fromFrame The number of the frame within scene @a fromScene from
	 * which frames should be moved.
	 * @param frameCount The number of frames to move. All the frames must be
	 * from the same scene.
	 * @param toScene The number of the scene to which to move the frames.
	 * @param toFrame The number of the frame within scene @a toScene to which
	 * the frames should be moved.
	 */
	void moveFrames(int fromScene, int fromFrame, int frameCount,
			int toScene, int toFrame) override;
	/**
	 * Returns the file path of the image of the specified frame.
	 */
	const char* getImagePath(int scene, int frame) const;
	/**
	 * Replaces the image of the frame at index {@a frameNumber} of scene
	 * {@a sceneNumber}.
	 * @param sceneNumber The index of the scene containing the frame to alter.
	 * @param frameNumber The index of the frame to alter.
	 * @param [in,out] The image to swap with. On exit, the frame at index
	 * {@a frameNumber} will have the image formerly held by
	 * {@a otherImage} and {@a otherImage} will have the image formerly held
	 * by the frame.
	 */
	void replaceImage(int sceneNumber, int frameNumber,
			WorkspaceFile& otherImage) override;
	/**
	 * Returns the number of sounds attached to a frame.
	 * @param scene The number of the scene that contains the frame that you
	 * need to examine.
	 * @param frame The number of the frame from @a scene to examine.
	 * @return The number of sounds attached to frame number @a frame of scene
	 * @a scene.
	 */
	int soundCount(int scene, int frame) const override;
	/**
	 * Returns the total number of sounds in the animation.
	 */
	int soundCount() const override;
	/**
	 * Adds a sound to a frame of animation.
	 * @param scene The number of the scene containing the frame to add the
	 * sound to.
	 * @param frame The number of the frame within scene number @a scene to
	 * add the sound to.
	 * @param soundNumber The position of the sound to add. Must be between
	 * 0 and @code{.cpp} SoundCount(scene, frame) @endcode inclusive.
	 * @param sound The name sound to add. Ownership is passed.
	 */
	void addSound(int scene, int frame, int soundNumber,
			Sound* sound) override;
	/**
	 * changes the name of a sound of a frame.
	 * @param scene The number of the scene containing the frame to change.
	 * @param frame The number of the frame within scene number @a scene
	 * containing the sound to be named.
	 * @param soundNumber Which sound within frame number @a frame of scene
	 * number @a scene to be replaced.
	 * @param soundName The new name of the sound to be added. Ownership
	 * is passed; must have been created with {@c new char[]}.
	 * @return The old name of the sound. Ownership is returned; must be
	 * deleted with {@c delete[]}.
	 * @throws Nothing.
	 */
	const char* setSoundName(int scene, int frame, int soundNumber,
			const char* soundName) override;
	/**
	 * Removes a sound of a frame.
	 * @param scene The number of the scene containing the frame to change.
	 * @param frame The number of the frame within scene number @a scene
	 * containing the sound to be changed.
	 * @param soundNumber Which sound within frame number @a frame of scene
	 * number @a scene to be removed.
	 * @return The file name of the sound that is being removed. Ownership
	 * is returned.
	 * @throws Nothing.
	 */
	Sound* removeSound(int scene, int frame, int soundNumber) override;
	/**
	 * Plays the sounds attached to the specified frame.
	 * @param scene The number of the scene containing the sounds.
	 * @param frame The number of the frame within scene number @a scene
	 * @param audioDriver The audio driver to play the sounds with
	 */
	void playSounds(int scene, int frame, AudioDriver* audioDriver) const override;
	void accept(FileNameVisitor& v) const override;
};

#endif /* SCENEVECTOR_H_ */
