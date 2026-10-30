/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TSTOPMOTIONUNDO_H_
#define TSTOPMOTIONUNDO_H_

#include <QObject>

class Animation;
class Executor;
class SceneVector;
class RealOggEmptyJpg;
class TestHome;
class AnimTester;
class MockFrontend;

class TestStopmotionUndo : public QObject {
	Q_OBJECT
	Animation* anim;
	MockFrontend* mockFrontend;
	SceneVector* sv;
	Executor* ex;
	RealOggEmptyJpg* mfs;
	TestHome* testEnvFs;
	AnimTester* animTester;
	void setUpAnim();
public:
	TestStopmotionUndo();
	~TestStopmotionUndo() override;
private slots:
	void newScene();
	void removeScene();
	void moveScene();
	void addFrames();
	void removeFrames();
	void moveFrames();
	void setImagePath();
	void duplicateImage();
	void addSound();
	void removeSound();
	void setSoundName();
	void stopmotionCommandsInvertCorrectly();
};

#endif /* TSTOPMOTIONUNDO_H_ */
