/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
		  playButton(nullptr), removeFramesButton(nullptr), loopButton(nullptr),
		  pauseButton(nullptr), timer(nullptr), sceneNr(0), frameNr(0),
		  fps(0), isLooping(false),
		  startFrame(-1), endFrame(0) {
	fps = PreferencesTool::get()->getPreference("fps", 10);
	timer = new QTimer(this);
	QObject::connect( timer, &QTimer::timeout, this, &RunAnimationHandler::playNextFrame );
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
		QObject::disconnect( playButton, &QAbstractButton::clicked, this, &RunAnimationHandler::runAnimation );
		QObject::connect( playButton, &QAbstractButton::clicked, this, &RunAnimationHandler::pauseAnimation );

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
	int activeFrame = 0;
	if (selection) {
		activeFrame = selection->getActiveFrame();
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
		QObject::disconnect( playButton, &QAbstractButton::clicked, this, &RunAnimationHandler::pauseAnimation );
		QObject::connect(playButton, &QAbstractButton::clicked, this, &RunAnimationHandler::runAnimation);

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
		QObject::disconnect( playButton, &QAbstractButton::clicked, this, &RunAnimationHandler::pauseAnimation );
		QObject::connect(playButton, &QAbstractButton::clicked, this, &RunAnimationHandler::runAnimation);

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
