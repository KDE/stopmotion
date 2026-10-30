/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
	void run() override;
	
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
