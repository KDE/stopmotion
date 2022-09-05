/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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

#ifndef QTGUI_H
#define QTGUI_H

#include "src/domain/undo/undoredoobserver.h"

#include <QObject>
#include <QMainWindow>

class FrameBar;
class FrameView;
class FlexibleSpinBox;
class CameraHandler;
class EditMenuHandler;
class RunAnimationHandler;
class ModelHandler;
class SoundHandler;
class FramePreferencesMenu;
class PreferencesMenu;
class ToolsMenu;
class LanguageHandler;
class EditObserver;

class QApplication;
class QKeyEvent;
class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;
class QAction;
class QMenu;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QFileSystemWatcher;

/**
 *The main window class for the stopmotion application.
 *This class sets up the main GUI and connects the buttons.
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
*/
class MainWindowGUI : public QMainWindow, public UndoRedoObserver {
	Q_OBJECT
public:
	enum {SAVE, SAVE_AS, UNDO, REDO, CUT, COPY, PASTE, GOTO};

	/**
	 * Sets up the program main GUI and connect the widgets and handlers.
	 * @param stApp the application class for changing translator
	 * on runtime.
	 */
	MainWindowGUI(QApplication *stApp);
	~MainWindowGUI();

	/**
	 * Finish constructing the UI. The preferences must be loaded before calling this.
	 */
	void ConstructUI();

	/**
	 * Overloaded mouse listener. Closes the embedded menues when the user
	 * clicks inside the main window.
	 * @param e information about the event.
	 */
	void mousePressEvent ( QMouseEvent * e );

	/**
	 * Overloaded event listener which receives information when a keyboard key is
	 * pressed.
	 * @param k information about the key event.
	 */
	void keyPressEvent( QKeyEvent *k );

	void updateCanUndo(bool newCanUndo);
	void updateCanRedo(bool newCanRedo);

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
	QFileSystemWatcher *fileWatcher;
	EditObserver *editObserver;

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

	//Menus
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

	QString lastVisitedDir;

	enum SaveDialogResult {
		saveDialogSave,
		saveDialogDiscard,
		saveDialogCancel
	};

	/**
	 * Creates the handlers for handling user requests.
	 * @param stApp the application class for changing translator
	 * on runtime.
	 */
	void createHandlers(QApplication *stApp);

	/**
	 * Begins monitoring the workspace directory for interesting changes.
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
	 * Overloaded event listener which receives information when a keyboard key is
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
	 * Updates the most recent menu.
	 */
	void updateMostRecentMenu();

	/**
	 * Sets the title to the project name and an indicator for whether the
	 * project is saved or not.
	 * @param modified Whether or not the project is different from the most
	 * recently saved project.
	 */
	void setTitle(bool modified);

	/**
	 * If the project has unsaved changes, asks the user if the project should
	 * be saved. If so, saves it.
	 * @return The user's response. {@ref SaveDialogDiscard} is returned if
	 * there were found to be no changes.
	 */
	SaveDialogResult saveIfNecessary();
	void doOpenProject(const char* projectFile);

public slots:

	/**
	 * Opens a saved project.
	 * @param projectFile the project to open
	 */
	void openProject(const char *projectFile);

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
	 * Opens a project.
	 */
	void openMostRecent();
	void openSecondMostRecent();
	void openThirdMostRecent();

	/**
	 * Saves the project to the last saved file.
	 * @return {@c false} if a filename was required but the user cancelled;
	 * in this case the project was not saved.
	 */
	bool saveProject();

	/**
	 * Saves the project to a given filename from the user.
	 * @return {@c false} if the user cancelled; in this case the project was
	 * not saved.
	 */
	bool saveProjectAs();

	/**
	 * Brings up an about dialog with information about the application.
	 */
	void showAboutDialog();

	/**
	 * Brings up an help dialog with the stopmotion user manual.
	 */
	void showHelpDialog();

	/**
	 * Brings up a preferences menu where the user can set preferences for the
	 * application.
	 */
	void showPreferencesMenu();

	/**
	 * Exports the current project to a video file chosen by the user. It uses the
	 * registered video encoder -- if registered -- to export.
	 */
	void exportToVideo();

	/**
	 * Exports the current project to a valid Cinerella project.
	 */
	void exportToCinerella();

	void whatsThis();

	/**
	 * Notification that the framebar's active frame has changed.
	 */
	void updateNewActiveFrame(int scene, int frame);

	/**
	 * Notification that the clipboard contents have changed.
	 */
	void updatePasteEnabled();
	void createMostRecentMenu();

public slots:
	/**
	 * This slot is notified when the size of the model changes so that menuframe
	 * menu options can be adjusted (activated/deactivated, etc).
	 * @param modelSize the new size of the model.
	 */
	void modelSizeChanged( int modelSize );

	/**
	 * Sets different buttons such as undo, save as and copy to be enabled.
	 */
	void activateMenuOptions();
};

#endif
