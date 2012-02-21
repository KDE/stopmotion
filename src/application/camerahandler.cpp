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
#include "src/application/camerahandler.h"

#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "graphics/icons/cameraoff.xpm"
#include "graphics/icons/cameraon.xpm"

#include <QImage>
#include <QInputDialog>
#include <QWhatsThis>
#include <QPixmap>

#include <vector>


CameraHandler::CameraHandler ( QObject *parent, QStatusBar *sb, 
		ModelHandler* modelHandler, const char *name) 
	: QObject(parent), statusBar(sb), modelHandler(modelHandler)
{
	cameraButton = 0;
	frameView = 0;

	isCameraOn = false;
	sprintf(temp, "%s/.stopmotion/capturedfile.jpg", getenv("HOME") ); 
	
	timer = new QTimer(this);
	timer->setSingleShot(true);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(storeFrame()) );
	setObjectName(name);
}


CameraHandler::~CameraHandler( )
{

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
	return frameView->setViewMode(mode);
}


void CameraHandler::cameraOn()
{
	DomainFacade::getFacade()->getFrontend()->showProgress("Connecting camera... ");
	cameraButton->setIcon( QPixmap(cameraoff) );
	isCameraOn = frameView->on();
	if (!isCameraOn) {
		cameraOff();
	}
	DomainFacade::getFacade()->getFrontend()->hideProgress();
}


void CameraHandler::cameraOff()
{
	cameraButton->setIcon( QPixmap(cameraon) );
	emit cameraStateChanged(false);
	frameView->off();
	isCameraOn = false;
	DomainFacade::getFacade()->setActiveFrame(DomainFacade::getFacade()->getActiveFrameNumber());
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
	timer->start(60);
}


void CameraHandler::storeFrame()
{
	QImage i;
	i.load(temp);
	if ( !i.isNull() ) {
		modelHandler->addFrame(temp);
		if (DomainFacade::getFacade()->getActiveFrameNumber() == 0) {
			emit capturedFrame();
		}
	}
	else {
		timer->start(60);
	}
}


void CameraHandler::switchToVideoView()
{
	cameraButton->setEnabled(true);
	emit cameraStateChanged(true);
	DomainFacade::getFacade()->getFrontend()->hideProgress();
}


void CameraHandler::setMixCount(int mixCount)
{
	frameView->setMixCount(mixCount);
	
	switch( frameView->getViewMode() ) {
		case 0: 
		{
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
	frameView->setPlaybackSpeed(playBackSpeed);
}
