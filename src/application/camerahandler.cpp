/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
#include "stopmotion_debug.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/uiexception.h"
#include "src/presentation/frontends/qtfrontend/frameview.h"


CameraHandler::CameraHandler ( QObject *parent, QStatusBar *sb, 
		ModelHandler* modelHandler, const char *name) 
	: QObject(parent), statusBar(sb), cameraButton(nullptr), timer(nullptr),
	  capturedFile(WorkspaceFile::capturedImage), isCameraOn(false),
	  modelHandler(modelHandler), frameView(nullptr) {
	timer = new QTimer(this);
	timer->setSingleShot(true);
	QObject::connect( timer, &QTimer::timeout, this, &CameraHandler::storeFrame );
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
		Q_EMIT cameraStateChanged(true);
	}
	DomainFacade::getFacade()->getFrontend()->hideProgress();
}


void CameraHandler::cameraOff()
{
	cameraButton->setIcon( QPixmap(cameraon) );
	frameView->off();
	isCameraOn = false;
	Q_EMIT cameraStateChanged(false);
}


void CameraHandler::toggleCamera()
{
	if(isCameraOn) {
		cameraOff();
	} else {
		qCDebug(STOPMOTION_LOG, "Playing video from webcam");
		cameraOn();
	}
}


void CameraHandler::captureFrame()
{
	qCDebug(STOPMOTION_LOG, "Capturing image from webcam");
	timer->start(60);
}


void CameraHandler::storeFrame()
{
	QImage i;
	const char* path = capturedFile.path();
	i.load(QString::fromLocal8Bit(path));
	if ( !i.isNull() ) {
		modelHandler->addFrame(QString::fromLocal8Bit(path));
		Q_EMIT capturedFrame();
	} else {
		timer->start(60);
	}
}


void CameraHandler::switchToVideoView()
{
	cameraButton->setEnabled(true);
	Q_EMIT cameraStateChanged(true);
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
