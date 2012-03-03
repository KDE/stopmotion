/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include "src/config.h"

#include "src/presentation/frontends/qtfrontend/videoview.h"
#include "src/presentation/frontends/qtfrontend/frameview.h"
// #include "camerainitthread.h"

#include <qobject.h>
#include <qpushbutton.h>
#include <qstatusbar.h>
#include <qwidgetstack.h>
#include <qtimer.h>


/**
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class CameraHandler : public QObject
{
	Q_OBJECT
public:
	CameraHandler ( QObject *parent = 0, QStatusBar *sb = 0, 
			const char *name = 0 );
	
	~CameraHandler();

	void setCameraButton( QPushButton *cameraButton );
	void setWidgetStack(QWidgetStack *frameViewStack);
	void setFrameView(FrameView *frameView);
	void setVideoView(VideoView *videoView);
	
	/**
	 * Sets the viewing mode/type of effect used when displaying the video.
	 * @param mode the type of effect to be showed on the video. The modes are:
	 *             0: Image mixing/onion skinning
	 *             1: Image differentiating
	 *             2: Playback
	 * @return true if the mode was succesfully changed
	 */
	bool setViewMode(int mode);
	
	/**
	 * Checks if the camera is on.
	 * @return true if the camera is on.
	 */
	bool isCameraRunning();

public slots:
	void toggleCamera();
	void captureFrame();
	
	/**
	 * Slot to recieve a message when the videoView are finished seting up the
	 * camera.
	 */
	void switchToVideoView();
	
	
private:
	QStatusBar *statusBar;
	QPushButton *cameraButton;
	QTimer *timer;
	char temp[256];
	bool isCameraOn;
	
	QWidgetStack *frameViewStack;
	VideoView *videoView;
	FrameView *frameView;
	
	void cameraOn();
	void cameraOff();

private slots:
	/**
	 * Slot called by the timer which checks if the frame has been fully captured.
	 */
	void storeFrame();
	
	void setMixCount(int mixCount);
	
	void setPlaybackSpeed(int playBackSpeed);
	
signals:
	void capturedFrame();
	
	/**
	 * This signal is emitted when the state of the camera is changed.
	 * @param isActivated true if the camera has been activated.
	 */
	void cameraStateChanged(bool isActivated);
	
};

#endif
