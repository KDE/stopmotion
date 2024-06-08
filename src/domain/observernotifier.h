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
	~ObserverNotifier() override;
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
	void clear() override;
	int sceneCount() const override;

	void addScene(int where, Scene* newScene) override;
	void addScene(int where) override;
	void preallocateScenes(int count) override;
	Scene* removeScene(int from) override;
	void moveScene(int from, int to) override;
	const Scene* getScene(int which) const override;
	int frameCount(int scene) const override;
	void addFrame(int scene, int where, Frame* frame) override;
	void addFrames(int scene, int where,
			const std::vector<Frame*>& frames) override;
	void preallocateFrames(int scene, int count) override;
	Frame* removeFrame(int scene, int frame) override;
	void removeFrames(int scene, int frame, int count,
			std::vector<Frame*>& out) override;
	void moveFrames(int fromScene, int fromFrame, int frameCount,
			int toScene, int toFrame) override;
	void replaceImage(int sceneNumber, int frameNumber,
			WorkspaceFile& otherImage) override;
	int soundCount(int scene, int frame) const override;
	int soundCount() const override;
	void addSound(int scene, int frame, int soundNumber,
			Sound* sound) override;
	const char* setSoundName(int scene, int frame, int soundNumber,
			const char* soundName) override;
	Sound* removeSound(int scene, int frame, int soundNumber) override;
	void playSounds(int scene, int frame, AudioDriver* audioDriver) const override;
	void resync();
	void accept(FileNameVisitor& v) const override;
};

#endif /* OBSERVERNOTIFIER_H_ */
