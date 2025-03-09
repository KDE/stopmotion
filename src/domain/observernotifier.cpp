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

#include "observernotifier.h"
#include "animation/animationimpl.h"
#include "animation/frame.h"
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
	Scene* s;
public:
	SceneAdder(int where, Scene* sc) : n(where), s(sc) {
	}
	void op(AnimationImpl& del) override {
		if (s)
			del.addScene(n, s);
		else
			del.addScene(n);
	}
	void update(Observer& ob) override {
		ob.updateNewScene(n);
	}
};

void ObserverNotifier::addScene(int where, Scene* newScene) {
	SceneAdder sa(where, newScene);
	doOp(sa);
}

void ObserverNotifier::addScene(int where) {
	SceneAdder sa(where, 0);
	doOp(sa);
}

void ObserverNotifier::preallocateScenes(int count) {
	del->preallocateScenes(count);
}

class SceneRemover : public ObservableOperation {
	int sc;
public:
	Scene* r;
	SceneRemover(int from) : sc(from), r(0) {
	}
	void op(AnimationImpl& del) override {
		r = del.removeScene(sc);
	}
	void update(Observer& ob) override {
		ob.updateRemoveScene(sc);
	}
};

Scene* ObserverNotifier::removeScene(int from) {
	SceneRemover sr(from);
	doOp(sr);
	return sr.r;
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
	const std::vector<Frame*>& frs;
public:
	FrameAdder(int scene, int where, const std::vector<Frame*>& frames)
			: sc(scene), n(where), frs(frames) {
	}
	~FrameAdder() override {
	}
	void op(AnimationImpl& del) override {
		del.addFrames(sc, n, frs);
	}
	void update(Observer& ob) override {
		int count = frs.size();
		ob.updateAdd(sc, n, count);
	}
};

void ObserverNotifier::addFrame(int scene, int where, Frame* frame) {
	std::vector<Frame*> frs;
	frs.push_back(frame);
	FrameAdder fa(scene, where, frs);
	doOp(fa);
}

void ObserverNotifier::addFrames(int scene, int where,
		const std::vector<Frame*>& frames) {
	FrameAdder fa(scene, where, frames);
	doOp(fa);
}

void ObserverNotifier::preallocateFrames(int scene, int count) {
	del->preallocateFrames(scene, count);
}

class FrameRemover : public ObservableOperation {
	int sc;
	int n;
	Frame* r;
public:
	FrameRemover(int scene, int where)
			: sc(scene), n(where), r(0) {
	}
	~FrameRemover() override {
		delete r;
	}
	Frame* release() {
		Frame* v = r;
		r = 0;
		return v;
	}
	void op(AnimationImpl& del) override {
		del.removeFrame(sc, n);
	}
	void update(Observer& ob) override {
		ob.updateRemove(sc, n, n);
	}
};

Frame* ObserverNotifier::removeFrame(int scene, int frame) {
	FrameRemover fr(scene, frame);
	doOp(fr);
	return fr.release();
}

class FramesRemover : public ObservableOperation {
	int sc;
	int fr;
	int c;
	std::vector<Frame*>& r;
public:
	FramesRemover(int scene, int where, int count, std::vector<Frame*>& out)
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
		std::vector<Frame*>& out) {
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
	Sound* s;
public:
	SoundAdder(int scene, int frame, int soundNumber, Sound* sound)
			: SoundChanger(scene, frame), sn(soundNumber), s(sound) {
	}
	void op(AnimationImpl& del) override {
		del.addSound(scene(), frame(), sn, s);
	}
};

void ObserverNotifier::addSound(int scene, int frame, int soundNumber,
		Sound* sound) {
	SoundAdder adder(scene, frame, soundNumber, sound);
	doOp(adder);
}

class SoundNamer : public SoundChanger {
	int sn;
	const char* nm;
	const char* r;
public:
	SoundNamer(int scene, int frame, int soundNumber, const char* name)
			: SoundChanger(scene, frame), sn(soundNumber), nm(name), r(0) {
	}
	void op(AnimationImpl& del) override {
		r = del.setSoundName(scene(), frame(), sn, nm);
	}
	const char* returnValue() const {
		return r;
	}
};

const char* ObserverNotifier::setSoundName(int scene, int frame,
		int soundNumber, const char* soundName) {
	SoundNamer sn(scene, frame, soundNumber, soundName);
	doOp(sn);
	return sn.returnValue();
}

class SoundRemover : public SoundChanger {
	int sn;
	Sound* r;
public:
	SoundRemover(int scene, int frame, int soundNumber)
			: SoundChanger(scene, frame), sn(soundNumber), r(0) {
	}
	void op(AnimationImpl& del) override {
		r = del.removeSound(scene(), frame(), sn);
	}
	Sound* returnValue() const {
		return r;
	}
};

Sound* ObserverNotifier::removeSound(int scene, int frame, int soundNumber) {
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
