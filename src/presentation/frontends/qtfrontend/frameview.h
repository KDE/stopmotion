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
#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include "src/presentation/observer.h"
#include "src/presentation/imagecache.h"

#include <QWidget>
#include <QTimer>

class ImageGrabThread;
struct SDL_Surface;
class QResizeEvent;
class QPaintEvent;
class ImageGrabThread;
class ImageGrabber;
class DomainFacade;

/**
 * Widget for viewing the frames in the animation using SDL. This widget also
 * serves as videoview widget for displaying video from an external source
 * by grabbing through the harddrive.
 *
 * Note: I'm considering redesigning the entire framework around this class, both
 * to make it more intuitive and to work with dynamic plugins for filters such
 * as onionskinning, diffing, you name it! (plugins are cool) =) However this is
 * not very important and is left for a weekend where i'm bored :p
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FrameView : public QWidget, public ActiveFrameObserver
{
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
	* the playback mode is choosen
	*/
	FrameView(QWidget *parent=0, const char *name=0, int playbackSpeed = 10);

	/**
	* Cleans up after the frameview.
	*/
	~FrameView();

	/**
	 * Sets the view to 4:3 format.
	 */
	void setWidescreenRatio();

	/**
	 * Sets the view to 16:9 format.
	 */
	void setNormalRatio();

	void initCompleted();

	/**
	 * Receives notification when a new frame is selected.
	 */
	void updateNewActiveFrame(int scene, int frame);

	/**
	 * Receives notification when a frame is to be played.
	 */
	void updatePlayFrame(int scene, int frame);

	/**
	 * Turns on the webcamera/video import mode.
	 */
	bool on();

	/**
	 * Turns off the webcamera/video import mode.
	 */
	void off();

	/**
	 * Sets the viewing mode/type of effect used when displaying the video.
	 * @param mode the type of effect to be showed on the video. The modes are:\n
	 *             0: Image mixing/onion skinning\n
	 *             1: Image differentiating\n
	 *             2: Playback\n
	 * @return true if the mode was succesfully changed
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
	 * @param playbackSpeed the speed to be setted
	 */
	void setPlaybackSpeed(int playbackSpeed);

signals:
	void cameraReady();

public slots:
	/**
	 * Draws the next frame from the camera.
	 */
	void redraw();

	/**
	 * Function for performing playbacks. Will call redraw with regular intervals.
	 */
	void nextPlayBack();

protected:
	void resizeEvent(QResizeEvent *);
	void paintEvent(QPaintEvent *);

private:
	static const int alphaLut[5];

	SDL_Surface *screen;
	SDL_Surface *videoSurface;
	ImageCache imageCache;

	QTimer grabTimer;
	QTimer playbackTimer;
	ImageGrabThread *grabThread;
	ImageGrabber *grabber;
	char *capturedImg;

	/** The facade cached away in this class for efficiency reasons */
	DomainFacade *facade;

	/** Is the frame view showing the camera output? */
	bool isPlayingVideo;

	int widthConst, heightConst;
	ImageMode mode;
	int playbackSpeed;
	int activeScene;
	int activeFrame;
	int mixCount;

	/**
	 * Loads the new active frames picture into the frameview.
	 * @param sceneNumber The scene that the active frame belongs to.
	 * @param frameNumber The frame within the scene that is to be active.
	 */
	void setActiveFrame(int sceneNumber, int frameNumber);

	/**
	 * Highly tweaked/optimized homemade function for taking the rgb differences
	 * between two surfaces.
	 *
	 * deltaRed = abs(r1 - r2), deltaGreen = abs(g1 - g2), deltaBlue = abs(b1 - b2)
	 * for all pixels.
	 *
	 * @param s1 the first surface of the two to differentiate.
	 * @param s2 the second surface of the two to differentiate.
	 * @return a surface with the rgb difference of s1 and s2.
	 */
	SDL_Surface* differentiateSurfaces(SDL_Surface *s1, SDL_Surface *s2);
	void freeProperty(const char *prop, const char *tag = "");
};

#endif
