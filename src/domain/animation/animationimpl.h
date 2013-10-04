/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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


#ifndef ANIMATIONIMPL_H_
#define ANIMATIONIMPL_H_

#include <vector>

class WorkspaceFile;
class FrameIterator;
class Scene;
class Frame;
class Sound;
class FileNameVisitor;

/**
 * Base class for implementation class of the animation, and any wrappers.
 * Represents an animation that can be updated by the undo system. This
 * interface should therefore not be used by any code upstream of the
 * undo system.
 */
class AnimationImpl {
public:
	virtual ~AnimationImpl() = 0;
	/**
	 * Clears the animation so that there are no scenes and no frames.
	 */
	virtual void clear() = 0;
	/**
	 * Returns the number of scenes in the animation.
	 * @return The number of scenes.
	 */
	virtual int sceneCount() const = 0;
	/**
	 * Constructs an iterator for the frames in the specified scene. The
	 * iterator is valid as long as that scene is not altered.
	 * @param scene The scene to examine.
	 * @return The iterator; ownership is returned.
	 */
	FrameIterator* makeFrameIterator(int scene) const;
	/**
	 * Constructs an iterator for the frames in the specified scene. The
	 * iterator is valid as long as that scene is not altered.
	 * @param scene The scene to examine.
	 * @param start The frame number from which to start the iteration.
	 * @param end The frame number at which to end the iteration (i.e. this
	 * frame will not be considered part of the iteration). Passing the number
	 * of frames in the scene is acceptable.
	 * @return The iterator; ownership is returned.
	 */
	virtual FrameIterator* makeFrameIterator(int scene, int start, int end)
			const = 0;
	/**
	 * Adds a scene to the animation.
	 * @param where Where in the sequence of scenes to insert this one. Must be
	 * between 0 and @c SceneCount() inclusive.
	 * @param newScene The scene to be inserted; ownership is passed.
	 */
	virtual void addScene(int where, Scene* newScene) = 0;
	/**
	 * Adds a fresh empty scene.
	 * @param where The scene number that this new scene will have.
	 */
	virtual void addScene(int where) = 0;
	/**
	 * Preallocates memory so that AddScene can be called without the risk of
	 * throwing an exception.
	 * @param count Number of calls to @ref AddScene required without
	 * exceptions being thrown.
	 */
	virtual void preallocateScenes(int count) = 0;
	/**
	 * Removes a scene from the animation, together with all its frames.
	 * @param from The scene to remove. Must be between 0 and @code{.cpp}
	 * SceneCount() - 1 @endcode inclusive.
	 * @return The removed scene. Ownership is returned.
	 */
	virtual Scene* removeScene(int from) = 0;
	/**
	 * Moves a scene.
	 * @param from The scene number to move. Must be between 0 and
	 * @c SceneCount() inclusive.
	 * @param from The number the scene is to become. Must be between 0 and
	 * @c SceneCount() inclusive.
	 */
	virtual void moveScene(int from, int to) = 0;
	/**
	 * Returns the scene requested, which must not be modified.
	 * @param which The number of the scene to be returned.
	 * @return The scene requested.
	 */
	virtual const Scene* getScene(int which) const = 0;
	/**
	 * Returns the number of frames in scene number @c scene.
	 * @param scene The number of the scene to query.
	 * @return The number of frames in scene number @c scene.
	 */
	virtual int frameCount(int scene) const = 0;
	/**
	 * Adds a frame to the animation.
	 * @param scene The number of the scene to add the frames to.
	 * @param where The number of the frame within scene number @c scene to
	 * add the frame.
	 * @param frame The frame to add. Ownership is passed.
	 */
	virtual void addFrame(int scene, int where, Frame* frame) = 0;
	/**
	 * Adds frames to the animation.
	 * @param scene The scene to which the frames will be added.
	 * @param where The frame number for the first of the added frames.
	 * @param frames The frames to add.
	 * @note On failure, no frames will have been added.
	 */
	virtual void addFrames(int scene, int where,
			const std::vector<Frame*>& frames) = 0;
	/**
	 * Preallocates enough memory for @ref AddFrames to add @c count frames
	 * to scene number @scene.
	 */
	virtual void preallocateFrames(int scene, int count) = 0;
	/**
	 * Removes a frame from the animation.
	 * @param scene The scene to remove the frame from.
	 * @param frame The number of the first frame within scene @c scene
	 * to remove.
	 * @return The removed frame; ownership is returned.
	 * @throws Will not throw.
	 */
	virtual Frame* removeFrame(int scene, int frame) = 0;
	/**
	 * Removes frames from the animation.
	 * @param scene The scene from which to remove the frames.
	 * @param frame The first frame to remove.
	 * @param count The number of frames to remove.
	 * @param [out] out The removed frames will be added to the end of this
	 * vector. Ownership is returned.
	 * @note Upon failure, {@c out} and the animation will both be untouched.
	 * @pre The scene must have at least @code{.cpp} frame + count @endcode
	 * frames.
	 */
	virtual void removeFrames(int scene, int frame, int count,
			std::vector<Frame*>& out) = 0;
	/**
	 * Moves frames.
	 * @param fromScene The number of the scene from which to move frames.
	 * @param fromFrame The number of the frame within scene @c fromScene from
	 * which frames should be moved.
	 * @param frameCount The number of frames to move. All the frames must be
	 * from the same scene.
	 * @param toScene The number of the scene to which to move the frames.
	 * @param toFrame The number of the frame within scene @c toScene to which
	 * the frames should be moved.
	 */
	virtual void moveFrames(int fromScene, int fromFrame, int frameCount,
			int toScene, int toFrame) = 0;
	/**
	 * Replaces the image of the frame at index {@c frameNumber} of scene
	 * {@c sceneNumber}.
	 * @param sceneNumber The index of the scene containing the frame to alter.
	 * @param frameNumber The index of the frame to alter.
	 * @param [in,out] The image to swap with. On exit, the frame at index
	 * {@c frameNumber} will have the image formerly held by
	 * {@c otherImage} and {@c otherImage} will have the image formerly held
	 * by the frame.
	 */
	virtual void replaceImage(int sceneNumber, int frameNumber,
			WorkspaceFile& otherImage) = 0;
	/**
	 * Returns the number of sounds attached to a frame.
	 * @param scene The number of the scene that contains the frame that you
	 * need to examine.
	 * @param frame The number of the frame from @c scene to examine.
	 * @return The number of sounds attached to frame number @c frame of scene
	 * @c scene.
	 */
	virtual int soundCount(int scene, int frame) const = 0;
	/**
	 * Adds a sound to a frame of animation.
	 * @param scene The number of the scene containing the frame to add the
	 * sound to.
	 * @param frame The number of the frame within scene number @c scene to
	 * add the sound to.
	 * @param soundNumber The position of the sound to add. Must be between
	 * 0 and @code{.cpp} SoundCount(scene, frame) @endcode inclusive.
	 * @param sound The name sound to add. Ownership is passed.
	 */
	virtual void addSound(int scene, int frame, int soundNumber,
			Sound* sound) = 0;
	/**
	 * changes the name of a sound of a frame.
	 * @param scene The number of the scene containing the frame to change.
	 * @param frame The number of the frame within scene number @scene
	 * containing the sound to be named.
	 * @param soundNumber Which sound within frame number @frame of scene
	 * number @scene to be replaced.
	 * @param soundName The new name of the sound to be added. Ownership
	 * is passed; must have been created with {@c new char[]}.
	 * @return The old name of the sound. Ownership is returned; must be
	 * deleted with {@c delete[]}.
	 * @throws Nothing.
	 */
	virtual const char* setSoundName(int scene, int frame, int soundNumber,
			const char* soundName) = 0;
	/**
	 * Removes a sound of a frame.
	 * @param scene The number of the scene containing the frame to change.
	 * @param frame The number of the frame within scene number @scene
	 * containing the sound to be changed.
	 * @param soundNumber Which sound within frame number @frame of scene
	 * number @scene to be removed.
	 * @return The file name of the sound that is being removed. Ownership
	 * is returned.
	 * @throws Nothing.
	 */
	virtual Sound* removeSound(int scene, int frame, int soundNumber) = 0;
	virtual void accept(FileNameVisitor& v) const = 0;
};

#endif /* ANIMATIONIMPL_H_ */
