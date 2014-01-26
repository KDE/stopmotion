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

#include "tstopmotionundo.h"

#include "testundo.h"
#include "oomtestutil.h"
#include "src/domain/undo/addallcommands.h"
#include "src/domain/undo/executor.h"
#include "src/domain/animation/animation.h"
#include "src/domain/animation/scenevector.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/sound.h"
#include "src/technical/audio/audioformat.h"

#include <QtTest/QtTest>

#include <stdlib.h>
#include <error.h>

class RealOggEmptyJpg : public MockableFileSystem {
	MockableFileSystem* delegate;
	FILE* fake;
	int fakeReads;
public:
	RealOggEmptyJpg() : delegate(0), fake(reinterpret_cast<FILE*>(1)),
			fakeReads(0) {
	}
	~RealOggEmptyJpg() {
	}
	bool hasExtension(const char* filename, const char* extension) {
		const char* dot = strrchr(filename, '.');
		return dot && strcmp(dot, extension) == 0;
	}
	bool isSound(const char* filename) {
		return hasExtension(filename, ".ogg");
	}
	bool isImage(const char* filename) {
		return hasExtension(filename, ".jpg");
	}
	void setDelegate(MockableFileSystem* mfs) {
		delegate = mfs;
	}
	FILE* openFake(const char* filename, const char* mode) {
		if (isImage(filename)) {
			fakeReads = 0;
			return fake;
		} else if (isSound(filename)) {
			if (strstr(mode, "w") == 0)
				return delegate->fopen("resources/click.ogg", mode);
			return fake;
		}
		return 0;
	}
	FILE* fopen(const char* filename, const char* mode) {
		FILE* r = openFake(filename, mode);
		if (r)
			return r;
		return delegate->fopen(filename, mode);
	}
	FILE* freopen(const char* filename, const char* mode, FILE* fh) {
		if (fh == fake) {
			if (filename)
				return fopen(filename, mode);
			fakeReads = 0;
			return fake;
		}
		return delegate->freopen(filename, mode, fh);
	}
	int fclose(FILE* fh) {
		if (fh == fake)
			return 0;
		return delegate->fclose(fh);
	}
	int fflush(FILE* fh) {
		if (fh == fake)
			return 0;
		return delegate->fflush(fh);
	}
	size_t fread (void *out, size_t blockSize,
			     size_t blockCount, FILE *fh) {
		if (fh == fake) {
			if (0 < fakeReads)
				return 0;
			++fakeReads;
			return blockCount;
		}
		return delegate->fread(out, blockSize, blockCount, fh);
	}
	size_t fwrite (const void *in, size_t blockSize,
			      size_t blockCount, FILE *fh) {
		if (fh == fake)
			return blockCount;
		return delegate->fwrite(in, blockSize, blockCount, fh);
	}
	int access (const char *name, int /*type*/) {
		// always assume files within the workspace do not exist
		// (as access is only called to find empty slots to use in the
		// workspace) but files requested outside of the workspace exist
		return strstr(name, ".stopmotion/tmp")? -1 : 0;
	}
	int ferror(FILE*) {
		return 0;
	}
	int unlink(const char *) {
		return 0;
	}
	int ov_test(FILE *, OggVorbis_File *, const char *, long) {
		return 0;
	}
	int ov_clear(OggVorbis_File *) {
		return 0;
	}
	int ov_open(FILE *,OggVorbis_File *,const char *, long) {
		return 0;
	}
	long ov_read(OggVorbis_File *,char *,int, int, int, int, int *) {
		return 0;
	}
	char *getenv(const char *name) {
		return delegate->getenv(name);
	}
};

