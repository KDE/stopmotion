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
#include "toolsmenu.h"
#include "ui_toolsmenu.h"

#include "src/foundation/preferencestool.h"
#include "graphics/icons/play.xpm"
#include "graphics/icons/fastforward.xpm"
#include "graphics/icons/rewind.xpm"
#include "graphics/icons/steptoend.xpm"
#include "graphics/icons/steptobeginning.xpm"
#include "graphics/icons/stop.xpm"
#include "graphics/icons/pause.xpm"
#include "graphics/icons/loop.xpm"
#include "graphics/icons/capture.xpm"
#include "graphics/icons/cameraon.xpm"
#include "graphics/icons/addframeicon.xpm"
#include "graphics/icons/removeframeicon.xpm"
#include "graphics/icons/newscene.xpm"
#include "graphics/icons/removescene.xpm"
#include "graphics/icons/gimp.xpm"

#include "src/application/runanimationhandler.h"
#include "src/application/modelhandler.h"
#include "src/application/camerahandler.h"
#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"

#include <QShortcut>
#include <QWidget>
#include <QToolTip>
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
#include <QLabel>
#include <QTimer>


ToolsMenu::ToolsMenu(RunAnimationHandler *runAnimationHandler,
		ModelHandler *modelHandler, CameraHandler *cameraHandler,
		FrameBar *frameBar, QWidget *parent)
	: QWidget(parent), ui(0), runAnimationHandler(runAnimationHandler),
	  modelHandler(modelHandler), cameraHandler(cameraHandler),
	  frameBar(frameBar) {
	ui = new Ui::Form;
	ui->setupUi(this);

	loopAccel = 0;
	playAccel = 0;
	mixAccel = 0;
	diffAccel = 0;
	playbackAccel = 0;

	captureTimer = new QTimer(this);
	connect( captureTimer, SIGNAL( timeout() ), cameraHandler, SLOT(captureFrame()));

	setupUi();
	createAccelerators();
}

ToolsMenu::~ToolsMenu() {
	delete captureTimer;
}

