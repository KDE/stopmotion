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

#ifndef OBSERVERNOTIFIER_H_
#define OBSERVERNOTIFIER_H_

#include "src/domain/animation/animationimpl.h"
#include <vector>

class Observer;
class Frontend;
class ObservableOperation;
class AudioDriver;
class FileNameVisitor;
class Frame;
class Scene;
class Sound;
class WorkspaceFile;


class ObserverNotifier : public AnimationImpl {
	AnimationImpl* del;
	Frontend* frontend;
	typedef std::vector<Observer*> observers_t;
	observers_t observers;
	void doOp(ObservableOperation& oo);
public:
	~ObserverNotifier();
	/**
	 * Constructs a wrapper for another {@ref AnimationImpl} that notifies
	 * observers of changes and reports errors to the user.
	 * @param delegate The wrapped {@ref AnimationImpl}. Ownership is passed.
	 * @param frontend The UI, which is notified of errors and warnings.
	 * Ownership is not passed. Optional.
	 */
	ObserverNotifier(AnimationImpl* delegate, Frontend* frontend);
	void addObserver(Observer* newObserver);
	void removeObserver(Observer* o);
	void registerFrontend(Frontend* fe);
	// derived from AnimationImpl
	void clear();
	int sceneCount() const;

	void addScene(int where, Scene* newScene);
	void addScene(int where);
	void preallocateScenes(int count);
	Scene* removeScene(int from);
	void moveScene(int from, int to);
	const Scene* getScene(int which) const;
	int frameCount(int scene) const;
	void addFrame(int scene, int where, Frame* frame);
	void addFrames(int scene, int where,
			const std::vector<Frame*>& frames);
	void preallocateFrames(int scene, int count);
	Frame* removeFrame(int scene, int frame);
	void removeFrames(int scene, int frame, int count,
			std::vector<Frame*>& out);
	void moveFrames(int fromScene, int fromFrame, int frameCount,
			int toScene, int toFrame);
	void replaceImage(int sceneNumber, int frameNumber,
			WorkspaceFile& otherImage);
	int soundCount(int scene, int frame) const;
	int soundCount() const;
	void addSound(int scene, int frame, int soundNumber,
			Sound* sound);
	const char* setSoundName(int scene, int frame, int soundNumber,
			const char* soundName);
	Sound* removeSound(int scene, int frame, int soundNumber);
	void playSounds(int scene, int frame, AudioDriver* audioDriver) const;
	void resync();
	void accept(FileNameVisitor& v) const;
};

#endif /* OBSERVERNOTIFIER_H_ */
