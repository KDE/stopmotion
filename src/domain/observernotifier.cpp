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
#include "src/presentation/observer.h"
#include "src/presentation/frontends/frontend.h"

#include <memory>

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
	 * @note Exactly one of {@ref update(Observer&)} and
	 * {@ref update(Observer&, Frontend&)} must be overridden.
	 */
	virtual void update(Observer&) {
	}
	/**
	 * Performs the update on the specified observer.
	 * @note Exactly one of {@ref update(Observer&)} and
	 * {@ref update(Observer&, Frontend&)} must be overridden.
	 */
	virtual void update(Observer& ob, Frontend& fe) {
		update(ob);
	}
};

void ObserverNotifier::doOp(ObservableOperation& oo) {
	oo.op(*del);
	try {
		for (std::vector<Observer*>::iterator i = observers.begin();
				i != observers.end(); ++i) {
			try {
				oo.update(**i);
			} catch (std::exception& e) {
				try {
					//TODO: some sort of reset of the Observer?
					if (frontend)
						frontend->reportError(e.what(), 0);
				} catch (...) {
				}
			}
		}
	} catch (...) {
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
	// not found. Fail silently?
}

class AnimationClearer : public ObservableOperation {
public:
	AnimationClearer() {
	}
	void op(AnimationImpl& del) {
		del.clear();
	}
	void update(Observer& ob) {
		ob.updateClear();
	}
};

void ObserverNotifier::clear() {
	del->clear();
}

int ObserverNotifier::sceneCount() const {
	return del->sceneCount();
}

FrameIterator* ObserverNotifier::makeFrameIterator(int scene) const {
	return del->makeFrameIterator(scene);
}

FrameIterator* ObserverNotifier::makeFrameIterator(int scene, int start,
		int end) const {
	return del->makeFrameIterator(scene, start, end);
}

class SceneAdder : public ObservableOperation {
	int n;
	Scene* s;
public:
	SceneAdder(int where, Scene* sc) : n(where), s(sc) {
	}
	void op(AnimationImpl& del) {
		if (s)
			del.addScene(n, s);
		else
			del.addScene(n);
	}
	void update(Observer& ob) {
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
	void op(AnimationImpl& del) {
		r = del.removeScene(sc);
	}
	void update(Observer& ob) {
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
	void op(AnimationImpl& del) {
		del.moveScene(f, t);
	}
	void update(Observer& ob) {
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
	AnimationImpl* ai;
public:
	FrameAdder(int scene, int where, const std::vector<Frame*>& frames)
			: sc(scene), n(where), frs(frames), ai(0) {
	}
	~FrameAdder() {
	}
	void op(AnimationImpl& del) {
		del.addFrames(sc, n, frs);
		ai = &del;
	}
	void update(Observer& ob, Frontend& fe) {
		std::auto_ptr<FrameIterator> fi(ai->makeFrameIterator(sc, n, n + 1));
		ob.updateAdd(*fi, n, &fe);
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
	~FrameRemover() {
		delete r;
	}
	Frame* release() {
		Frame* v = r;
		r = 0;
		return v;
	}
	void op(AnimationImpl& del) {
		del.removeFrame(sc, n);
	}
	void update(Observer& ob) {
		// TODO: set the active scene?
		ob.updateRemove(n, n);
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
	~FramesRemover() {
	}
	void op(AnimationImpl& del) {
		del.removeFrames(sc, fr, c, r);
	}
	void update(Observer& ob) {
		// TODO: set the active scene?
		ob.updateRemove(fr, fr + c - 1);
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
	~FrameMover() {
	}
	void op(AnimationImpl& del) {
		del.moveFrames(fromSc, fromFr, c, toSc, toFr);
	}
	void update(Observer& ob) {
		// TODO: set the active scene?
		ob.updateMove(fromFr, fromFr + c - 1, toFr);
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
	~FrameReplacer() {
	}
	void op(AnimationImpl& del) {
		del.replaceImage(sc, fr, image);
	}
	void update(Observer& ob) {
		// TODO: set the active scene?
		ob.updateAnimationChanged(fr);
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

void ObserverNotifier::addSound(int scene, int frame, int soundNumber,
		Sound* sound) {
	del->addSound(scene, frame, soundNumber, sound);
}

const char* ObserverNotifier::setSoundName(int scene, int frame,
		int soundNumber, const char* soundName) {
	return del->setSoundName(scene, frame, soundNumber, soundName);
}

Sound* ObserverNotifier::removeSound(int scene, int frame, int soundNumber) {
	return del->removeSound(scene, frame, soundNumber);
}

void ObserverNotifier::registerFrontend(Frontend* fe) {
	frontend = fe;
}

void ObserverNotifier::notifyNewActiveScene(int scene) {
	for (std::vector<Observer*>::iterator i = observers.begin();
			i != observers.end(); ++i) {
		std::auto_ptr<FrameIterator> frameIt(
				del->makeFrameIterator(scene));
		(*i)->updateNewActiveScene(scene, *frameIt, frontend);
	}
}

void ObserverNotifier::notifyNewActiveFrame(int frame) {
	for (std::vector<Observer*>::iterator i = observers.begin();
			i != observers.end(); ++i) {
		(*i)->updateNewActiveFrame(frame);
	}
}

void ObserverNotifier::notifyPlayFrame(int frame) {
	for (std::vector<Observer*>::iterator i = observers.begin();
			i != observers.end(); ++i) {
		(*i)->updatePlayFrame(frame);
	}
}

void ObserverNotifier::accept(FileNameVisitor& v) const {
	del->accept(v);
}
