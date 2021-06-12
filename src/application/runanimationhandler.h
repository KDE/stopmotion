/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
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
#ifndef RUNANIMATIONHANDLER_H
#define RUNANIMATIONHANDLER_H

#include "src/config.h"

#include <QObject>

class Selection;
class QPushButton;
class QTimer;
class QStatusBar;

/**
 * Handles the running of the animation as a sequence of pictures. This is implemented
 * as a timer trigger, to to make it easier to time the fps and, more importantly, to
 * avoid threads.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class RunAnimationHandler : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates the RunAnimationHandler and initializes the member fields.
	 * @param parent the parent of this QOject.
	 * @param sb the statusBar for displaying information to the user
	 * @param name the name of this QObject
	 */
	RunAnimationHandler ( QObject *parent = 0, QStatusBar *sb = 0,
			Selection *selection = 0, const char *name = 0 );


	/**
	 * Stores the playButton so that it can be toggled
	 * @param playButton the button which starts and stops the
	 * animation running. This is needed for toggling the button states.
	 */
	void setPlayButton(QPushButton *playButton);

	/**
	 * Stores the removeFramesButton so that it can be deactivated as needed.
	 * @param removeFramesButton the button for removing frames.
	 * This is needed for canceling this while running the animations to
	 * avoid system crash :D
	 */
	void setRemoveFramesButton(QPushButton *removeFramesButton);

	/**
	 * Sets the loop button.
	 * @param loopButton the button to be used as loop button
	 */
	void setLoopButton(QPushButton *loopButton);

	/**
	 * Sets the pause button.
	 * @param pauseButton the button to be used as pause button
	 */
	void setPauseButton(QPushButton *pauseButton);

signals:

	/**
	 * The animation has reached this frame.
	 */
	void playFrame(int scene, int frame);
	void paused();
	void stopped(int scene, int startFrame, int endFrame);

public slots:

	/**
	 * Starts the animation if it isn't playing and stops it if it is.
	 *
	 * This function is provided for ease of use with keyaccelerators.
	 */
	void toggleRunning();

	/**
	 * Runs the animation.
	 */
	void runAnimation();

	/**
	 * Stops the running of the animation.
	 */
	void stopAnimation();

	/**
	 * Freezes the running of the animation.
	 */
	void pauseAnimation();

	/**
	 * Sets the speed of the animation in frames per second
	 * @param fps the number of frames per second the animation will run at.
	 */
	void setSpeed(int fps);

	/**
	 * Toggles between looping the animation when it is running and closing it
	 * when it reaches the end.
	 */
	void toggleLooping();

private:
	QStatusBar *statusBar;
	Selection *selection;
	QPushButton *playButton;
	QPushButton *removeFramesButton;
	QPushButton *loopButton;
	QPushButton *pauseButton;
	QTimer *timer;
	int sceneNr;
	int frameNr;
	int fps;
	bool isLooping;
	int startFrame;
	int endFrame;

private slots:
	/**
	 * Slot for playing the next frame. This slot is triggered by the timer.
	 */
	void playNextFrame();
	/**
	 * Runs the animation from previously-set current frame
	 */
	void resumeAnimation();
};

#endif
