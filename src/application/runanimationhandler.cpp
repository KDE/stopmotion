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
#include "runanimationhandler.h"

#include "src/foundation/preferencestool.h"
#include "src/domain/domainfacade.h"


RunAnimationHandler::RunAnimationHandler ( QObject *parent, QStatusBar *sb, 
		const char *name ) 
		: QObject(parent, name), statusBar(sb)
{
	playButton         = NULL;
	pauseButton        = NULL;
	removeFramesButton = NULL;
	loopButton         = NULL;
	timer              = NULL;
	
	fps = PreferencesTool::get()->getPreference("fps", 10);
	
	frameNr = 0;
	isLooping = false;
	
	timer = new QTimer(this);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(playNextFrame()) );
}


void RunAnimationHandler::setPlayButton( QPushButton * playButton )
{
	this->playButton = playButton;
}


void RunAnimationHandler::setRemoveFramesButton( QPushButton * removeFramesButton )
{
	this->removeFramesButton = removeFramesButton;
}


void RunAnimationHandler::setLoopButton( QPushButton * loopButton )
{
	this->loopButton = loopButton;
}


void RunAnimationHandler::toggleRunning()
{
	if(timer->isActive()) {
		stopAnimation();
	}
	else {
		runAnimation();
	}
}


void RunAnimationHandler::runAnimation()
{
	DomainFacade *f = DomainFacade::getFacade();
	if (f->getActiveSceneNumber() >= 0) {
		if (f->getSceneSize(f->getActiveSceneNumber()) > 0) {
			f->initAudioDevice();
			QObject::disconnect( playButton, SIGNAL(clicked()), this, SLOT(runAnimation()) );
			QObject::connect( playButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()) );
			
			playButton->setToggleButton(true);
			playButton->toggle();
			removeFramesButton->setEnabled(false);
			frameNr = f->getActiveFrameNumber();
			statusBar->message( tr("Running animation"), 2000 );
			timer->start( 1000/fps, false );
		}
	}
}


void RunAnimationHandler::stopAnimation()
{
	if ( timer->isActive() ) {
 		QObject::disconnect( playButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()) );
 		QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(runAnimation()));
		
		if ( playButton->isOn() ) {
			playButton->toggle();
		}
		
		playButton->setToggleButton(false);
		removeFramesButton->setEnabled(true);
		
		DomainFacade *f = DomainFacade::getFacade();
		f->setActiveFrame( frameNr );
		f->shutdownAudioDevice();
		
		statusBar->clear();
		timer->stop();
		f->setActiveFrame(0);
	}
}


void RunAnimationHandler::setPauseButton(QPushButton * pauseButton)
{
	this->pauseButton = pauseButton;
}


void RunAnimationHandler::pauseAnimation()
{
	if ( timer->isActive() ) {
		QObject::disconnect( playButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()) );
		QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(runAnimation()));
		
		if ( playButton->isOn() ) {
			playButton->toggle();
		}
		
		playButton->setToggleButton(false);
		removeFramesButton->setEnabled(true);
		
		DomainFacade *f = DomainFacade::getFacade();
		f->setActiveFrame( frameNr );
		f->shutdownAudioDevice();
		
		statusBar->clear();
		timer->stop();
	}
	
}


void RunAnimationHandler::selectPreviousFrame()
{
	int afn = DomainFacade::getFacade()->getActiveFrameNumber();
	if (afn > 0) {
		DomainFacade::getFacade()->setActiveFrame(afn-1);
	}
}


void RunAnimationHandler::selectNextFrame()
{
	int afn = DomainFacade::getFacade()->getActiveFrameNumber();
	if (afn > -1 && afn < (int)DomainFacade::getFacade()->
			getSceneSize(DomainFacade::getFacade()->
			getActiveSceneNumber()) - 1) {
		DomainFacade::getFacade()->setActiveFrame(afn+1);
	}
}


void RunAnimationHandler::selectPreviousScene()
{
	int asn = DomainFacade::getFacade()->getActiveSceneNumber();
	if (asn > 0) {
		DomainFacade::getFacade()->setActiveScene(asn-1);
	}
}


void RunAnimationHandler::selectNextScene()
{
	int asn = DomainFacade::getFacade()->getActiveSceneNumber();
	if (asn > -1 && asn < (int)DomainFacade::getFacade()->
			getNumberOfScenes() -1) {
		DomainFacade::getFacade()->setActiveScene(asn+1);
	}
}


void RunAnimationHandler::setSpeed(int fps)
{
	this->fps = fps;
	if ( timer->isActive() ) {
		timer->changeInterval(1000/this->fps);
	}
	
	//Adding the fps to the preferencestool.
	PreferencesTool::get()->setPreference("fps", fps);
}


void RunAnimationHandler::toggleLooping()
{
	isLooping = !isLooping;
}


void RunAnimationHandler::playNextFrame()
{
	DomainFacade *facade = DomainFacade::getFacade();
	
	if (facade->getActiveSceneNumber() >= 0) {
		facade->playFrame( frameNr );
		
		if (isLooping) {
			frameNr = (frameNr < facade->getSceneSize(
						facade->getActiveSceneNumber()) - 1) 
						? frameNr+1 : 0;
		}
		else {
			if( frameNr < facade->getSceneSize(
					facade->getActiveSceneNumber()) - 1 ) {
				++frameNr;
			}
			else {
				this->stopAnimation();
			}
		}
	}
	else {
		stopAnimation();
	}
}
