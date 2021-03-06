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
	~TestStopmotionUndo();
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