void ToolsMenu::setupUi() {
	setFocusPolicy(Qt::ClickFocus);

	ui->addFramesButton->setIcon( QPixmap(addframeicon) );
	connect(ui->addFramesButton, SIGNAL(clicked()), modelHandler, SLOT(chooseFrame()));

	ui->removeFramesButton->setIcon( QPixmap(removeframeicon) );
	connect(ui->removeFramesButton, SIGNAL(clicked()), modelHandler, SLOT(removeFrames()));

	runAnimationHandler->setRemoveFramesButton(ui->removeFramesButton);
	modelHandler->setRemoveFramesButton(ui->removeFramesButton);

	ui->addSceneButton->setIcon( QPixmap(newscene) );
	connect(ui->addSceneButton, SIGNAL(clicked()), modelHandler, SLOT(newScene()));

	ui->removeSceneButton->setIcon( QPixmap(removescene) );
	connect(ui->removeSceneButton, SIGNAL(clicked()), modelHandler, SLOT(removeScene()));

	ui->cameraButton->setIcon( QPixmap(cameraon) );
	cameraHandler->setCameraButton(ui->cameraButton);
	connect( ui->cameraButton, SIGNAL(clicked()), cameraHandler, SLOT(toggleCamera()) );

	ui->captureGroup->hide();
	connect( cameraHandler, SIGNAL(cameraStateChanged(bool)), this, SLOT(activateCaptureGroup(bool)) );

	ui->captureButton->setIcon(  QPixmap(captureicon) );
	connect(ui->captureButton, SIGNAL(clicked()), cameraHandler, SLOT(captureFrame()));

	connect(ui->viewChooseCombo, SIGNAL(activated (int)),this, SLOT(changeViewingMode(int)));

	ui->unitChooseCombo->setEnabled(false);
	connect(ui->unitChooseCombo, SIGNAL(activated (int)),this, SLOT(changeUnitMode(int)));

	ui->mixSlider->setMinimum(0);
	ui->mixSlider->setMaximum(5);
	ui->mixSlider->setPageStep(1);
	ui->mixSlider->setValue(2);
	ui->mixSlider->setTickPosition(QSlider::TicksBelow);
	connect( ui->mixSlider, SIGNAL(valueChanged(int)), cameraHandler, SLOT(setMixCount(int)) );
	connect( ui->mixSlider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderValue(int)) );

	ui->speedChooser->setMinimum(1);
	ui->speedChooser->setMaximum(30);
	ui->speedChooser->setValue(1);
	ui->speedChooser->setValue(PreferencesTool::get()->getPreference("fps", 10));
	connect( ui->speedChooser, SIGNAL(valueChanged(int)), runAnimationHandler, SLOT(setSpeed(int)));
	connect( ui->speedChooser, SIGNAL( valueChanged(int) ), cameraHandler, SLOT(setPlaybackSpeed(int)) );
	connect( ui->speedChooser, SIGNAL(editingFinished()), frameBar, SLOT(setFocus()) );

	ui->playButton->setIcon( QPixmap(playicon));

	runAnimationHandler->setPlayButton(ui->playButton);
	connect(ui->playButton, SIGNAL(clicked()), runAnimationHandler, SLOT(runAnimation()));
	ui->playButton->setEnabled(false);

	ui->nextFrameButton->setIcon( QPixmap(fastforwardicon) );
	ui->nextFrameButton->setAutoRepeat(true);
	connect( ui->nextFrameButton, SIGNAL(clicked()),
			frameBar, SLOT(selectNextFrame()) );
	ui->nextFrameButton->setEnabled(false);

	ui->previousFrameButton->setIcon( QIcon(QPixmap(rewindicon)) );
	ui->previousFrameButton->setAutoRepeat(true);
	connect( ui->previousFrameButton, SIGNAL(clicked()),
			frameBar, SLOT(selectPreviousFrame()) );
	ui->previousFrameButton->setEnabled(false);

	ui->toEndButton->setIcon( QIcon(QPixmap(steptoendicon)) );
	connect( ui->toEndButton, SIGNAL(clicked()),
			frameBar, SLOT(selectNextScene()) );
	ui->toEndButton->setEnabled(false);

	ui->toBeginningButton->setIcon( QIcon(QPixmap(steptobeginningicon)) );
	connect( ui->toBeginningButton, SIGNAL(clicked()),
			frameBar, SLOT(selectPreviousScene()) );
	ui->toBeginningButton->setEnabled(false);

	ui->stopButton->setIcon( QIcon(QPixmap(stopicon)) );
	connect( ui->stopButton, SIGNAL(clicked()), runAnimationHandler, SLOT(stopAnimation()));
	ui->stopButton->setEnabled(false);

	ui->pauseButton->setIcon( QIcon(QPixmap(pauseicon)) );
	ui->pauseButton->setEnabled(false);
	runAnimationHandler->setPauseButton(ui->pauseButton);
	connect(ui->pauseButton, SIGNAL(clicked()), runAnimationHandler, SLOT(pauseAnimation()));

	ui->loopButton->setIcon( QIcon(QPixmap(loopicon)) );
	runAnimationHandler->setLoopButton(ui->loopButton);
	connect( ui->loopButton, SIGNAL(clicked()), runAnimationHandler, SLOT(toggleLooping()) );
	ui->loopButton->setEnabled(false);

	//Launcher for the Gimp.
	ui->launchGimp->setIcon(QIcon(QPixmap(gimpicon)));
	connect(ui->launchGimp, SIGNAL(clicked()), modelHandler, SLOT(editCurrentFrame()));
}


void ToolsMenu::createAccelerators() {
	loopAccel = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_L), this);
	connect(loopAccel, SIGNAL(activated()), ui->loopButton, SLOT(toggle()));

	playAccel = new QShortcut(QKeySequence(Qt::Key_K), this);
	connect(playAccel, SIGNAL(activated()), runAnimationHandler, SLOT(toggleRunning()));

	mixAccel = new QShortcut(QKeySequence(Qt::Key_1), this);
	connect(mixAccel, SIGNAL(activated()), this, SLOT(setMixingMode()));

	diffAccel = new QShortcut(QKeySequence(Qt::Key_2), this);
	connect(diffAccel, SIGNAL(activated()), this, SLOT(setDiffingMode()));

	playbackAccel = new QShortcut(QKeySequence(Qt::Key_3), this);
	connect(playbackAccel, SIGNAL(activated()), this, SLOT(setPlaybackMode()));
}


void ToolsMenu::activateCaptureGroup(bool activate) {
	if (activate) {
		ui->captureGroup->show();
	} else {
		ui->captureGroup->hide();
	}
}


