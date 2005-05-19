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
#include "toolsmenu.h"

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

#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qaccel.h>
#include <qmessagebox.h>

#include <iostream>


ToolsMenu::ToolsMenu( RunAnimationHandler *runAnimationHandler, ModelHandler *modelHandler,
		CameraHandler *cameraHandler, QWidget *parent ) : QFrame(parent),
		runAnimationHandler(runAnimationHandler), modelHandler(modelHandler), 
		cameraHandler(cameraHandler)
{
	captureGroup = NULL;
	runAnimationGroup = NULL;
	addFrameButton = NULL;
	removeFramesButton = NULL;
	newSceneButton = NULL;
	removeSceneButton = NULL;
	cameraButton = NULL;
	captureFrameButton = NULL;
	playButton = NULL;
	nextFrameButton = NULL;
	previousFrameButton = NULL;
	toEndButton = NULL;
	toBeginningButton = NULL;
	stopButton = NULL;
	pauseButton = NULL;
	loopButton = NULL;
	mixSlider = NULL;
	animationSpeedChooser = NULL;
	viewChooseCombo = NULL;
	animationSpeedChooserCaption = NULL;
	mixSliderCaption = NULL;
	horizontalSpace = NULL;
	verticalSpace = NULL;
	horizontalDummySpace = NULL;
	groupSpace = NULL;
	grid = NULL;
	captureGrid = NULL;
	runAnimationGrid = NULL;
	loopAccel = NULL;
	playAccel = NULL;
	mixAccel = NULL;
	diffAccel = NULL;
	playbackAccel = NULL;
	
	addWidgets();
	createAccelerators();
}


void ToolsMenu::createAccelerators()
{
	loopAccel = new QAccel( this );
	loopAccel->connectItem( loopAccel->insertItem(CTRL+Key_L), 
			loopButton, SLOT(toggle()) );
	
	playAccel = new QAccel( this );        
	playAccel->connectItem( playAccel->insertItem(Key_K), 
			runAnimationHandler, SLOT(toggleRunning()) );
	playAccel->connectItem( playAccel->insertItem(Key_P), 
			runAnimationHandler, SLOT(toggleRunning()) );
	
	mixAccel = new QAccel( this );
	mixAccel->connectItem( mixAccel->insertItem(Key_1), 
			this, SLOT(setMixingMode()) );
	
	diffAccel = new QAccel( this );
	diffAccel->connectItem( diffAccel->insertItem(Key_2), 
			this, SLOT(setDiffingMode()) );
	
	playbackAccel = new QAccel( this );
	playbackAccel->connectItem( playbackAccel->insertItem(Key_3), 
			this, SLOT(setPlaybackMode()) );
}


