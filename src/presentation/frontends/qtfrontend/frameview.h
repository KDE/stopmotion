/***************************************************************************
 *   Copyright (C) 2005-2016 by Linuxstopmotion contributors;              *
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
#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include <QObject>
#include <QTimer>
#include <QWidget>

#include "src/domain/animation/workspacefile.h"
#include "src/presentation/imagecache.h"

class ImageGrabThread;
class QResizeEvent;
class QPaintEvent;
class QPixmap;
class ImageGrabber;
class DomainFacade;

/**
 * Widget for viewing the frames in the animation and image about to be
 * captured.
 */
class FrameView : public QWidget {
	Q_OBJECT
public:
	enum ImageMode {
		imageModeMix,
		imageModeDiff,
		imageModePlayback
	};
	/**
	* Creates and initializes the frameview.
	* @param parent the parent widget.
	* @param name the name of this widget.
	* @param playbackSpeed which speed the playback has to be played in if
	* the playback mode is chosen
	*/
	FrameView(QWidget *parent=0, const char *name=0, int playbackSpeed = 10);

	/**
	* Cleans up after the frameview.
	*/
	~FrameView();

	void initCompleted();

	/**
	 * Sets the viewing mode/type of effect used when displaying the video.
	 * @param mode the type of effect to be showed on the video. The modes are:\n
	 *             0: Image mixing/onion skinning\n
	 *             1: Image differentiating\n
	 *             2: Playback\n
	 * @return true if the mode was successfully changed
	 */
	bool setViewMode(ImageMode mode);

	void setMixCount(int mixCount);

	/**
	 * Returns the view mode.
	 * @return the view mode.
	 */
	int getViewMode() const;

	/**
	 * Sets the speed for the playback.
	 * @param playbackSpeed the speed to be set.
	 */
	void setPlaybackSpeed(int playbackSpeed);

	/**
	 * Gets the speed for playback.
	 * @returns Frames per second.
	 */
	int getPlaybackSpeed() const;

signals:
	void cameraReady();

public slots:
	/**
	 * Turns on the webcamera/video import mode.
	 */
	bool on();

	/**
	 * Turns off the webcamera/video import mode.
	 */
	void off();

	/**
	 * Draws the next frame from the camera.
	 */
	void redraw();

	/**
	 * Function for performing playbacks. Will call redraw with regular intervals.
	 */
	void nextPlayBack();

	/**
	 * Receives notification when a new frame is selected.
	 */
	void updateNewActiveFrame(int scene, int frame);

	/**
	 * Receives notification when a frame is to be played.
	 */
	void updatePlayFrame(int scene, int frame);

	/**
	 * Receives notification when a frame has been edited outside of this
	 * application.
	 */
	void fileChanged(const QString &path);

	/**
	 * Receives notification that the files in the workspace have changed.
	 */
	void workspaceCleared();

protected:
	void resizeEvent(QResizeEvent *);
	void paintEvent(QPaintEvent *);

private:
	static const int alphaLut[5];

	QPixmap cameraOutput;
	ImageCache imageCache;

	QTimer grabTimer;
	QTimer playbackTimer;
	ImageGrabThread *grabThread;
	ImageGrabber *grabber;
	WorkspaceFile capturedFile;

	/** The facade cached away in this class for efficiency reasons */
	DomainFacade *facade;

	/** Is the frame view showing the camera output? */
	bool isPlayingVideo;

	ImageMode mode;
	int playbackSpeed;
	int activeScene;
	int activeFrame;
	int mixCount;
	int playbackModeFrame;

	/**
	 * Loads the new active frames picture into the frameview.
	 * @param sceneNumber The scene that the active frame belongs to.
	 * @param frameNumber The frame within the scene that is to be active.
	 */
	void setActiveFrame(int sceneNumber, int frameNumber);

	/**
	 * Draws the appropriate blend of frames and camera output based on the
	 * current image mode.
	 */
	void drawOnionSkins();
};

#endif
