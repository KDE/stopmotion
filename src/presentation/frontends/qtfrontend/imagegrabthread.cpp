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
