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
#include "src/application/runanimationhandler.h"

#include <algorithm>
#include <QPushButton>
#include <QTimer>
#include <QStatusBar>

#include "src/foundation/preferencestool.h"
#include "src/foundation/uiexception.h"
#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/selection.h"
#include "src/presentation/frontends/frontend.h"

RunAnimationHandler::RunAnimationHandler(QObject *parent, QStatusBar *sb,
		Selection *sel, const char *name )
		: QObject(parent), statusBar(sb), selection(sel),
		  playButton(0), removeFramesButton(0), loopButton(0),
		  pauseButton(0), timer(0), sceneNr(0), frameNr(0),
		  fps(0), isLooping(false),
		  startFrame(-1), endFrame(0) {
	fps = PreferencesTool::get()->getPreference("fps", 10);
	timer = new QTimer(this);
	QObject::connect( timer, SIGNAL(timeout()), this, SLOT(playNextFrame()) );
	setObjectName(name);
}


void RunAnimationHandler::setPlayButton( QPushButton * playButton ) {
	this->playButton = playButton;
}


void RunAnimationHandler::setRemoveFramesButton(
		QPushButton * removeFramesButton) {
	this->removeFramesButton = removeFramesButton;
}


void RunAnimationHandler::setLoopButton(QPushButton * loopButton) {
	this->loopButton = loopButton;
}


void RunAnimationHandler::toggleRunning() {
	if(timer->isActive()) {
		stopAnimation();
	} else if (startFrame < 0) {
		runAnimation();
	} else {
		resumeAnimation();
	}
}

void RunAnimationHandler::resumeAnimation() {
	DomainFacade *f = DomainFacade::getFacade();
	int sceneSize = f->getSceneSize(sceneNr);
	if (0 <= sceneNr && sceneNr < f->getNumberOfScenes() && sceneSize > 0) {
		if (sceneSize < endFrame)
			endFrame = sceneSize;
		if (endFrame <= startFrame) {
			startFrame = endFrame;
			stopAnimation();
		}
		if (endFrame <= frameNr)
			frameNr = startFrame;
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
	int activeFrame = selection->getActiveFrame();
	if (selection) {
		sceneNr = selection->getActiveScene();
		startFrame = activeFrame;
		int sel = selection->getSelectionAnchor();
		if (startFrame < sel) {
			endFrame = sel + 1;
		} else {
			endFrame = startFrame + 1;
			startFrame = sel;
		}
	}
	if (endFrame - startFrame <= 1) {
		// only one or zero frames selected. Play the entire scene from the
		// selected frame.
		startFrame = std::max(activeFrame, 0);
		endFrame = DomainFacade::getFacade()->getSceneSize(sceneNr) ;
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
		f->shutdownAudioDevice();

		statusBar->clearMessage();
		timer->stop();
		if (startFrame < endFrame)
			emit stopped(sceneNr, startFrame, endFrame - 1);
		startFrame = -1;
	}
}


void RunAnimationHandler::setPauseButton(QPushButton * pauseButton) {
	this->pauseButton = pauseButton;
}


void RunAnimationHandler::pauseAnimation() {
	if ( timer->isActive() ) {
		QObject::disconnect( playButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()) );
		QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(runAnimation()));

		playButton->setChecked(false);
		removeFramesButton->setEnabled(true);

		emit paused();
		DomainFacade *f = DomainFacade::getFacade();
		f->shutdownAudioDevice();

		statusBar->clearMessage();
		timer->stop();
	}

}


void RunAnimationHandler::setSpeed(int fps) {
	this->fps = fps;
	if ( timer->isActive() ) {
		timer->setInterval(1000/this->fps);
	}
	PreferencesTool* preferences = PreferencesTool::get();
	preferences->setPreference("fps", fps);
	try {
		preferences->flush();
	} catch (UiException& ex) {
		DomainFacade::getFacade()->getFrontend()->handleException(ex);
	}
}


void RunAnimationHandler::toggleLooping() {
	isLooping = !isLooping;
}


void RunAnimationHandler::playNextFrame() {
	DomainFacade *facade = DomainFacade::getFacade();
	if (sceneNr >= 0) {
		emit playFrame(sceneNr, frameNr);
		facade->playSounds(sceneNr, frameNr);
		++frameNr;
		int sceneSize = facade->getSceneSize(sceneNr);
		if (sceneSize < endFrame)
			endFrame = sceneSize;
		if (frameNr < endFrame)
			return;
		if (isLooping) {
			frameNr = startFrame;
			return;
		}
	}
	stopAnimation();
}