void ToolsMenu::retranslateStrings() {
	ui->speedChooserCaption->setText( tr("FPS chooser") );
	ui->mixSliderCaption->setText( tr("Number of images:") );

	ui->viewChooseCombo->clear();
	ui->viewChooseCombo->addItem( tr("Mix") );
	ui->viewChooseCombo->addItem( tr("Diff") );
	ui->viewChooseCombo->addItem( tr("Playback") );
	ui->viewChooseCombo->addItem( tr("Auto") );

	ui->unitChooseCombo->clear();
	ui->unitChooseCombo->addItem("Off");
	ui->unitChooseCombo->addItem( tr("Per second") );
	ui->unitChooseCombo->addItem( tr("Per minute") );
	ui->unitChooseCombo->addItem( tr("Per hour") );
	ui->unitChooseCombo->setCurrentIndex(0);

	//Tooltip and whatsthis text
	QString infoText =
			tr("<h4>Add Frames (CTRL+F)</h4> "
			"<p>Click on this button to <em>add</em> frames to the "
			"animation.</p>");
	ui->addFramesButton->setToolTip(infoText);
	ui->addFramesButton->setWhatsThis(infoText);

	infoText =
			tr("<h4>Remove Selection (Delete)</h4> "
			"<p>Click this button to <em>remove</em> the selected frames "
			"from the animation.</p>");
	ui->removeFramesButton->setToolTip(infoText);
	ui->removeFramesButton->setWhatsThis(infoText);

	infoText =
			tr("<h4>New Scene (CTRL+E)</h4> "
			"<p>Click this button to <em>create</em> a new <em>scene</em> "
			"to the animation.</p>");
	ui->addSceneButton->setToolTip(infoText);
	ui->addSceneButton->setWhatsThis(infoText);

	infoText =
			tr("<h4>Remove Scene (SHIFT+Delete)</h4> "
			"<p>Click this button to <em>remove</em> the selected scene "
			"from the animation.</p>");
	ui->removeSceneButton->setToolTip(infoText);
	ui->removeSceneButton->setWhatsThis(infoText);

	infoText =
			tr("<h4>Toggle camera on/off (C)</h4> "
			"<p>Click this button to toggle the camera on and off</p> ");
	ui->cameraButton->setToolTip(infoText);
	ui->cameraButton->setWhatsThis(infoText );

	infoText =
			tr("<h4>Launch Gimp</h4> "
			"<p>Click this button to open the active frame in Gimp</p> "
			"<p>Note that you can also drag images from the frame bar and drop them on Gimp</p>");
	ui->launchGimp->setToolTip(infoText);
	ui->launchGimp->setWhatsThis(infoText );

	infoText =
			tr("<h4>Capture Frame (Space)</h4> "
			"<p>Click on this button to <em>capture</em> a frame from the "
			"camera an put it in the animation</p> <p> This can also be "
			"done by pressing the <b>Space key</b></p>");
	ui->captureButton->setWhatsThis(infoText);
	ui->captureButton->setToolTip(infoText);

	infoText =
			tr("<h4>Number of images</h4> "
			"<p>By changing the value in this slidebar you can specify how many images "
			"backwards in the animation which should be mixed on top of the camera or "
			"if you are in playback mode: how many images to play. </p> "
			"<p>By mixing the previous image(s) onto the camera you can more easily see "
			"how the next shot will be in relation to the other, thereby making a smoother "
			"stop motion animation!</p>");
	ui->mixSliderCaption->setWhatsThis(infoText );
	ui->mixSlider->setWhatsThis(infoText);

	infoText =
			tr("<h4>FPS chooser</h4> "
			"<p>By changing the value in this "
			"chooser you set which speed the "
			"animation in the <b>FrameView</b> "
			"should run at.</p> "
			"<p>To start an animation press the "
			"<b>Run Animation</b> button.</p>");
    ui->speedChooserCaption->setWhatsThis(infoText );
    ui->speedChooser->setWhatsThis(infoText);

	infoText = tr("<h4>Play animation (K, P)</h4>");
	ui->playButton->setToolTip(infoText);

	infoText = tr("<h4>Stop animation (K, P)</h4>");
	ui->stopButton->setToolTip(infoText);

	infoText = tr("<h4>Previous frame (J, Left)</h4>");
	ui->previousFrameButton->setToolTip(infoText);

	infoText = tr("<h4>Next frame (L, Right)</h4>");
	ui->nextFrameButton->setToolTip(infoText);

	infoText = tr("<h4>Previous scene (I)</h4>");
	ui->toBeginningButton->setToolTip(infoText);

	infoText = tr("<h4>Next scene (O)</h4>");
	ui->toEndButton->setToolTip(infoText);

	infoText =
			tr("<h4>Loop animation (CTRL+L)</h4> <p>With this button you can set whether "
			"you want the animation to play to the end, or to loop indefinitely.</p>");
	ui->loopButton->setToolTip(infoText);
}


void ToolsMenu::updateSliderValue(int sliderValue) {
	if ( captureTimer->isActive() && sliderValue != 0) {
		int factor = 0;
		int index = ui->unitChooseCombo->currentIndex();
		switch (index) {
			case 1:
				factor = 1000;
				break;
			case 2:
				factor = 60000;
				break;
			case 3:
				factor = 3600000;
				break;
		}
		captureTimer->setInterval(factor / sliderValue);
	}
}


void ToolsMenu::setMixingMode() {
	changeViewingMode(0);
}


void ToolsMenu::setDiffingMode() {
	changeViewingMode(1);
}


void ToolsMenu::setPlaybackMode() {
	changeViewingMode(2);
}


