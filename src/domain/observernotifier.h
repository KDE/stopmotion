/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
	using observers_t = std::vector<Observer*>;
	observers_t observers;
	void doOp(ObservableOperation& oo);
public:
	ObserverNotifier(const ObserverNotifier&) = delete;
	ObserverNotifier& operator=(const ObserverNotifier&) = delete;
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
