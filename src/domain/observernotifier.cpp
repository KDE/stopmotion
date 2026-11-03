/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "observernotifier.h"
#include "animation/animationimpl.h"
#include "animation/frame.h"
#include "animation/scene.h"
#include "animation/sound.h"
#include "animation/workspacefile.h"
#include "src/presentation/observer.h"
#include "src/presentation/frontends/frontend.h"
#include "src/foundation/logger.h"

#include <exception>

class FileNameVisitor;
class Scene;
class Sound;

class ObservableOperation {
public:
	virtual ~ObservableOperation() {
	}
	/**
	 * Performs the operation.
	 */
	virtual void op(AnimationImpl&) = 0;
	/**
	 * Performs the update on the specified observer.
	 */
	virtual void update(Observer&) = 0;
};

void ObserverNotifier::doOp(ObservableOperation& oo) {
	oo.op(*del);
	// exceptions must not escape from here, or we might lose an object that
	// is of importance to the undo system.
	for (Observer* obs : observers) {
		try {
			oo.update(*obs);
		} catch (std::exception& e) {
			try {
				if (frontend)
					frontend->reportWarning(e.what());
			} catch (...) {
			}
			for (Observer* obs2 : observers) {
				obs2->resync();
			}
		}
	}
}

ObserverNotifier::~ObserverNotifier() {
	delete del;
}

ObserverNotifier::ObserverNotifier(AnimationImpl* delegate, Frontend* fe)
		: del(delegate), frontend(fe) {
}

void ObserverNotifier::addObserver(Observer* newObserver) {
	observers.push_back(newObserver);
}

void ObserverNotifier::removeObserver(Observer* o) {
	for (std::vector<Observer*>::iterator i = observers.begin();
			i != observers.end(); ++i) {
		if (*i == o) {
			observers.erase(i);
			return;
		}
	}
	Logger::get().logWarning("Failed to find Observer to remove");
}

class AnimationClearer : public ObservableOperation {
public:
	AnimationClearer() {
	}
	void op(AnimationImpl& del) override {
		del.clear();
	}
	void update(Observer& ob) override {
		ob.updateClear();
	}
};

void ObserverNotifier::clear() {
	AnimationClearer ac;
	doOp(ac);
}

int ObserverNotifier::sceneCount() const {
	return del->sceneCount();
}

class SceneAdder : public ObservableOperation {
	int n;
	std::unique_ptr<Scene>* s;
public:
	/** @param sc The scene to add, or null for a new empty one. */
	SceneAdder(int where, std::unique_ptr<Scene>* sc) : n(where), s(sc) {
	}
	void op(AnimationImpl& del) override {
		if (s)
			del.addScene(n, std::move(*s));
		else
			del.addScene(n);
	}
	void update(Observer& ob) override {
		ob.updateNewScene(n);
	}
};

void ObserverNotifier::addScene(int where, std::unique_ptr<Scene>&& newScene) {
	SceneAdder sa(where, &newScene);
	doOp(sa);
}

void ObserverNotifier::addScene(int where) {
	SceneAdder sa(where, nullptr);
	doOp(sa);
}

void ObserverNotifier::preallocateScenes(int count) {
	del->preallocateScenes(count);
}

class SceneRemover : public ObservableOperation {
	int sc;
public:
	std::unique_ptr<Scene> r;
	explicit SceneRemover(int from) : sc(from) {
	}
	void op(AnimationImpl& del) override {
		r = del.removeScene(sc);
	}
	void update(Observer& ob) override {
		ob.updateRemoveScene(sc);
	}
};

std::unique_ptr<Scene> ObserverNotifier::removeScene(int from) {
	SceneRemover sr(from);
	doOp(sr);
	return std::move(sr.r);
}

class SceneMover : public ObservableOperation {
	int f;
	int t;
public:
	SceneMover(int from, int to) : f(from), t(to) {
	}
	void op(AnimationImpl& del) override {
		del.moveScene(f, t);
	}
	void update(Observer& ob) override {
		ob.updateMoveScene(f, t);
	}
};

void ObserverNotifier::moveScene(int from, int to) {
	SceneMover sm(from, to);
	doOp(sm);
}

const Scene* ObserverNotifier::getScene(int which) const {
	return del->getScene(which);
}

int ObserverNotifier::frameCount(int scene) const {
	return del->frameCount(scene);
}

class FrameAdder : public ObservableOperation {
	int sc;
	int n;
	std::vector<std::unique_ptr<Frame>>& frs;
	int count;
public:
	FrameAdder(int scene, int where, std::vector<std::unique_ptr<Frame>>& frames)
			: sc(scene), n(where), frs(frames), count(frames.size()) {
	}
	~FrameAdder() override {
	}
	void op(AnimationImpl& del) override {
		del.addFrames(sc, n, frs);
	}
	void update(Observer& ob) override {
		ob.updateAdd(sc, n, count);
	}
};

void ObserverNotifier::addFrames(int scene, int where,
		std::vector<std::unique_ptr<Frame>>& frames) {
	FrameAdder fa(scene, where, frames);
	doOp(fa);
}

void ObserverNotifier::preallocateFrames(int scene, int count) {
	del->preallocateFrames(scene, count);
}

