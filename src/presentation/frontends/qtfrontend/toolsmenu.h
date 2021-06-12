/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
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
#ifndef TOOLSMENU_H
#define TOOLSMENU_H

#include <QWidget>

class QShortcut;
class QTimer;
class QWidget;
class RunAnimationHandler;
class ModelHandler;
class CameraHandler;
class FrameBar;
class Ui_Form;

/**
 * The tools menu on the left side in the application.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ToolsMenu : public QWidget {
	Q_OBJECT
public:
	/**
	 * Sets up the toolsmenu.
	 * @param parent the parent of the widget.
	 */
	ToolsMenu( RunAnimationHandler *runAnimationHandler,
			ModelHandler *modelHandler, CameraHandler *cameraHandler,
			FrameBar *frameBar, QWidget *parent = 0 );

	~ToolsMenu();

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
	void fixNavigationButtons(int modelSize);

private:
	Ui_Form* ui;

	RunAnimationHandler *runAnimationHandler;
	ModelHandler *modelHandler;
	CameraHandler *cameraHandler;
	FrameBar *frameBar;

	QShortcut *loopAccel;
	QShortcut *playAccel;
	QShortcut *mixAccel;
	QShortcut *diffAccel;
	QShortcut *playbackAccel;
	QTimer *captureTimer;

	/**
	 * Creates connections, adds icons to the buttons and sets
	 * different properties.
	 */
	void setupUi();

	/**
	 * Creates key accelerators (keyboard shortcuts)
	 * More can be found in the function MainWindowGUI::createAccelerators().
	 */
	void createAccelerators();

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
