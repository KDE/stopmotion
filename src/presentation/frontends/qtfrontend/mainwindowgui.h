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
#ifndef QTGUI_H
#define QTGUI_H

#include "src/config.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"
#include "src/presentation/frontends/qtfrontend/frameview.h"
#include "src/presentation/frontends/qtfrontend/flexiblespinbox.h"
#include "src/application/camerahandler.h"
#include "src/application/editmenuhandler.h"
#include "src/application/runanimationhandler.h"
#include "src/application/modelhandler.h"
#include "src/application/languagehandler.h"
#include "src/application/soundhandler.h"
#include "src/application/externalchangemonitor.h"
#include "src/presentation/frontends/qtfrontend/menuframe.h"
#include "src/presentation/frontends/qtfrontend/framepreferencesmenu.h"
#include "src/presentation/frontends/qtfrontend/preferencesmenu.h"
#include "src/presentation/frontends/qtfrontend/toolsmenu.h"

#include <QtGui>


/**
 *The main window class for the stopmotion application. 
 *This class sets up the main GUI and connects the buttons.
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
*/
class MainWindowGUI : public QMainWindow
{
	Q_OBJECT
public:
	enum {SAVE, SAVE_AS, UNDO, REDO, CUT, COPY, PASTE, GOTO};
	
	/**
	 * Sets up the program main GUI and connect the widgets and handlers.
	 * @param stApp the application class for changing translator
	 * on runtime.
	 */
	MainWindowGUI(QApplication *stApp);
	
	/**
	 * Cleans up after the mainwindowgui.
	 */
	~MainWindowGUI();
	
	/**
	 * Overloaded mouse listener. Closes the embedded menues when the user
	 * clicks inside the main window.
	 * @param e information about the event.
	 */
	void mousePressEvent ( QMouseEvent * e );
	
	/**
	 * Overloaded event listener which recieves information when a keyboard key is
	 * pressed.
	 * @param k information about the key event. 
	 */
	void keyPressEvent( QKeyEvent *k );
	
private:
	QApplication *stApp;
	QWidget *centerWidget;
	QVBoxLayout *centerWidgetLayout;
	QWidget *bottomWidget;
	QVBoxLayout *bottomWidgetLayout;
	QWidget *workArea;
	QHBoxLayout *workAreaLayout;
	FrameBar *frameBar;
	FrameView *frameView;
	QWidget *gotoMenuWidget;
	QHBoxLayout *gotoMenuWidgetLayout;
	
	//Actions
	QAction *newAct;
	QAction *openAct;
	QAction *mostRecentAct;
	QAction *secondMostRecentAct;
	QAction *thirdMostRecentAct;
	QAction *saveAct;
	QAction *saveAsAct;
	QAction *videoAct;
	QAction *cinerellaAct;
	QAction *quitAct;
	QAction *undoAct;
	QAction *redoAct;
	QAction *cutAct;
	QAction *copyAct;
	QAction *pasteAct;
	QAction *gotoFrameAct;
	QAction *configureAct;
 	QAction *whatsthisAct;
	QAction *aboutAct;
	QAction *helpAct;
	
	//Menues
	QMenu *fileMenu;
	QMenu *exportMenu;
	QMenu *mostRecentMenu;
	QMenu *editMenu;
	QMenu *settingsMenu;
	QMenu *languagesMenu;
	QMenu *helpMenu;
	ToolsMenu *toolsMenu;
	FramePreferencesMenu *framePreferencesMenu;
	PreferencesMenu *preferencesMenu;
	
	//MenuFrame *gotoMenu;
	QPushButton *gotoMenuCloseButton;
	
	//Widgets
	QLabel *numberDisplay;
	FlexibleSpinBox *gotoSpinner;
	QLabel *gotoFrameLabel;
	
	//Handlers
	ModelHandler *modelHandler;
	SoundHandler *soundHandler;
	CameraHandler *cameraHandler;
	EditMenuHandler *editMenuHandler;
	LanguageHandler *languageHandler;
	RunAnimationHandler *runAnimationHandler;
	ExternalChangeMonitor *changeMonitor;
	
	char *lastVisitedDir;
	
