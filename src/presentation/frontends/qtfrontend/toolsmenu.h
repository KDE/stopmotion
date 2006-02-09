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
#ifndef TOOLSMENU_H
#define TOOLSMENU_H

#include "src/application/runanimationhandler.h"
#include "src/application/modelhandler.h"
#include "src/application/camerahandler.h"

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qcombobox.h>


/**
 * The tools menu on the left side in the application.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ToolsMenu : public QFrame
{
	Q_OBJECT
public:
	/**
	 * Sets up the toolsmenu.
	 * @param parent the parent of the widget.
	 */
	ToolsMenu( RunAnimationHandler *runAnimationHandler, ModelHandler *modelHandler,
			CameraHandler *cameraHandler, QWidget *parent=0 );
	
	/** 
	 * Creates key accelerators (keyboard shortcuts)
	 * More can be found in the function MainWindowGUI::createAccelerators().
	 */
	void createAccelerators();
	
	/**
	 * Add widgets to the menu.
	 */
	void addWidgets();
	
	/**
	 * Retranslates the strings.
	 * This function is called after a new translator has been installed so that 
	 * the program strings are retranslated to the new language.
	 */
	void retranslateStrings();
	
public slots:
	
	/**
	 * Activates or deactivates the captureGroup
	 * @param activate true if the group should be activated.
	 */
	void activateCaptureGroup(bool activate);
	
	/**
	 * This slot is notified when the size of the model changes so that menuframe
	 * menu options can be adjusted (activated/deactivated, etc).
	 * @param modelSize the new size of the model.
	 */
	void modelSizeChanged(int modelSize);
	
private:
	RunAnimationHandler *runAnimationHandler;
	ModelHandler *modelHandler;
	CameraHandler *cameraHandler;
	
	QGroupBox *captureGroup;
	QGroupBox *runAnimationGroup;
	
	//Widgets
	QPushButton *addFrameButton;
	QPushButton *removeFramesButton;
	QPushButton *newSceneButton;
	QPushButton *removeSceneButton;
	QPushButton *cameraButton;
	QPushButton *captureFrameButton;
	QPushButton *playButton;
	QPushButton *nextFrameButton;
	QPushButton *previousFrameButton;
	QPushButton *toEndButton;
	QPushButton *toBeginningButton;
	QPushButton *stopButton;
	QPushButton *pauseButton;
	QPushButton *loopButton;
	QSlider *mixSlider;
	QSpinBox *animationSpeedChooser;
	QComboBox *viewChooseCombo;
	QComboBox *unitChooseCombo;
	QLabel *animationSpeedChooserCaption;
	QLabel *mixSliderCaption;
	QSpacerItem *horizontalSpace;
	QSpacerItem *verticalSpace;
	QSpacerItem *horizontalDummySpace;
	QSpacerItem *groupSpace;
	QGridLayout *grid;
	QGridLayout *captureGrid;
	QGridLayout *runAnimationGrid;
	QAccel *loopAccel;
	QAccel *playAccel;
	QAccel *mixAccel;
	QAccel *diffAccel;
	QAccel *playbackAccel;
	QTimer *captureTimer;
	
private slots:

	/**
	 * Slot for notified the toolsmenu when the viewing mode changes, so that widgets
	 * can be updated.
	 * @param index the new viewing mode.
	 */
	void changeViewingMode(int index);
	
	/**
	 * Slot for notified the toolsmenu when the unit mode changes.
	 * @param index the new unit mode.
	 */
	void changeUnitMode(int index);
	
	/**
	 * Slot for updating the slider value when running in automatic mode.
	 * @param value the new slider value.
	 */
	void updateSliderValue(int value);
	
	/**
	 * Slot for setting the camera mode to mixing.
	 * Used by the shortcut key.
	 */
	void setMixingMode();
	
	/**
	 * Slot for setting the camera mode to Diffing.
	 * Used by the shortcut key.
	 */
	void setDiffingMode();
	
	/**
	 * Slot for setting the camera mode to Playback.
	 * Used by the shortcut key.
	 */
	void setPlaybackMode();
	
	/**
	 * Slot for being notified when the camera is turned on so that
	 * it disable of some buttons.
	 * @param isOn true if the camera is turned on. 
	 */
	void cameraOn(bool isOn);
	
signals:
	void viewingModeChanged();
};

#endif
