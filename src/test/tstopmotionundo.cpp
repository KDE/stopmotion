/***************************************************************************
 *   Copyright (C) 2013-2014 by Linuxstopmotion contributors;              *
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

#include "tstopmotionundo.h"

#include "testundo.h"
#include "testhome.h"
#include "fakefiles.h"
#include "oomtestutil.h"
#include "hash.h"
#include "src/domain/undo/addallcommands.h"
#include "src/domain/undo/executor.h"
#include "src/domain/animation/animation.h"
#include "src/domain/animation/scenevector.h"
#include "src/domain/animation/workspacefile.h"
#include "src/domain/filenamevisitor.h"
#include "src/presentation/frontends/frontend.h"
#include "src/technical/stringiterator.h"
#include "src/foundation/stringwriter.h"

#include <QTest>


#include <algorithm>
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

class UiException;

void getHashes(std::vector<Hash>& out, const char* filenameTemplate) {
	std::string filenameStr(filenameTemplate);
	char* filename = &filenameStr[0];
	char* digitPtr = strchr(filename, '?');
	for (int i = 0; i != 10; ++i) {
		*digitPtr = '0' + i;
		if (0 != access(filename, F_OK))
			return;
		out.resize(i + 1);
		Hash h;
		FILE* f = fopen(filename, "r");
		h.add(f);
		fclose(f);
		out[i] = h;
	}
}

class MockFrontend : public Frontend {
public:
	~MockFrontend() {
	}
	int run(int, char **) {
		return 0;
	}
	void showProgress(ProgressMessage, int) {
	}
	void hideProgress() {
	}
	void updateProgress(int) {
	}
	bool isOperationAborted() {
		return false;
	}
	void processEvents() {
	}
	void reportError(const char *, int) {
	}
	bool askQuestion(Question) {
		return true;
	}
	int runExternalCommand(const char *) {
		return 0;
	}
	void reportWarning(const char *) {
	}
	void handleException(UiException&) {
	}
};

TestStopmotionUndo::TestStopmotionUndo() : anim(0), mockFrontend(0),
		sv(0), ex(0), mfs(0), animTester(0) {
	sv = new SceneVector();
	ex = makeAnimationCommandExecutor(*sv);
	mfs = new RealOggEmptyJpg();
	testEnvFs = new TestHome();
	mockFrontend = new MockFrontend();
	loadOomTestUtil();
}

TestStopmotionUndo::~TestStopmotionUndo() {
	wrapFileSystem(0);
	delete mockFrontend;
	delete testEnvFs;
	delete mfs;
	delete ex;
	delete sv;
}

class SceneVectorTestHelper : public ModelTestHelper {
	SceneVector& sv;
	class HashingFileNameVisitor : public FileNameVisitor {
		Hash h;
	public:
		HashingFileNameVisitor(SceneVector& sv) {
			sv.accept(*this);
		}
		Hash get() const {
			return h;
		}
		void visitImage(const char* s) {
			h.add("");
			h.add(s);
		}
		void visitSound(const char* s) {
			h.add(s);
		}
		void reportNewScene() {
			h.add("");
		}
	};
	class DumpingFileNameVisitor : public FileNameVisitor {
		StringWriter dump;
	public:
		DumpingFileNameVisitor(SceneVector& sv) {
			sv.accept(*this);
		}
		const char* get() const {
			return dump.result();
		}
		void visitImage(const char* s) {
			dump.writeIdentifier("frame:");
			dump.writeChar('\n');
			dump.writeString(s);
			dump.writeChar('\n');
		}
		void visitSound(const char* s) {
			dump.writeString(s);
			dump.writeChar('\n');
		}
		void reportNewScene() {
			dump.writeIdentifier("scene:");
			dump.writeChar('\n');
		}
	};
public:
	SceneVectorTestHelper(SceneVector& s) : sv(s) {
	}
	~SceneVectorTestHelper() {
	}
	void resetModel(Executor&) {
		sv.clear();
	}
	Hash hashModel(const Executor&) {
		HashingFileNameVisitor v(sv);
		return v.get();
	}
	void dumpModel(std::string& out, const Executor&) {
		DumpingFileNameVisitor v(sv);
		out = v.get();
	}
};

void TestStopmotionUndo::stopmotionCommandsInvertCorrectly() {
	SceneVectorTestHelper helper(*sv);
	wrapFileSystem(0);
	wrapFileSystem(testEnvFs);
	wrapFileSystem(mfs);
	testUndo(*ex, helper);
	wrapFileSystem(0);
}

class AnimTester {
	const Animation* anim;
	std::vector<Hash> imageHashes;
	std::vector<Hash> soundHashes;
	int scene;
	int frame;
	int sound;
	int soundCount;
	bool seenFrame;
	void frameEnd() {
		QCOMPARE(sound, anim->soundCount(scene, frame));
		sound = 0;
		++frame;
	}
	void sceneEnd() {
		if (seenFrame) {
			frameEnd();
			seenFrame = false;
		}
		QCOMPARE(frame, anim->frameCount(scene));
		frame = 0;
		++scene;
	}
	void testFrame(int n) {
		seenFrame = true;
		const char* path = anim->getImagePath(scene, frame);
		FILE* fh = fopen(path, "r");
		if (!fh) {
			char msg[500];
			snprintf(msg, sizeof(msg), "Failed to open file %s", path);
			QFAIL(msg);
		}
		Hash h;
		h.add(fh);
		fclose(fh);
		QCOMPARE(imageHashes[n], h);
	}
	void testSound(int n) {
		const char* path = anim->getSoundPath(scene, frame, sound - 1);
		FILE* fh = fopen(path, "r");
		Hash h;
		h.add(fh);
		fclose(fh);
		QCOMPARE(soundHashes[n], h);
	}
public:
	AnimTester(const Animation* a) : anim(a), scene(0),
			frame(0), sound(0), soundCount(0), seenFrame(false) {
		getHashes(imageHashes, "resources/frame?.jpg");
		getHashes(soundHashes, "resources/sound?.ogg");
	}
	~AnimTester() {
	}
	void test(const char* expected) {
		scene = 0;
		frame = 0;
		sound = 0;
		soundCount = 0;
		seenFrame = false;
		for (; *expected; ++expected) {
			switch (*expected) {
			case ';':
				sceneEnd();
				break;
			case ',':
				frameEnd();
				break;
			case '/':
				++sound;
				break;
			default:
				int n = *expected - '0';
				QVERIFY(0 <= n && n <= 9);
				if (sound == 0) {
					testFrame(n);
				} else {
					testSound(n);
					++soundCount;
				}
				break;
			}
		}
		sceneEnd();
		QCOMPARE(soundCount, anim->soundCount());
	}
};

class StringContainer : public StringIterator {
	typedef std::vector<const char*> vect_t;
	vect_t v;
	vect_t::size_type i;
public:
	StringContainer() : i(0) {
	}
	~StringContainer() {
	}
	int count() {
		return static_cast<int>(v.size()) - i;
	}
	bool atEnd() const {
		return v.size() == i;
	}
	const char* get() const {
		return v[i];
	}
	void next() {
		++i;
	}
	void add(const char* s) {
		v.insert(v.end(), s);
	}
};

void TestStopmotionUndo::setUpAnim() {
	wrapFileSystem(0);
	wrapFileSystem(testEnvFs);
	WorkspaceFile::clear();
	delete anim;
	anim = 0;
	delete animTester;
	animTester = 0;
	anim = new Animation();
	anim->registerFrontend(mockFrontend);
	animTester = new AnimTester(anim);
	anim->newScene(0);
	anim->newScene(1);
	anim->newScene(2);
	StringContainer frames0;
	frames0.add("resources/frame0.jpg");
	frames0.add("resources/frame1.jpg");
	anim->addFrames(0, 0, frames0);
	StringContainer frames1;
	frames1.add("resources/frame2.jpg");
	anim->addFrames(1, 0, frames1);
	StringContainer frames2;
	frames2.add("resources/frame3.jpg");
	frames2.add("resources/frame4.jpg");
	anim->addFrames(2, 0, frames2);
	anim->addSound(2, 0, "resources/sound0.ogg");
	anim->addSound(2, 1, "resources/sound1.ogg");
}

void TestStopmotionUndo::addFrames() {
	setUpAnim();
	StringContainer newFrames;
	newFrames.add("resources/frame5.jpg");
	newFrames.add("resources/frame6.jpg");
	anim->addFrames(0, 1, newFrames);
	animTester->test("0,5,6,1;2;3/0,4/1");
}

void TestStopmotionUndo::removeFrames() {
	setUpAnim();
	anim->removeFrames(0, 0, 1);
	animTester->test("1;2;3/0,4/1");
	anim->removeFrames(2, 1, 1);
	animTester->test("1;2;3/0");
}

void TestStopmotionUndo::moveFrames() {
	setUpAnim();
	anim->moveFrames(2, 0, 2, 0, 1);
	animTester->test("0,3/0,4/1,1;2;");
}

void TestStopmotionUndo::setImagePath() {
	setUpAnim();
	std::string oldPath = anim->getImagePath(2, 1);
	anim->setImagePath(2, 1, "resources/frame5.jpg");
	QVERIFY(oldPath != anim->getImagePath(2, 1));
	animTester->test("0,1;2;3/0,5/1");
}

void TestStopmotionUndo::duplicateImage() {
	setUpAnim();
	std::string oldPath = anim->getImagePath(2, 1);
	anim->duplicateImage(2, 1);
	QVERIFY(oldPath != anim->getImagePath(2, 1));
	animTester->test("0,1;2;3/0,4/1");
}

void TestStopmotionUndo::addSound() {
	setUpAnim();
	anim->addSound(2, 0, "resources/sound2.ogg");
	animTester->test("0,1;2;3/0/2,4/1");
}

void TestStopmotionUndo::removeSound() {
	setUpAnim();
	anim->addSound(2, 0, "resources/sound2.ogg");
	anim->removeSound(2, 0, 0);
	animTester->test("0,1;2;3/2,4/1");
	anim->removeSound(2, 0, 0);
	animTester->test("0,1;2;3,4/1");
}

void TestStopmotionUndo::setSoundName() {
	setUpAnim();
	anim->setSoundName(2, 0, 0, "tommy");
	animTester->test("0,1;2;3/0,4/1");
	QCOMPARE(anim->getSoundName(2, 0, 0), "tommy");
}

void TestStopmotionUndo::newScene() {
	setUpAnim();
	anim->newScene(1);
	animTester->test("0,1;;2;3/0,4/1");
}

void TestStopmotionUndo::removeScene() {
	setUpAnim();
	anim->removeScene(1);
	animTester->test("0,1;3/0,4/1");
	anim->removeScene(1);
	animTester->test("0,1");
}

void TestStopmotionUndo::moveScene() {
	setUpAnim();
	anim->moveScene(2, 0);
	animTester->test("3/0,4/1;0,1;2");
}
