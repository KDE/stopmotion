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

#include <QtTest/QtTest>

#include "testundo.h"
#include "oomtestutil.h"
#include "src/domain/undo/addallcommands.h"
#include "src/domain/undo/executor.h"
#include "src/domain/animation/scenevector.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/sound.h"
#include "src/technical/audio/audioformat.h"

TestStopmotionUndo::TestStopmotionUndo() : sv(0) {
	sv = new SceneVector();
	ex = makeAnimationCommandExecutor(*sv);
	loadOomTestUtil();
}

TestStopmotionUndo::~TestStopmotionUndo() {
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

void TestStopmotionUndo::stopmotionCommandsAreOk() {
	SceneVectorTestHelper helper(*sv);
	testUndo(*ex, helper);
}