void ToolsMenu::addWidgets()
{
	// Spacer elements
	horizontalSpace = 
			new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	verticalSpace = 
			new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
	horizontalDummySpace = 
			new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	groupSpace = 
			new QSpacerItem(0, 25, QSizePolicy::Minimum, QSizePolicy::Fixed);
	
	// Widgets for the toolsmenu:
	addFrameButton = new QPushButton(this);
	addFrameButton->setPixmap( QPixmap(addframeicon) );
	addFrameButton->setFocusPolicy( QWidget::NoFocus );
	connect(addFrameButton, SIGNAL(clicked()), modelHandler, SLOT(chooseFrame()));
	
	removeFramesButton = new QPushButton(this);
	removeFramesButton->setPixmap( QPixmap(removeframeicon) );
	removeFramesButton->setFocusPolicy( QWidget::NoFocus );
	connect(removeFramesButton, SIGNAL(clicked()), modelHandler, SLOT(removeFrames()));
	runAnimationHandler->setRemoveFramesButton(removeFramesButton);
	modelHandler->setRemoveFramesButton(removeFramesButton);
	
	newSceneButton = new QPushButton(this);
	newSceneButton->setPixmap( QPixmap(newscene) );
	newSceneButton->setFocusPolicy( QWidget::NoFocus );
	connect(newSceneButton, SIGNAL(clicked()), modelHandler, SLOT(newScene()));
	
	removeSceneButton = new QPushButton(this);
	removeSceneButton->setPixmap( QPixmap(removescene) );
	removeSceneButton->setFocusPolicy( QWidget::NoFocus );
	connect(removeSceneButton, SIGNAL(clicked()), modelHandler, SLOT(removeScene()));
	
	cameraButton = new QPushButton(this);
	cameraButton->setPixmap( QPixmap(cameraon) );
	cameraButton->setFocusPolicy( QWidget::NoFocus );	
	cameraHandler->setCameraButton(cameraButton);
	connect( cameraButton, SIGNAL(clicked()), cameraHandler, SLOT(toggleCamera()) );
	
	//Groupbox containing buttons related to capturing frame from the camera.
	captureGroup = new QGroupBox(this);
	captureGroup->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	captureGroup->hide();
	connect( cameraHandler, SIGNAL(cameraStateChanged(bool)), 
			this, SLOT(activateCaptureGroup(bool)) );
	
	
	captureFrameButton = new QPushButton( captureGroup );
	captureFrameButton->setPixmap(  QPixmap(captureicon) );
	captureFrameButton->setFocusPolicy( QWidget::NoFocus );
	connect(captureFrameButton, SIGNAL(clicked()), cameraHandler, SLOT(captureFrame()));
	
	viewChooseCombo = new QComboBox(captureGroup);
	viewChooseCombo->setFocusPolicy( QWidget::NoFocus );
	connect(viewChooseCombo, SIGNAL(activated (int)),this, SLOT(changeViewingMode(int)));
	
	mixSliderCaption = new QLabel(captureGroup);
	mixSliderCaption->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	
	mixSlider = new QSlider(0, 5, 1, 2, Qt::Horizontal, captureGroup);
	mixSlider->setTickmarks(QSlider::Below);
	mixSlider->setFocusPolicy( QWidget::NoFocus );
	connect( mixSlider, SIGNAL(valueChanged(int)), cameraHandler, SLOT(setMixCount(int)) );
	
	//Groupbox containing buttons related to running the animation.
	runAnimationGroup = new QGroupBox(this);
	runAnimationGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	animationSpeedChooserCaption = new QLabel(runAnimationGroup );
	animationSpeedChooserCaption->setSizePolicy(
			QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	
	animationSpeedChooser = new QSpinBox( 1, 30, 1, runAnimationGroup );
	animationSpeedChooser->setFocusPolicy( QWidget::NoFocus );
	animationSpeedChooser->setValue(PreferencesTool::get()->getPreference("fps", 10));
	connect( animationSpeedChooser, SIGNAL(valueChanged(int)),
			runAnimationHandler, SLOT(setSpeed(int)));
	connect( animationSpeedChooser, SIGNAL( valueChanged(int) ),
			cameraHandler, SLOT(setPlaybackSpeed(int)) );
	//This preference is set in the RunAnimationHandler::setSpeed function
	//animationSpeedChooser->setValue(PreferencesTool::get()->getPreference("fps", 10));
	
	playButton = new QPushButton( runAnimationGroup );
	playButton->setIconSet( QIconSet(QPixmap(playicon)) );
	playButton->setFocusPolicy( QWidget::NoFocus );
	playButton->setToggleButton(true);
	runAnimationHandler->setPlayButton(playButton);
	connect(playButton, SIGNAL(clicked()), runAnimationHandler, SLOT(runAnimation()));
	playButton->setEnabled(false);
	
	nextFrameButton = new QPushButton( runAnimationGroup );
	nextFrameButton->setIconSet( QIconSet(QPixmap(fastforwardicon)) );
	nextFrameButton->setFocusPolicy( QWidget::NoFocus );
	nextFrameButton->setAutoRepeat(true);
	connect( nextFrameButton, SIGNAL(clicked()), 
			runAnimationHandler, SLOT(selectNextFrame()) );
	nextFrameButton->setEnabled(false);
		
	previousFrameButton = new QPushButton( runAnimationGroup );
	previousFrameButton->setIconSet( QIconSet(QPixmap(rewindicon)) );
	previousFrameButton->setFocusPolicy( QWidget::NoFocus );
	previousFrameButton->setAutoRepeat(true);
	connect( previousFrameButton, SIGNAL(clicked()), 
			runAnimationHandler, SLOT(selectPreviousFrame()) );
	previousFrameButton->setEnabled(false);
	
	toEndButton = new QPushButton( runAnimationGroup );
	toEndButton->setIconSet( QIconSet(QPixmap(steptoendicon)) );
	toEndButton->setFocusPolicy( QWidget::NoFocus );
	connect( toEndButton, SIGNAL(clicked()), 
			runAnimationHandler, SLOT(selectNextScene()) );
	toEndButton->setEnabled(false);
	
	toBeginningButton = new QPushButton( runAnimationGroup );
	toBeginningButton->setIconSet( QIconSet(QPixmap(steptobeginningicon)) );
	toBeginningButton->setFocusPolicy( QWidget::NoFocus );
	connect( toBeginningButton, SIGNAL(clicked()), 
			runAnimationHandler, SLOT(selectPreviousScene()) );
	toBeginningButton->setEnabled(false);
	
	stopButton = new QPushButton( runAnimationGroup );
	stopButton->setIconSet( QIconSet(QPixmap(stopicon)) );
	stopButton->setFocusPolicy( QWidget::NoFocus );
	connect( stopButton, SIGNAL(clicked()), runAnimationHandler, SLOT(stopAnimation()));
	stopButton->setEnabled(false);
	
	pauseButton = new QPushButton( runAnimationGroup );
	pauseButton->setIconSet( QIconSet(QPixmap(pauseicon)) );
	pauseButton->setFocusPolicy( QWidget::NoFocus );
	pauseButton->setEnabled(false);
	runAnimationHandler->setPauseButton(pauseButton);
	connect(pauseButton, SIGNAL(clicked()), runAnimationHandler, SLOT(pauseAnimation()));
	
	loopButton = new QPushButton( runAnimationGroup );
	loopButton->setIconSet( QIconSet(QPixmap(loopicon)) );
	loopButton->setFocusPolicy( QWidget::NoFocus );
	loopButton->setToggleButton(true);
	runAnimationHandler->setLoopButton(loopButton);
	connect( loopButton, SIGNAL(clicked()), runAnimationHandler, SLOT(toggleLooping()) );
	
	// Layouts
	grid = new QGridLayout( this, 1, 1, 3);
	captureGrid = new QGridLayout( captureGroup, 1, 1, 3);
	runAnimationGrid = new QGridLayout( runAnimationGroup, 1, 1, 3);
	
	//Add the widgets to the grid
	grid->addWidget(addFrameButton, 0, 0);
	grid->addWidget(removeFramesButton, 0, 1);
	grid->addWidget(newSceneButton, 1, 0);
	grid->addWidget(removeSceneButton, 1, 1);
	grid->addMultiCellWidget(cameraButton, 2, 2, 0, 1);
	grid->addItem(horizontalDummySpace, 3, 0);
	grid->addMultiCellWidget(captureGroup, 4, 4, 0, 1);
	grid->addItem(groupSpace, 5, 0);
	grid->addMultiCellWidget(runAnimationGroup, 7, 7, 0, 1 );

	captureGrid->addItem( horizontalSpace, 0, 0 ); // left
	captureGrid->addItem( horizontalSpace, 0, 3);  // right
	captureGrid->addWidget( captureFrameButton, 1, 1 );
	captureGrid->addWidget( viewChooseCombo, 1, 2 ); 
	captureGrid->addItem( verticalSpace, 2, 1 ); // middle
	captureGrid->addMultiCellWidget( mixSliderCaption, 3, 3, 1, 2 );
	captureGrid->addMultiCellWidget( mixSlider, 4, 4, 1, 2 );
	captureGrid->addItem( verticalSpace, 0, 1);  // top
	captureGrid->addItem(verticalSpace, 5, 0);   // bottom
	
	runAnimationGrid->addItem( horizontalSpace, 0, 0 ); // left
	runAnimationGrid->addItem( horizontalSpace, 0, 6);  // right
	runAnimationGrid->addWidget( toBeginningButton, 1, 1 );
	runAnimationGrid->addWidget( previousFrameButton, 1, 2 );
	runAnimationGrid->addWidget( playButton, 1, 3 );
	runAnimationGrid->addWidget( nextFrameButton, 1, 4 );
	runAnimationGrid->addWidget( toEndButton, 1, 5 );
	runAnimationGrid->addWidget( stopButton, 2, 3 );
	runAnimationGrid->addWidget( pauseButton, 2, 2 );
	runAnimationGrid->addWidget( loopButton, 2, 4 );
	runAnimationGrid->addMultiCellWidget( animationSpeedChooserCaption, 4, 4, 1, 5 );
	runAnimationGrid->addMultiCellWidget( animationSpeedChooser, 5, 5, 1, 5 );
	runAnimationGrid->addItem( verticalSpace, 0, 1);  // top
	runAnimationGrid->addItem( verticalSpace, 6, 0);  // bottom
}


void ToolsMenu::activateCaptureGroup(bool activate)
{
	if(activate) {		
		captureGroup->show();
	}
	else {
		captureGroup->hide();
	}
}


void ToolsMenu::retranslateStrings()
{
	animationSpeedChooserCaption->setText( tr("FPS chooser") );
	mixSliderCaption->setText( tr("Number of images:") );
	
	viewChooseCombo->clear();
	viewChooseCombo->insertItem( tr("Mix"), 0 );
	viewChooseCombo->insertItem( tr("Diff"), 1 );
	viewChooseCombo->insertItem( tr("Playback"), 2 );
	
	//Tooltip and whatsthis text
	QString infoText = 
			tr("<h4>Add Frames (CTRL+F)</h4> "
			"<p>Click on this button to <em>add</em> frames to the "
			"animation.</p>");
	QToolTip::add(addFrameButton, infoText);
	QWhatsThis::add(addFrameButton, infoText);
	
	infoText = 
			tr("<h4>Remove Selection (Delete)</h4> "
			"<p>Click this button to <em>remove</em> the selected frames "
			"from the animation.</p>");
	QToolTip::add(removeFramesButton, infoText);
	QWhatsThis::add(removeFramesButton, infoText);
	
	infoText = 
			tr("<h4>New Scene (CTRL+E)</h4> "
			"<p>Click this button to <em>create</em> a new <em>scene</em> "
			"to the animation.</p>");
	QToolTip::add(newSceneButton, infoText);
	QWhatsThis::add(newSceneButton, infoText);
	
	infoText = 
			tr("<h4>Remove Scene (SHIFT+Delete)</h4> "
			"<p>Click this button to <em>remove</em> the selected scene "
			"from the animation.</p>");
	QToolTip::add(removeSceneButton, infoText);
	QWhatsThis::add(removeSceneButton, infoText);
	
	infoText = 
			tr("<h4>Toggle camera on/off (C)</h4> "
			"<p>Click this button to toggle the camera on and off</p> ");
			//"<p>You can also toggle the camera by pressing the <b>C key</b></p>");
	QWhatsThis::add( cameraButton, infoText );
	QToolTip::add( cameraButton, infoText );
	
	infoText =
			tr("<h4>Capture Frame (Space)</h4> "
			"<p>Click on this button to <em>capture</em> a frame from the "
			"camera an put it in the animation</p> <p> This can also be "
			"done by pressing the <b>Space key</b></p>");
	QWhatsThis::add( captureFrameButton, infoText );
	QToolTip::add( captureFrameButton, infoText );
	
	infoText = 
			tr("<h4>Number of images</h4> "
			"<p>By changing the value in this slidebar you can specify how many images "
			"backwards in the animation which should be mixed on top of the camera or "
			"if you are in playback mode: how many images to play. </p> "
			"<p>By mixing the previous image(s) onto the camera you can more easily see "
			"how the next shot will be in relation to the other, therby making a smoother "
			"stop motion animation!</p>");
	QWhatsThis::add( mixSliderCaption, infoText );
	QWhatsThis::add( mixSlider, infoText );
	
	infoText = 
			tr("<h4>FPS chooser</h4> "
			"<p>By changing the value in this "
			"chooser you set which speed the "
			"animation in the <b>FrameView</b> "
			"should run at.</p> "
			"<p>To start an animation press the "
			"<b>Run Animation</b> button.</p>");
    QWhatsThis::add( animationSpeedChooserCaption, infoText );
    QWhatsThis::add( animationSpeedChooser, infoText );
	
	infoText =
			tr("<h4>Play animation (K, P)</h4>");
	QToolTip::add(playButton, infoText);
	
	infoText =
			tr("<h4>Stop animation (K, P)</h4>");
	QToolTip::add(stopButton, infoText);
	
	infoText =
			tr("<h4>Previous frame (J, Left)</h4>");
	QToolTip::add(previousFrameButton, infoText);
	
	infoText =
			tr("<h4>Next frame (L, Right)</h4>");
	QToolTip::add(nextFrameButton, infoText);
	
	infoText =
			tr("<h4>Previous scene (I)</h4>");
	QToolTip::add(toBeginningButton, infoText);
	
	infoText =
			tr("<h4>Next scene (O)</h4>");
	QToolTip::add(toEndButton, infoText);
	
	infoText =
			tr("<h4>Loop animation (CTRL+L)</h4> <p>With this button you can set whether "
			"you want the animation to play to the end, or to loop indefinetly.</p>");
	QToolTip::add(loopButton, infoText);
}


void ToolsMenu::setMixingMode()
{
	changeViewingMode(0);
}


void ToolsMenu::setDiffingMode()
{
	changeViewingMode(1);
}


void ToolsMenu::setPlaybackMode()
{
	changeViewingMode(2);
}


void ToolsMenu::changeViewingMode(int index)
{
	if( cameraHandler->setViewMode(index) ) {
		viewChooseCombo->setCurrentItem(index);
		switch (index)
		{
			case 0:
			{
				//mixSlider->setMinValue();
				//Also triggers the sliders signal causing the 
				//videoviews mixcount to get updated
				mixSlider->setValue(PreferencesTool::get()->
						getPreference("mixcount", 2));
				mixSlider->setMaxValue(5);
				mixSlider->setEnabled(true);
				mixSliderCaption->setEnabled(true);
				break;
			}
			case 2:
			{
				mixSlider->setMaxValue(50);
				mixSlider->setValue(PreferencesTool::get()->
						getPreference("playbackcount", 5));
				mixSliderCaption->setEnabled(true);
				mixSlider->setEnabled(true);
				break;
			}
			default:
			{
				mixSlider->setEnabled(false);
				mixSliderCaption->setEnabled(false);
				break;
			}
		}
	}
	else {
		QMessageBox::warning(this, tr("Notice"), tr(
				"Playback only currently works when running the grabber \n"
				"as a deamon. Go to the preferences menu (CTRL+P) to switch \n"
				"to running the image grabbing as a deamon."),
				QMessageBox::Ok,
				QMessageBox::NoButton, 
				QMessageBox::NoButton);
		viewChooseCombo->setCurrentItem(0);
		cameraHandler->setViewMode(0);
	}
}


void ToolsMenu::modelSizeChanged(int modelSize)
{
	//Not <=1 because it is signed with a meaning for -1.
	if(modelSize == 0 || modelSize == 1) {
		if(previousFrameButton->isEnabled()) {
			previousFrameButton->setEnabled(false);
			nextFrameButton->setEnabled(false);
			playButton->setEnabled(false);
			pauseButton->setEnabled(false);
			stopButton->setEnabled(false);
			toEndButton->setEnabled(false);
			toBeginningButton->setEnabled(false);
		}
	}
	else if(modelSize >= 2) {
		if(!previousFrameButton->isEnabled() && !cameraHandler->isCameraRunning()) {
			previousFrameButton->setEnabled(true);
			nextFrameButton->setEnabled(true);
			playButton->setEnabled(true);
			pauseButton->setEnabled(true);
			stopButton->setEnabled(true);
			toEndButton->setEnabled(true);
			toBeginningButton->setEnabled(true);
		}
	}
}


void ToolsMenu::cameraOn(bool isOn)
{
	if(isOn) {
		playButton->setEnabled(false);
		pauseButton->setEnabled(false);
		stopButton->setEnabled(false);
		runAnimationHandler->stopAnimation();
 		changeViewingMode(0);
		viewChooseCombo->setCurrentItem(0);
	}
	else {
		playButton->setEnabled(true);
		pauseButton->setEnabled(true);
		stopButton->setEnabled(true);
	}
}
