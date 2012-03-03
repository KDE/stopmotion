/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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
#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include "src/config.h"

#include "src/presentation/observer.h"

#include <qwidget.h>

/**
 * Abstract class for the widgets who displays video to the users. 
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class VideoView : public QWidget, public Observer
{
	Q_OBJECT
public:
	
	/**
	 * Initializes the videoview and register local attributes.
	 * @param parent the parent of the widget.
	 * @param name the name of the widget.
	 */
	VideoView(QWidget *parent=0, const char *name=0, WFlags f = 0);
	
	/**
	 * Cleans up after the widget
	 */
	~VideoView();
	
	virtual bool on();
	virtual void off();
	virtual void capture();
	
	virtual void setPlaybackSpeed(int playbackSpeed);
	
	/**
	 * Function which is called by the initThread when it is done running.
	 * This function then signals the cameraHandler who again can adjust the 
	 * various buttons accordingly.
	 */
	virtual void initCompleted();
	
	
	/**
	 *Function to recieven notification when a frame is added.
	 *
	 */
	virtual void updateAdd(const vector<char*>& frames, unsigned int, Frontend*);
	
	/**
	 *Function to recieve notification when one or more frames are deleted.
	 *@param frameNumber the frame to remove
	 */
	virtual void updateRemove(unsigned int, unsigned int);
	
	/**
	 *Function to recieve notification when one or more frames are moved.
	 *
	 */
	virtual void updateMove(unsigned int fromFrame, unsigned int toFrame, unsigned int movePosition);
	
	
	/**
	 *Function to recieve notification when a new frame is selected.
	 */
	virtual void updateNewActiveFrame(int frameNumber);
	
	/**
	 * Function to receive notification when the model is erased.
	 */
	virtual void updateClear();
	
	/**
	 * Function to recieve notification when a frame is to be played.
	 * @param frameNumber the frame to be played
	 */
	virtual void updatePlayFrame(int frameNumber);
	
	/**
	 * Function to recieve notification when a new scene is added to the
	 * model.
	 * @param index the index of the new scene.
	 */
	virtual void updateNewScene(int index);
	
	/**
	 * Function to recueve notification when a scene is removed from
	 * the model.
	 * @param sceneNumber the scene which has been removed from the model.
	 */
	virtual void updateRemoveScene(int sceneNumber);
	
	/**
	 * Function which recieve notification when a scene in the animation
	 * has been moved.
	 * @param sceneNumber the scene which have been moved.
	 * @param movePosition the position the scene has been moved to.
	 */
	virtual void updateMoveScene(int sceneNumber, int movePosition);
	
	virtual void updateNewActiveScene(int sceneNumber, vector<char*> frames,
		Frontend *frontend);
	
	virtual void updateAnimationChanged(vector<Frame*>& frames);
	
	void setMixCount(int mixCount);


	virtual bool setViewMode(int mode);
	virtual int getViewMode();

protected:
	/**Number of images backwards which should be mixed onto the camera*/
	int mixCount;	
	
	virtual void initCamera();
	
private:


signals:
	void cameraReady();
};

#endif