	/**
	 * Creates the handlers for handling user requests.
	 * @param stApp the application class for changing translator
	 * on runtime.
	 */
	void createHandlers(QApplication *stApp);
	
	/**
	 * Sets up the ExternalChangeMonitor to monitor the project directories
	 * for changes in the project files.
	 */
	void setupDirectoryMonitoring();
	
	/**
	 * Creates key accelerators (keyboard shortcuts)
	 *
	 * More can be found in the function ToolsMenu::createAccelerators().
	 */
	void createAccelerators();
	
	/**
	 * Creates the actions from which the menus are created.
	 */
	void createActions();
	
	/**
	 * Creates and sets up the menu and the toolbar.
	 */
	 void createMenus();
	
	/**
	 * Creates the preferences menu.
	 * @param parent 
	 */
	void makePreferencesMenu(QVBoxLayout *layout);
	
	/**
	 *Creates and sets up the frameview.
	 *@param parent the widget the frameview will be inserted into.
	 */
	void makeViews(QHBoxLayout *layout);
	
	/**
	 *Creates and sets up the toolsmenu.
	 *@param parent the widget the toolsmenu will be inserted into.
	 */
	void makeToolsMenu(QHBoxLayout *layout);
	
	/**
	 * Creates and sets up the menu for going to a specified framenumber.
	 * @param parent the widget the gotomenu will be inserted into.
	 */
	void makeGotoMenu(QVBoxLayout *layout);
	
	/**
	 * Sets up the statusbar with custom widgets.
	 */
	void makeStatusBar();
		
	/**
	 *Overloaded event listener for when when a drag enters the application.
	 *@param event information about the dragEnterEvent
	 */
	void dragEnterEvent ( QDragEnterEvent * event);
	
	/**
	 *Overloaded event listener for when a drop event occur in the application.
	 *@param event information about the dropEvent
	 */
	void dropEvent(QDropEvent *event);
	
	/**
	 * Overloaded event listener which recieves information when a keyboard key is
	 * released
	 * @param k information about the key event. 
	 */
	void keyReleaseEvent ( QKeyEvent * k );
	
	/**
	 * Retranslates the tooltip and whatsthis text.
	 *
	 * This function is called from retranslateStrings.
	 */
	void retranslateHelpText();
	
	/**
	 * Changes the project history so that they shows correctly in the
	 * most recent menu.
	 */
	void setMostRecentProject();
	
	/**
	 * Updates the most recent menu.
	 */
	void updateMostRecentMenu();
	
private slots:
	
	/**
	 * Retranslates the strings.
	 *
	 * This function is called after a new translator has been installed so that 
	 * the program strings are retranslated to the new language.
	 */
	void retranslateStrings();
	
	/**
	 * Creates a new project.
	 */
	void newProject();
	
	/**
	 * Opens a project.
	 */
	void openProject();
	
	/**
	 * Opens a saved project.
	 * @param projectFile the project to open
	 */
	void openProject(const char *projectFile);
	
	/**
	 * Opens a project.
	 */
	void openMostRecent();
	void openSecondMostRecent();
	void openThirdMostRecent();
	
	/**
	 * Saves the project to the last saved file.
	 */
	void saveProject();
	
	/**
	 * Saves the project to a given filename from the user.
	 */
	void saveProjectAs();
	
	/**
	 * Brings up an about dialog with information about the application.
	 */
	void showAboutDialog();
	
	/**
	 * Brings up an help dialog with the stopmotion user manua.
	 */
	void showHelpDialog();
	
	/**
	 * Brings up a preferences menu where the user can set preferences for the
	 * application.
	 */
	void showPreferencesMenu();
	
	/**
	 * Exports the current project to a video file choosen by the user. It uses the
	 * registered video encoder -- if registered -- to export.
	 */
	void exportToVideo();
	
	/**
	 * Exports the current project to a valid Cinerella project.
	 */
	void exportToCinerella();

	void whatsThis();
	
public slots:
	/**
	 * This slot is notified when the size of the model changes so that menuframe
	 * menu options can be adjusted (activated/deactivated, etc).
	 * @param modelSize the new size of the model.
	 */
	void modelSizeChanged( int modelSize );
	
	/**
	 * Sets differents buttons such as undo, save as and copy to be enabled.
	 */
	void activateMenuOptions();
};

#endif