void ToolsMenu::changeViewingMode(int index) {
	if ( cameraHandler->setViewMode(index) ) {
		ui->viewChooseCombo->setCurrentIndex(index);
		ui->unitChooseCombo->setCurrentIndex(0);
		switch (index) {
			case 0:
			{
				ui->mixSlider->setValue(PreferencesTool::get()->getPreference("mixcount", 2));
				ui->mixSlider->setMaximum(5);
				ui->mixSlider->setEnabled(true);
				ui->mixSliderCaption->setEnabled(true);
				ui->unitChooseCombo->setEnabled(false);
				captureTimer->stop();
				break;
			}
			case 2:
			{
				ui->mixSlider->setMaximum(50);
				ui->mixSlider->setValue(PreferencesTool::get()->getPreference("playbackcount", 5));
				ui->mixSliderCaption->setEnabled(true);
				ui->mixSlider->setEnabled(true);
				ui->unitChooseCombo->setEnabled(false);
				captureTimer->stop();
				break;
			}
			case 3:
			{
				ui->mixSlider->setMaximum(10);
				ui->mixSlider->setValue(1);
				ui->mixSliderCaption->setEnabled(true);
				ui->mixSlider->setEnabled(true);
				ui->unitChooseCombo->setEnabled(true);
				break;
			}
			default:
			{
				ui->mixSlider->setEnabled(false);
				ui->mixSliderCaption->setEnabled(false);
				ui->unitChooseCombo->setEnabled(false);
				captureTimer->stop();
				break;
			}
		}
	} else {
		QMessageBox::warning(this, tr("Notice"), tr(
				"Playback only currently works when running the grabber "
				"as a daemon. Go to the preferences menu (CTRL+P) to switch "
				"to running the image grabbing as a daemon."),
				QMessageBox::Ok,
				QMessageBox::NoButton,
				QMessageBox::NoButton);
		ui->viewChooseCombo->setCurrentIndex(0);
		cameraHandler->setViewMode(0);
	}
}


void ToolsMenu::changeUnitMode(int index) {
	int sliderValue = ui->mixSlider->value();
	if (sliderValue == 0 || index == 0) {
		if (captureTimer->isActive()) {
			captureTimer->stop();
		}
		return;
	}

	int factor = 0;
	switch (index) {
		case 1:
			factor = 1000;
			break;
		case 2:
			factor = 60000;
			break;
		case 3:
			factor = 3600000;
			break;
		default:
			if ( captureTimer->isActive() ) {
				captureTimer->stop();
			}
			break;
	}

	if ( captureTimer->isActive() == false) {
		// Grab the first frame manually
		cameraHandler->captureFrame();
		// then grab at the given interval
		captureTimer->start(factor / sliderValue);
	} else {
		captureTimer->setInterval(factor / sliderValue);
	}
}


void ToolsMenu::fixNavigationButtons(int modelSize) {
	//Not <=1 because it is signed with a meaning for -1.
	if (modelSize == 0 || modelSize == 1) {
		if (ui->previousFrameButton->isEnabled()) {
			ui->previousFrameButton->setEnabled(false);
			ui->nextFrameButton->setEnabled(false);
			ui->playButton->setEnabled(false);
			ui->loopButton->setEnabled(false);
			ui->pauseButton->setEnabled(false);
			ui->stopButton->setEnabled(false);
			ui->toEndButton->setEnabled(false);
			ui->toBeginningButton->setEnabled(false);
		}
	} else if (modelSize >= 2) {
		if ( !ui->previousFrameButton->isEnabled() && !cameraHandler->isCameraRunning() ) {
			ui->previousFrameButton->setEnabled(true);
			ui->nextFrameButton->setEnabled(true);
			ui->playButton->setEnabled(true);
			ui->loopButton->setEnabled(true);
			ui->pauseButton->setEnabled(true);
			ui->stopButton->setEnabled(true);
			ui->toEndButton->setEnabled(true);
			ui->toBeginningButton->setEnabled(true);
		}
	}
}


void ToolsMenu::cameraOn(bool isOn) {
	if (isOn) {
		ui->playButton->setEnabled(false);
		ui->loopButton->setEnabled(false);
		ui->pauseButton->setEnabled(false);
		ui->stopButton->setEnabled(false);
		ui->launchGimp->setEnabled(false);
		runAnimationHandler->stopAnimation();
 		changeViewingMode(0);
		ui->viewChooseCombo->setCurrentIndex(0);
	} else {
		ui->playButton->setEnabled(true);
		ui->loopButton->setEnabled(true);
		ui->pauseButton->setEnabled(true);
		ui->stopButton->setEnabled(true);
		ui->launchGimp->setEnabled(true);
		if ( captureTimer->isActive() ) {
			captureTimer->stop();
		}
	}
	fixNavigationButtons(DomainFacade::getFacade()->getModelSize());
}
