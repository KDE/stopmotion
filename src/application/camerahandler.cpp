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
#include "camerahandler.h"

#include "src/domain/domainfacade.h"
// #include "src/presentation/frontends/qtfrontend/gstvideoview.h"
#include "src/foundation/preferencestool.h"

#include "graphics/icons/cameraoff.xpm"
#include "graphics/icons/cameraon.xpm"

#include <qimage.h>
#include <qinputdialog.h>
#include <qwhatsthis.h>
#include <vector>



CameraHandler::CameraHandler ( QObject *parent, QStatusBar *sb, 
		ModelHandler* modelHandler, const char *name) 
		: QObject(parent, name), statusBar(sb), modelHandler(modelHandler)
{
	cameraButton = NULL;
	frameViewStack = NULL;
	videoView = NULL;
	frameView = NULL;
	timer     = NULL;

	isCameraOn = false;
	sprintf(temp, "%s/.stopmotion/capturedfile.jpg", getenv("HOME") ); 
	
	timer = new QTimer(this);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(storeFrame()) );
}


/**
 *@todo add cleanup code for gstreamer components
 */
CameraHandler::~CameraHandler( )
{

}


void CameraHandler::setWidgetStack(QWidgetStack *frameViewStack)
{
	this->frameViewStack = frameViewStack;
}


void CameraHandler::setVideoView(VideoView *videoView)
{
	this->videoView = videoView;
}


void CameraHandler::setFrameView( FrameView * frameView )
{
	this->frameView = frameView;
}


void CameraHandler::setCameraButton( QPushButton *cameraButton )
{
	this->cameraButton = cameraButton;
}


bool CameraHandler::setViewMode(int mode)
{
	return videoView->setViewMode(mode);
}


void CameraHandler::cameraOn()
{
	cameraButton->setPixmap( QPixmap(cameraoff) );
	DomainFacade::getFacade()->getFrontend()->showProgress("Connecting camera... ");
	cameraButton->setEnabled(false);
	frameViewStack->raiseWidget(videoView);
	isCameraOn = videoView->on();
	if (isCameraOn == false) {
		cameraOff();
	}
}


void CameraHandler::cameraOff()
{
	cameraButton->setPixmap( QPixmap(cameraon) );
	DomainFacade::getFacade()->attatch(frameView);
	frameViewStack->raiseWidget(frameView);
	emit cameraStateChanged(false);
	videoView->off();
	isCameraOn = false;
	DomainFacade::getFacade()->setActiveFrame(
			DomainFacade::getFacade()->getActiveFrameNumber());
}


void CameraHandler::toggleCamera()
{
	if(isCameraOn == false) {
		Logger::get().logDebug("Playing video from webcam");
		cameraOn();
	}
	else {
		cameraOff();
	}
}


void CameraHandler::captureFrame()
{
	Logger::get().logDebug("Capturing image from webcam");
	//videoView->capture();
	timer->start(50, true);
}


void CameraHandler::storeFrame()
{
	QImage i;
	i.load(temp);
	if(i.isNull() == false) {
		//std::vector<char*> frame;
		//frame.push_back(temp);
		
		modelHandler->addFrame(temp);
		
		//DomainFacade::getFacade()->addFrames(frame);
		
		if(DomainFacade::getFacade()->getActiveFrameNumber() == 0) {
			emit capturedFrame();
		}
		videoView->capture();
	}
	else {
		timer->start(50, true);
	}
}


void CameraHandler::switchToVideoView()
{
	cameraButton->setEnabled(true);
	
	if(frameView != NULL) {
		DomainFacade::getFacade()->detatch(frameView);
	}
	
	emit cameraStateChanged(true);
	DomainFacade::getFacade()->getFrontend()->hideProgress();
}


void CameraHandler::setMixCount(int mixCount)
{
	videoView->setMixCount(mixCount);
	
	//Storing the new mixcount in the PreferencesTool
	switch(videoView->getViewMode()) 
	{
		case 0: 
		{
// 			cout << mixCount << endl;
			PreferencesTool::get()->setPreference("mixcount", mixCount);
			break;
		}
		case 2:
		{
			PreferencesTool::get()->setPreference("playbackcount", mixCount);
			break;
		}
	}
}


bool CameraHandler::isCameraRunning()
{
	return isCameraOn;
}


void CameraHandler::setPlaybackSpeed(int playBackSpeed)
{
	videoView->setPlaybackSpeed(playBackSpeed);
}
