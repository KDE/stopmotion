/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include <QObject>
#include "src/domain/animation/workspacefile.h"

class FrameView;
class ModelHandler;

class QPushButton;
class QStatusBar;
class QTimer;

/**
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class CameraHandler : public QObject
{
	Q_OBJECT
public:
	/**
	 * Constructor for setting up the camerahandler.
	 * @param parent the parent widget
	 * @param sb the statusbar for updates
	 * @param modelHandler for adding frames when capturing.
	 * @param name the name of the object
	 */
	CameraHandler ( QObject *parent = nullptr, QStatusBar *sb = nullptr,
			ModelHandler* modelHandler = nullptr, const char *name = nullptr);
	
	~CameraHandler() override;

	void setCameraButton( QPushButton *cameraButton );
	void setFrameView(FrameView *frameView);
	
	/**
	 * Sets the viewing mode which is the type of effect used when running the camera.
	 * @param mode the type of effect to be showed on the video. The modes are:
	 *             0: Image mixing/onion skinning
	 *             1: Image differentiating
	 *             2: Playback
	 * @return true if the change was successful
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
	 * Slot to receive a message when the videoView are finished setting up the
	 * camera.
	 */
	void switchToVideoView();
	
private:
	QStatusBar *statusBar;
	QPushButton *cameraButton;
	QTimer *timer;
	WorkspaceFile capturedFile;
	bool isCameraOn;
	ModelHandler *modelHandler;
	
	FrameView *frameView;
	
	void cameraOn();
	void cameraOff();

private slots:
	/**
	 * Slot called by the timer which checks if the frame has been fully captured.
	 */
	void storeFrame();

public slots:
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
