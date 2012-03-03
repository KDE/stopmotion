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
#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include "src/config.h"

#include "videoview.h"
#include "src/technical/grabber/imagegrabber.h"
#include "src/presentation/observer.h"
#include "src/domain/animation/frame.h"
#include "imagegrabthread.h"
#include "src/domain/domainfacade.h"

#include <qtimer.h>
#include <vector>
#include <qwidget.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


class ImageGrabThread;
struct SDL_Surface;

/**
 * Widget for viewing the frames in the animation using SDL. This widget also 
 * serves as videoview widget for displaying video from an external source
 * by grabbing through the harddrive.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FrameView : public VideoView
{
	Q_OBJECT
public:
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
	
	/**
	 * Function to recieven notification when a frame is added.
	 * @param frames paths to the frames
	 */
	void updateAdd(const vector<char*>& frames, unsigned int, Frontend*);
	
	/**
	 *Function to recieve notification when one or more frames are deleted.
	 */
	void updateRemove(unsigned int, unsigned int);
	
	/**
	 *Function to recieve notification when one or more frames are moved.
	 *
	 */
	void updateMove(unsigned int fromFrame, unsigned int toFrame, unsigned int movePosition);
	
	/**
	 *Function to recieve notification when a new frame is selected.
	 */
	void updateNewActiveFrame(int frameNumber);
	
	/**
	 * Function to receive notification when the model is erased.
	 */
	void updateClear();
	
	/**
	 * Function to recieve notification when a frame is to be played.
	 * @param frameNumber the frame to be played
	 */
	void updatePlayFrame(int frameNumber);
	
	/**
	 * Function to recieve notification when a new scene is added to the
	 * model.
	 * @param index the index of the new scene.
	 */
	void updateNewScene(int index);
	
	/**
	 * Function to recueve notification when a scene is removed from
	 * the model.
	 * @param sceneNumber the scene which has been removed from the model.
	 */
	void updateRemoveScene(int sceneNumber);
	
	/**
	 * Function which recieve notification when a scene in the animation
	 * has been moved.
	 * @param sceneNumber the scene which have been moved.
	 * @param movePosition the position the scene has been moved to.
	 */
	void updateMoveScene(int sceneNumber, int movePosition);
	
	/**
	 * Function which recieves notification when a scene is selected as the
	 * active scene in the animationmodel.
	 * @param sceneNumber the new active scene.
	 * @param frames paths to the pictures in the scene.
	 * @param frontend the frontend for getting a progressbar when adding 
	 * opening the new active scene.
	 */
	void updateNewActiveScene(int sceneNumber, vector<char*> frames,
		Frontend *frontend);
	
	/**
	 * Updates the frameview when an external program has altered the disk files.
	 * @param frameNumber the frame whose disk representation has been changed.
	 */
	void updateAnimationChanged(int frameNumber);
	
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
	 * @param mode the type of effect to be showed on the video. The modes are:
	 *             0: Image mixing/onion skinning
	 *             1: Image differentiating
	 *             2: Playback
	 * @return true if the mode was succesfully changed
	 */
	bool setViewMode(int mode);
	
	/**
	 * Sets the speed for the playback.
	 * @param playbackSpeed the speed to be setted
	 */
	void setPlaybackSpeed(int playbackSpeed);
	
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
	SDL_Surface *screen;
	SDL_Surface *videoSurface;
	
	/** The facade cached away for efficiency reasons */
	DomainFacade *facade;
	
	int widthConst, heightConst;
	bool isPlayingVideo;
	int mode;
	int playbackSpeed;
	char *capturedImg;
	
	ImageGrabThread *grabThread;
	QTimer grabTimer;
	QTimer playbackTimer;
	ImageGrabber *grabber;
	 
	
	/**
	 * Loads the new active frames picture into the frameview.
	 * @param frameNumber 
	 */
	void setActiveFrame(int frameNumber);
	
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
};

#endif
