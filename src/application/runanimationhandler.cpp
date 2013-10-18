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
#include "src/application/runanimationhandler.h"

#include "src/foundation/preferencestool.h"
#include "src/domain/domainfacade.h"
#include "src/presentation/observer.h"

#include <qpushbutton.h>
#include <qtimer.h>
#include <qstatusbar.h>

RunAnimationHandler::RunAnimationHandler ( QObject *parent, QStatusBar *sb,
		Selection *sel, const char *name )
		: QObject(parent), statusBar(sb), selection(sel),
		  playButton(0), pauseButton(0), removeFramesButton(0),
		  loopButton(0), timer(0), sceneNr(0), frameNr(0),
		  fps(0), isLooping(false),
		  startFrame(0), endFrame(0), observer(0) {
	fps = PreferencesTool::get()->getPreference("fps", 10);
	timer = new QTimer(this);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(playNextFrame()) );
	setObjectName(name);
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


void RunAnimationHandler::toggleRunning() {
	if(timer->isActive()) {
		stopAnimation();
	}
	else {
		resumeAnimation();
	}
}

void RunAnimationHandler::resumeAnimation() {
	DomainFacade *f = DomainFacade::getFacade();
	if (0 <= sceneNr && sceneNr < f->getNumberOfScenes()
			&& f->getSceneSize(sceneNr) > 0) {
		f->initAudioDevice();
		QObject::disconnect( playButton, SIGNAL(clicked()), this, SLOT(runAnimation()) );
		QObject::connect( playButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()) );

		//playButton->setToggleButton(true);
		playButton->setChecked(true);
		playButton->toggle();
		removeFramesButton->setEnabled(false);
		statusBar->showMessage( tr("Running animation"), 2000 );
		timer->start( 1000/fps);
		timer->setSingleShot(false);
	}
}

void RunAnimationHandler::runAnimation() {
	sceneNr = 0;
	startFrame = 0;
	endFrame = 0;
	if (selection) {
		sceneNr = selection->getActiveScene();
		startFrame = selection->getActiveFrame();
		if (selection->isSelecting()) {
			int sel = selection->getSelectionFrame();
			if (startFrame < sel) {
				endFrame = sel;
			} else {
				endFrame = startFrame;
				startFrame = sel;
			}
		}
	}
	if (endFrame == 0) {
		endFrame = DomainFacade::getFacade()->getSceneSize(sceneNr);
	}
	frameNr = startFrame;
	resumeAnimation();
}


void RunAnimationHandler::stopAnimation() {
	if ( timer->isActive() ) {
 		QObject::disconnect( playButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()) );
 		QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(runAnimation()));

		if ( playButton->isChecked() ) {
			playButton->toggle();
		}

		playButton->setChecked(false);
		removeFramesButton->setEnabled(true);

		DomainFacade *f = DomainFacade::getFacade();
		if (observer)
			observer->updateNewActiveFrame(sceneNr, frameNr);
		f->shutdownAudioDevice();

		statusBar->clearMessage();
		timer->stop();
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

		if ( playButton->isChecked() ) {
			playButton->toggle();
		}

		playButton->setChecked(false);
		removeFramesButton->setEnabled(true);

		if (observer)
			observer->updateNewActiveFrame(sceneNr, frameNr);
		DomainFacade *f = DomainFacade::getFacade();
		f->shutdownAudioDevice();

		statusBar->clearMessage();
		timer->stop();
	}

}


void RunAnimationHandler::selectPreviousFrame() {
	if (0 < frameNr) {
		--frameNr;
		if (observer)
			observer->updateNewActiveFrame(sceneNr, frameNr);
	}
}


void RunAnimationHandler::selectNextFrame() {
	int sceneSize = DomainFacade::getFacade()->getSceneSize(frameNr);
	if (0 <= frameNr && frameNr + 1 < sceneSize) {
		++frameNr;
		if (observer)
			observer->updateNewActiveFrame(sceneNr, frameNr);
	}
}


void RunAnimationHandler::selectPreviousScene() {
	if (0 <= frameNr && 0 < sceneNr) {
		--sceneNr;
		frameNr = 0;
		if (observer)
			observer->updateNewActiveFrame(sceneNr, frameNr);
	}
}


void RunAnimationHandler::selectNextScene() {
	int sceneCount = DomainFacade::getFacade()->getNumberOfScenes();
	if (0 <= sceneNr && sceneNr + 1 < sceneCount) {
		++sceneNr;
		frameNr = 0;
		if (observer)
			observer->updateNewActiveFrame(sceneNr, frameNr);
	}
}


void RunAnimationHandler::setSpeed(int fps)
{
	this->fps = fps;
	if ( timer->isActive() ) {
		timer->setInterval(1000/this->fps);
	}

	//Adding the fps to the preferencestool.
	PreferencesTool::get()->setPreference("fps", fps);
}


void RunAnimationHandler::toggleLooping()
{
	isLooping = !isLooping;
}


void RunAnimationHandler::playNextFrame() {
	DomainFacade *facade = DomainFacade::getFacade();
	if (sceneNr >= 0) {
		if (observer)
			observer->updatePlayFrame(sceneNr, frameNr);
		++frameNr;
		if (frameNr < facade->getSceneSize(sceneNr))
			return;
		if (isLooping) {
			frameNr = 0;
			return;
		}
	}
	stopAnimation();
}

void RunAnimationHandler::setObserver(ActiveFrameObserver* ob) {
	observer = ob;
}
