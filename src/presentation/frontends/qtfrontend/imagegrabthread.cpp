/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "imagegrabthread.h"

#include "technical/grabber/imagegrabber.h"


ImageGrabThread::ImageGrabThread(FrameView *frameView, ImageGrabber *grabber)
		: frameView(frameView), grabber(grabber)
{
	wasGrabSuccess = true;
}


void ImageGrabThread::run()
{
	//Don't want the frameview to update unless the grabbing worked.
	while (true) {
		wasGrabSuccess = grabber->grab();
		if (!wasGrabSuccess) {
			break;
		}
		emit grabbed();
	}
}


bool ImageGrabThread::wasGrabbingSuccess()
{
	return wasGrabSuccess;
}