class TestHome : public MockableFileSystem {
	MockableFileSystem* delegate;
	char* fakeHome;
public:
	TestHome() : delegate(0), fakeHome(0) {
	}
	~TestHome() {
	}
	void setDelegate(MockableFileSystem* mfs) {
		delegate = mfs;
		char* home = delegate->getenv("HOME");
		if (home) {
			static const char appendix[] = "/.stopmotion/test";
			unsigned int len = strlen(home);
			fakeHome = (char*) malloc(len + sizeof(appendix));
			if (fakeHome) {
				strncpy(fakeHome, home, len);
				strncpy(fakeHome + len, appendix, sizeof(appendix));
			}
		}
	}
	FILE* fopen(const char* filename, const char* mode) {
		return delegate->fopen(filename, mode);
	}
	FILE* freopen(const char* filename, const char* mode, FILE* fh) {
		return delegate->freopen(filename, mode, fh);
	}
	int fclose(FILE* fh) {
		return delegate->fclose(fh);
	}
	int fflush(FILE* fh) {
		return delegate->fflush(fh);
	}
	size_t fread (void *out, size_t blockSize,
			     size_t blockCount, FILE *fh) {
		return delegate->fread(out, blockSize, blockCount, fh);
	}
	size_t fwrite (const void *in, size_t blockSize,
			      size_t blockCount, FILE *fh) {
		return delegate->fwrite(in, blockSize, blockCount, fh);
	}
	int access (const char *name, int type) {
		return delegate->access(name, type);
	}
	int ferror(FILE* fh) {
		return delegate->ferror(fh);
	}
	int unlink(const char *name) {
		return delegate->unlink(name);
	}
	int ov_test(FILE *f, OggVorbis_File *vf, const char *initial, long ibytes) {
		return delegate->ov_test(f, vf, initial, ibytes);
	}
	int ov_clear(OggVorbis_File *vf) {
		return delegate->ov_clear(vf);
	}
	int ov_open(FILE *f, OggVorbis_File *vf, const char *initial, long ibytes) {
		return delegate->ov_open(f, vf, initial, ibytes);
	}
	long ov_read(OggVorbis_File *vf,char *buffer, int length, int bigendianp,
			int word, int sgned, int *bitstream) {
		return delegate->ov_read(vf, buffer, length, bigendianp, word, sgned,
				bitstream);
	}
	char *getenv(const char *name) {
		if (0 == strcmp(name, "HOME")) {
			return fakeHome;
		}
		return delegate->getenv(name);
	}
};

TestStopmotionUndo::TestStopmotionUndo() : anim(0), sv(0), ex(0), mfs(0) {
	anim = new Animation();
	sv = new SceneVector();
	ex = makeAnimationCommandExecutor(*sv);
	mfs = new RealOggEmptyJpg();
	testEnvFs = new TestHome();
	loadOomTestUtil();
}

TestStopmotionUndo::~TestStopmotionUndo() {
	setMockFileSystem(0);
}

class SceneVectorTestHelper : public ModelTestHelper {
	SceneVector& sv;
public:
	SceneVectorTestHelper(SceneVector& s) : sv(s) {
	}
	~SceneVectorTestHelper() {
	}
	void resetModel(Executor&) {
		sv.clear();
	}
	Hash hashModel(const Executor&) {
		Hash h;
		// soundCount is kept as a separate variable so we hash this as well
		// so that we can be sure that it is kept in sync with the actual
		// number of sounds.
		h.add(sv.soundCount());
		int sceneCount = sv.sceneCount();
		for (int s = 0; s != sceneCount; ++s) {
			const Scene *scene = sv.getScene(s);
			int frameCount = scene->getSize();
			for (int f = 0; f != frameCount; ++f) {
				const Frame* frame = scene->getFrame(f);
				h.add(frame->getImagePath());
				int soundCount = frame->soundCount();
				for (int snd = 0; snd != soundCount; ++snd) {
					const Sound* sound = frame->getSound(snd);
					h.add(sound->getName());
					h.add(sound->getAudio()->getSoundPath());
				}
			}
		}
		return h;
	}
};

void TestStopmotionUndo::stopmotionCommandsInvertCorrectly() {
	SceneVectorTestHelper helper(*sv);
	setMockFileSystem(mfs);
	testUndo(*ex, helper);
	setMockFileSystem(0);
}

void TestStopmotionUndo::addFrames() {
}

void TestStopmotionUndo::removeFrames() {
}

void TestStopmotionUndo::moveFrames() {
}

void TestStopmotionUndo::setImagePath() {
}

void TestStopmotionUndo::duplicateImage() {
}

void TestStopmotionUndo::addSound() {
}

void TestStopmotionUndo::removeSound() {
}

void TestStopmotionUndo::setSoundName() {
}
