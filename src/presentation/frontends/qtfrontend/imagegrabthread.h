/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
#ifndef IMAGEGRABTHREAD_H
#define IMAGEGRABTHREAD_H

#include <QThread>

class FrameView;
class ImageGrabber;

/**
 * Thread used for polling an external program to update the camera. Only
 * used for polling, not for when the camera is running in daemon mode.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ImageGrabThread : public QThread
{
	Q_OBJECT
public:
	/**
	 * Constructs and initializes the object.
	 * @param frameView the frame view to be used for displaying images
	 * @param grabber the grabber to use be used for grabbing images
	 */
	ImageGrabThread(FrameView *frameView, ImageGrabber *grabber);
		
	/**
	 * Displays the images grabbed with the registered grabber. These
	 * are displayed one after one and it looks like we are streaming
	 * live from the camera.
	 */
	void run();
	
	/**
	 * Checks if last grabbing was successful.
	 * @return true if last grabbing was success, false otherwise
	 */
	bool wasGrabbingSuccess();

signals:
	void grabbed();

private:
	FrameView *frameView;
	ImageGrabber *grabber;
	bool wasGrabSuccess;
};

#endif
