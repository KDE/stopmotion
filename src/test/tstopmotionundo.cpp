/*
 * SPDX-FileCopyrightText: 2013-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
#include <cstdio>
#include <cstring>
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
	~MockFrontend() override {
	}
	int run(int, char **) override {
		return 0;
	}
	void showProgress(ProgressMessage, int) override {
	}
	void hideProgress() override {
	}
	void updateProgress(int) override {
	}
	bool isOperationAborted() override {
		return false;
	}
	void processEvents() override {
	}
	void reportError(const char *, int) {
	}
	bool askQuestion(Question) override {
		return true;
	}
	int runExternalCommand(const char *) override {
		return 0;
	}
	void reportWarning(const char *) override {
	}
	void handleException(UiException&) override {
	}
};

TestStopmotionUndo::TestStopmotionUndo() : anim(nullptr), mockFrontend(nullptr),
		sv(nullptr), ex(nullptr), mfs(nullptr), animTester(nullptr) {
	sv = new SceneVector();
	ex = makeAnimationCommandExecutor(*sv);
	mfs = new RealOggEmptyJpg();
	testEnvFs = new TestHome();
	mockFrontend = new MockFrontend();
	loadOomTestUtil();
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
		void visitImage(const char* s) override {
			h.add("");
			h.add(s);
		}
		void visitSound(const char* s) override {
			h.add(s);
		}
		void reportNewScene() override {
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
		void visitImage(const char* s) override {
			dump.writeIdentifier("frame:");
			dump.writeChar('\n');
			dump.writeString(s);
			dump.writeChar('\n');
		}
		void visitSound(const char* s) override {
			dump.writeString(s);
			dump.writeChar('\n');
		}
		void reportNewScene() override {
			dump.writeIdentifier("scene:");
			dump.writeChar('\n');
		}
	};
public:
	SceneVectorTestHelper(SceneVector& s) : sv(s) {
	}
	~SceneVectorTestHelper() override {
	}
	void resetModel(Executor&) override {
		sv.clear();
	}
	Hash hashModel(const Executor&) override {
		HashingFileNameVisitor v(sv);
		return v.get();
	}
	void dumpModel(std::string& out, const Executor&) override {
		DumpingFileNameVisitor v(sv);
		out = v.get();
	}
};

void TestStopmotionUndo::stopmotionCommandsInvertCorrectly() {
	SceneVectorTestHelper helper(*sv);
	wrapFileSystem(nullptr);
	wrapFileSystem(testEnvFs);
	wrapFileSystem(mfs);
	testUndo(*ex, helper);
	wrapFileSystem(nullptr);
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

TestStopmotionUndo::~TestStopmotionUndo() {
	delete animTester;
	delete anim;
	wrapFileSystem(nullptr);
	delete mockFrontend;
	delete testEnvFs;
	delete mfs;
	delete ex;
	delete sv;
}

class StringContainer : public StringIterator {
	using vect_t = std::vector<const char*>;
	vect_t v;
	vect_t::size_type i;
public:
	StringContainer() : i(0) {
	}
	~StringContainer() override {
	}
	int count() override {
		return static_cast<int>(v.size()) - i;
	}
	bool atEnd() const override {
		return v.size() == i;
	}
	const char* get() const override {
		return v[i];
	}
	void next() override {
		++i;
	}
	void add(const char* s) {
		v.insert(v.end(), s);
	}
};

void TestStopmotionUndo::setUpAnim() {
	wrapFileSystem(nullptr);
	wrapFileSystem(testEnvFs);
	WorkspaceFile::clear();
	delete anim;
	anim = nullptr;
	delete animTester;
	animTester = nullptr;
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
