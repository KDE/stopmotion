/***************************************************************************
 *   Copyright (C) 2005-2017 by Linuxstopmotion contributors;              *
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
#include "camerahandler.h"

#include <QTimer>
#include <QPushButton>
#include <QPixmap>

#include "modelhandler.h"
#include "workspacefile.h"
#include "frontends/frontend.h"
#include "graphics/icons/cameraoff.xpm"
#include "graphics/icons/cameraon.xpm"
#include "src/domain/domainfacade.h"
#include "src/foundation/logger.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/uiexception.h"
#include "src/presentation/frontends/qtfrontend/frameview.h"


CameraHandler::CameraHandler ( QObject *parent, QStatusBar *sb, 
		ModelHandler* modelHandler, const char *name) 
	: QObject(parent), statusBar(sb), cameraButton(0), timer(0),
	  capturedFile(WorkspaceFile::capturedImage), isCameraOn(false),
	  modelHandler(modelHandler), frameView(0) {
	timer = new QTimer(this);
	timer->setSingleShot(true);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(storeFrame()) );
	setObjectName(name);
}


CameraHandler::~CameraHandler( ) {
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
	return frameView->setViewMode((FrameView::ImageMode) mode);
}


void CameraHandler::cameraOn()
{
	DomainFacade::getFacade()->getFrontend()->showProgress(
			Frontend::connectingCamera);
	cameraButton->setIcon( QPixmap(cameraoff) );
	isCameraOn = frameView->on();
	if (!isCameraOn) {
		cameraOff();
	} else {
		emit cameraStateChanged(true);
	}
	DomainFacade::getFacade()->getFrontend()->hideProgress();
}


void CameraHandler::cameraOff()
{
	cameraButton->setIcon( QPixmap(cameraon) );
	frameView->off();
	isCameraOn = false;
	emit cameraStateChanged(false);
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
	const char* path = capturedFile.path();
	i.load(path);
	if ( !i.isNull() ) {
		modelHandler->addFrame(path);
		emit capturedFrame();
	} else {
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
	
	PreferencesTool* preferences = PreferencesTool::get();
	switch( frameView->getViewMode() ) {
		case 0: 
		{
			preferences->setPreference("mixcount", mixCount);
			break;
		}
		case 2:
		{
			preferences->setPreference("playbackcount", mixCount);
			break;
		}
	}
	try {
		preferences->flush();
	} catch (UiException& ex) {
		DomainFacade::getFacade()->getFrontend()->handleException(ex);
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