class FramesRemover : public ObservableOperation {
	int sc;
	int fr;
	int c;
	std::vector<std::unique_ptr<Frame>>& r;
public:
	FramesRemover(int scene, int where, int count, std::vector<std::unique_ptr<Frame>>& out)
			: sc(scene), fr(where), c(count), r(out) {
	}
	~FramesRemover() override {
	}
	void op(AnimationImpl& del) override {
		del.removeFrames(sc, fr, c, r);
	}
	void update(Observer& ob) override {
		ob.updateRemove(sc, fr, fr + c - 1);
	}
};

void ObserverNotifier::removeFrames(int scene, int frame, int count,
		std::vector<std::unique_ptr<Frame>>& out) {
	FramesRemover fr(scene, frame, count, out);
	doOp(fr);
}

class FrameMover : public ObservableOperation {
	int fromSc;
	int fromFr;
	int c;
	int toSc;
	int toFr;
public:
	FrameMover(int fromScene, int fromFrame, int count,
			int toScene, int toFrame)
			: fromSc(fromScene), fromFr(fromFrame), c(count),
			  toSc(toScene), toFr(toFrame) {
	}
	~FrameMover() override {
	}
	void op(AnimationImpl& del) override {
		del.moveFrames(fromSc, fromFr, c, toSc, toFr);
	}
	void update(Observer& ob) override {
		ob.updateMove(fromSc, fromFr, c, toSc, toFr);
	}
};

void ObserverNotifier::moveFrames(int fromScene, int fromFrame, int frameCount,
		int toScene, int toFrame) {
	FrameMover fm(fromScene, fromFrame, frameCount, toScene, toFrame);
	doOp(fm);
}

class FrameReplacer : public ObservableOperation {
	int sc;
	int fr;
	WorkspaceFile& image;
public:
	FrameReplacer(int scene, int where, WorkspaceFile& newImage)
			: sc(scene), fr(where), image(newImage) {
	}
	~FrameReplacer() override {
	}
	void op(AnimationImpl& del) override {
		if (image.path())
			del.replaceImage(sc, fr, image);
	}
	void update(Observer& ob) override {
		ob.updateAnimationChanged(sc, fr);
	}
};

void ObserverNotifier::replaceImage(int sceneNumber, int frameNumber,
		WorkspaceFile& otherImage) {
	FrameReplacer fr(sceneNumber, frameNumber, otherImage);
	doOp(fr);
}

int ObserverNotifier::soundCount(int scene, int frame) const {
	return del->soundCount(scene, frame);
}

class SoundChanger : public ObservableOperation {
	int sc;
	int fr;
public:
	SoundChanger(int scene, int frame)
			: sc(scene), fr(frame) {
	}
	~SoundChanger() override {
	}
	void op(AnimationImpl&) override {
	}
	void update(Observer& ob) override {
		ob.updateSoundChanged(sc, fr);
	}
	int scene() const {
		return sc;
	}
	int frame() const {
		return fr;
	}
};

class SoundAdder : public SoundChanger {
	int sn;
	std::unique_ptr<Sound>& s;
public:
	SoundAdder(int scene, int frame, int soundNumber,
			std::unique_ptr<Sound>& sound)
			: SoundChanger(scene, frame), sn(soundNumber), s(sound) {
	}
	void op(AnimationImpl& del) override {
		del.addSound(scene(), frame(), sn, std::move(s));
	}
};

void ObserverNotifier::addSound(int scene, int frame, int soundNumber,
		std::unique_ptr<Sound>&& sound) {
	SoundAdder adder(scene, frame, soundNumber, sound);
	doOp(adder);
}

class SoundNamer : public SoundChanger {
	int sn;
	std::string nm;
public:
	SoundNamer(int scene, int frame, int soundNumber, std::string name)
			: SoundChanger(scene, frame), sn(soundNumber), nm(std::move(name)) {
	}
	void op(AnimationImpl& del) override {
		// nm becomes the old name
		nm = del.setSoundName(scene(), frame(), sn, std::move(nm));
	}
	std::string returnValue() {
		return std::move(nm);
	}
};

std::string ObserverNotifier::setSoundName(int scene, int frame,
		int soundNumber, std::string soundName) {
	SoundNamer sn(scene, frame, soundNumber, std::move(soundName));
	doOp(sn);
	return sn.returnValue();
}

class SoundRemover : public SoundChanger {
	int sn;
	std::unique_ptr<Sound> r;
public:
	SoundRemover(int scene, int frame, int soundNumber)
			: SoundChanger(scene, frame), sn(soundNumber) {
	}
	void op(AnimationImpl& del) override {
		r = del.removeSound(scene(), frame(), sn);
	}
	std::unique_ptr<Sound> returnValue() {
		return std::move(r);
	}
};

std::unique_ptr<Sound> ObserverNotifier::removeSound(int scene, int frame,
		int soundNumber) {
	SoundRemover sr(scene, frame, soundNumber);
	doOp(sr);
	return sr.returnValue();
}

void ObserverNotifier::registerFrontend(Frontend* fe) {
	frontend = fe;
}

class AnimationResynchronizer : public ObservableOperation {
public:
	AnimationResynchronizer() {
	}
	void op(AnimationImpl&) override {
	}
	void update(Observer& ob) override {
		ob.resync();
	}
};

void ObserverNotifier::resync() {
	AnimationResynchronizer resyncher;
	doOp(resyncher);
}

int ObserverNotifier::soundCount() const {
	return del->soundCount();
}

void ObserverNotifier::playSounds(int scene, int frame, AudioDriver* audioDriver) const {
	del->playSounds(scene, frame, audioDriver);
}

void ObserverNotifier::accept(FileNameVisitor& v) const {
	del->accept(v);
}
