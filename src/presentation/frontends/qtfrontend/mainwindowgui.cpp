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
#include "mainwindowgui.h"

#include "logger.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"
#include "src/presentation/frontends/qtfrontend/frameview.h"
#include "src/presentation/frontends/qtfrontend/flexiblespinbox.h"
#include "src/application/camerahandler.h"
#include "src/application/editmenuhandler.h"
#include "src/application/runanimationhandler.h"
#include "src/application/modelhandler.h"
#include "src/application/languagehandler.h"
#include "src/application/soundhandler.h"
#include "src/presentation/frontends/qtfrontend/framepreferencesmenu.h"
#include "src/presentation/frontends/qtfrontend/preferencesmenu.h"
#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "src/technical/video/videoencoder.h"
#include "src/presentation/frontends/qtfrontend/toolsmenu.h"
#include "src/presentation/frontends/qtfrontend/helpwindow.h"
#include "src/presentation/frontends/qtfrontend/aboutdialog.h"
#include "src/presentation/frontends/qtfrontend/editobserver.h"

#include "graphics/icons/windowicon.xpm"
#include "graphics/icons/configureicon.xpm"
#include "graphics/icons/undoicon.xpm"
#include "graphics/icons/redoicon.xpm"
#include "graphics/icons/cuticon.xpm"
#include "graphics/icons/copyicon.xpm"
#include "graphics/icons/pasteicon.xpm"
#include "graphics/icons/filenewicon.xpm"
#include "graphics/icons/fileopenicon.xpm"
#include "graphics/icons/filesaveicon.xpm"
#include "graphics/icons/filesaveasicon.xpm"
#include "graphics/icons/quiticon.xpm"
#include "graphics/icons/whatsthisicon.xpm"
#include "graphics/icons/close.xpm"
#include "graphics/icons/videoexport.xpm"
#include "graphics/icons/languages.xpm"

#include <QShortcut>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QWhatsThis>
#include <QFileSystemWatcher>
#include <QClipboard>

#include <cassert>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <string>

using namespace std;
using namespace Qt;

MainWindowGUI::MainWindowGUI(QApplication *stApp)
	: stApp(stApp), centerWidget(nullptr), centerWidgetLayout(nullptr), bottomWidget(nullptr),
	  bottomWidgetLayout(nullptr), workArea(nullptr), workAreaLayout(nullptr),
	  frameBar(nullptr), frameView(nullptr), gotoMenuWidget(nullptr), gotoMenuWidgetLayout(nullptr),
	  fileWatcher(nullptr), editObserver(nullptr),
	  newAct(nullptr), openAct(nullptr), mostRecentAct(nullptr), secondMostRecentAct(nullptr),
	  thirdMostRecentAct(nullptr), saveAct(nullptr), saveAsAct(nullptr), videoAct(nullptr), cinerellaAct(nullptr),
	  quitAct(nullptr), undoAct(nullptr), redoAct(nullptr), cutAct(nullptr), copyAct(nullptr), pasteAct(nullptr),
	  gotoFrameAct(nullptr), configureAct(nullptr), whatsthisAct(nullptr), aboutAct(nullptr),
	  helpAct(nullptr),
	  fileMenu(nullptr), exportMenu(nullptr), mostRecentMenu(nullptr), editMenu(nullptr),
	  settingsMenu(nullptr), languagesMenu(nullptr), helpMenu(nullptr), toolsMenu(nullptr),
	  framePreferencesMenu(nullptr), preferencesMenu(nullptr),
	  gotoMenuCloseButton(nullptr), numberDisplay(nullptr), gotoSpinner(nullptr),
	  gotoFrameLabel(nullptr),
	  modelHandler(nullptr), soundHandler(nullptr), cameraHandler(nullptr), editMenuHandler(nullptr),
	  languageHandler(nullptr), runAnimationHandler(nullptr),
	  lastVisitedDir(getenv("PWD")) {

	centerWidget = new QWidget;
	centerWidget->setObjectName("CenterWidget");
	centerWidgetLayout = new QVBoxLayout;
	centerWidgetLayout->setSpacing(5);
	centerWidgetLayout->setContentsMargins(0, 0, 0, 0);

	frameBar = new FrameBar;
	centerWidgetLayout->addWidget(frameBar);
}


void MainWindowGUI::ConstructUI() {
	createHandlers(stApp);
	createAccelerators();
	bottomWidget = new QWidget;
	bottomWidget->setObjectName("BottomWidget");
	centerWidgetLayout->addWidget(bottomWidget);
	bottomWidgetLayout = new QVBoxLayout;
	bottomWidgetLayout->setSpacing(0);
	bottomWidgetLayout->setContentsMargins(0, 0, 0, 0);
	makePreferencesMenu(bottomWidgetLayout);
	//Initializes and sets up the workarea consisting of the toolsmenu and the frameview.
	workArea = new QWidget;
	workArea->setObjectName("WorkArea");
	bottomWidgetLayout->addWidget(workArea);
	bottomWidget->setLayout(bottomWidgetLayout);
	workAreaLayout = new QHBoxLayout;
	workAreaLayout->setObjectName("WorkAreaLayout");
	workAreaLayout->setSpacing(5);
	workAreaLayout->setContentsMargins(0, 0, 0, 0);
	makeToolsMenu(workAreaLayout);
	makeViews(workAreaLayout);
	workArea->setLayout(workAreaLayout);
	connect(runAnimationHandler, &RunAnimationHandler::playFrame, frameView,
			&FrameView::updatePlayFrame);
	connect(frameBar, qOverload<int, int>(&FrameBar::newActiveFrame),
			runAnimationHandler, &RunAnimationHandler::stopAnimation);
	makeGotoMenu(centerWidgetLayout);
	centerWidget->setLayout(centerWidgetLayout);
	setCentralWidget(centerWidget);
	makeStatusBar();
	//Initializes and sets up the menu system.
	createActions();
	createMenus();
	//These slots will activate/deactivate menu options based on the changes in the model.
	connect(frameBar, &FrameBar::modelSizeChanged, this,
			&MainWindowGUI::modelSizeChanged);
	connect(frameBar, qOverload<int, int>(&FrameBar::newActiveFrame), this,
			&MainWindowGUI::updateNewActiveFrame);
	// update paste menu item depending on what's on the clipboard
	connect(QApplication::clipboard(), &QClipboard::dataChanged, this,
			&MainWindowGUI::updatePasteEnabled);
	setupDirectoryMonitoring();
	//Mainwindow preferences.
	setWindowIcon(QPixmap(windowicon));
	setContentsMargins(5, 0, 5, 0);
	setAcceptDrops(true);
	statusBar()->showMessage(tr("Ready to rumble ;-)"), 2000);
	statusBar()->setSizeGripEnabled(false);

	retranslateStrings();

	DomainFacade* facade = DomainFacade::getFacade();
	setTitle(facade->canUndo());
	facade->setUndoRedoObserver(this);
}


MainWindowGUI::~MainWindowGUI() {
	delete editObserver;
}


void MainWindowGUI::createHandlers(QApplication *stApp) {
	languageHandler = new LanguageHandler( this, stApp );
	connect( languageHandler, &LanguageHandler::languageChanged, this, &MainWindowGUI::retranslateStrings );

	runAnimationHandler = new RunAnimationHandler(this, statusBar(), frameBar);
	connect(runAnimationHandler, &RunAnimationHandler::stopped,
			frameBar, qOverload<int, int, int>(&FrameBar::setSelection));

	modelHandler = new ModelHandler( this, this->statusBar(), frameBar, &lastVisitedDir );
	connect( modelHandler, &ModelHandler::modelChanged, this, &MainWindowGUI::activateMenuOptions );

	cameraHandler = new CameraHandler( this, this->statusBar(), modelHandler );

	editMenuHandler = new EditMenuHandler( this, this->statusBar(), frameBar );
	connect( editMenuHandler, &EditMenuHandler::addFrames,
			modelHandler, &ModelHandler::addFrames );
	connect(editMenuHandler, &EditMenuHandler::removeFrames,
			modelHandler, &ModelHandler::removeFrames);
	connect(editMenuHandler, &EditMenuHandler::undoOrRedo,
			this, &MainWindowGUI::activateMenuOptions);

	soundHandler = new SoundHandler( this, this->statusBar(), frameBar,
			this->lastVisitedDir );
}


void MainWindowGUI::setupDirectoryMonitoring() {
	fileWatcher = new QFileSystemWatcher(this);
	editObserver = new EditObserver(fileWatcher);
	connect(fileWatcher, &QFileSystemWatcher::fileChanged,
			frameBar, &FrameBar::fileChanged);
	connect(fileWatcher, &QFileSystemWatcher::fileChanged,
			frameView, &FrameView::fileChanged);
	DomainFacade::getFacade()->attach(editObserver);
}


void MainWindowGUI::createAccelerators()
{
	QShortcut *nextFrameAccel = new QShortcut(QKeySequence(Qt::Key_L), this);
	connect(nextFrameAccel, &QShortcut::activated, frameBar, &FrameBar::selectNextFrame);
	QShortcut *nextFrameAccel2 = new QShortcut(QKeySequence(Qt::Key_Right), this);
	connect(nextFrameAccel2, &QShortcut::activated, frameBar, &FrameBar::selectNextFrame);
	QShortcut *nextFrameSelectionAccel = new QShortcut(
			QKeySequence(Qt::ShiftModifier | Qt::Key_L), this);
	connect(nextFrameSelectionAccel, &QShortcut::activated,
			frameBar, &FrameBar::moveSelectionToNextFrame);
	QShortcut *nextFrameSelectionAccel2 = new QShortcut(
			QKeySequence(Qt::ShiftModifier | Qt::Key_Right), this);
	connect(nextFrameSelectionAccel2, &QShortcut::activated,
			frameBar, &FrameBar::moveSelectionToNextFrame);

	QShortcut *previousFrameAccel = new QShortcut(QKeySequence(Qt::Key_J), this);
	connect(previousFrameAccel, &QShortcut::activated, frameBar, &FrameBar::selectPreviousFrame);
	QShortcut *previousFrameAccel2 = new QShortcut(QKeySequence(Qt::Key_Left), this );
	connect(previousFrameAccel2, &QShortcut::activated, frameBar, &FrameBar::selectPreviousFrame);
	QShortcut *previousFrameSelectionAccel = new QShortcut(
			QKeySequence(Qt::ShiftModifier | Qt::Key_J), this);
	connect(previousFrameSelectionAccel, &QShortcut::activated,
			frameBar, &FrameBar::moveSelectionToPreviousFrame);
	QShortcut *previousFrameSelectionAccel2 = new QShortcut(
			QKeySequence(Qt::ShiftModifier | Qt::Key_Left), this);
	connect(previousFrameSelectionAccel2, &QShortcut::activated,
			frameBar, &FrameBar::moveSelectionToPreviousFrame);

	QShortcut *nextSceneAccel = new QShortcut(QKeySequence(Qt::Key_O), this);
	connect(nextSceneAccel, &QShortcut::activated, frameBar, &FrameBar::selectNextScene);

	QShortcut *prevSceneAccel = new QShortcut(QKeySequence(Qt::Key_I), this);
	connect(prevSceneAccel, &QShortcut::activated, frameBar, &FrameBar::selectPreviousScene);

	QShortcut *toggleCameraAccel = new QShortcut(QKeySequence(Qt::Key_C), this);
	connect(toggleCameraAccel, &QShortcut::activated, cameraHandler, &CameraHandler::toggleCamera);

	QShortcut *captureAccel = new QShortcut(QKeySequence(Qt::Key_Space), this);
	connect(captureAccel, &QShortcut::activated, cameraHandler, &CameraHandler::captureFrame);

	QShortcut *addFrameAccel = new QShortcut(QKeySequence(Qt::Key_F), this);
	connect(addFrameAccel, &QShortcut::activated, modelHandler, &ModelHandler::chooseFrame);

	QShortcut *newSceneAccel = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_E), this);
	connect(newSceneAccel, &QShortcut::activated, modelHandler, &ModelHandler::newScene);

	QShortcut *removeFramesAccel = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	connect(removeFramesAccel, &QShortcut::activated, modelHandler, &ModelHandler::removeFrames);

	QShortcut *removeSceneAccel = new QShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete), this);
	connect(removeSceneAccel, &QShortcut::activated, modelHandler, &ModelHandler::removeScene);
}


void MainWindowGUI::createActions()
{
	//File menu
	newAct = new QAction(this);
	newAct->setIcon(QPixmap(filenewicon));
	newAct->setShortcut(QKeySequence(ControlModifier | Key_N));
	connect(newAct, &QAction::triggered, this, &MainWindowGUI::newProject);

	openAct = new QAction(this);
	openAct->setIcon(QPixmap(fileopenicon));
	openAct->setShortcut(QKeySequence(ControlModifier | Key_O));
	connect(openAct, &QAction::triggered, this, qOverload<>(&MainWindowGUI::openProject));

	mostRecentAct = new QAction(this);
	mostRecentAct->setIcon(QPixmap(windowicon));
	connect(mostRecentAct, &QAction::triggered, this, &MainWindowGUI::openMostRecent);

	secondMostRecentAct = new QAction(this);
	secondMostRecentAct->setIcon(QPixmap(windowicon));
	connect(secondMostRecentAct, &QAction::triggered, this, &MainWindowGUI::openSecondMostRecent);

	thirdMostRecentAct = new QAction(this);
	thirdMostRecentAct->setIcon(QPixmap(windowicon));
	connect(thirdMostRecentAct, &QAction::triggered, this, &MainWindowGUI::openThirdMostRecent);

	saveAct = new QAction(this);
	saveAct->setIcon(QPixmap(filesaveasicon));
	saveAct->setShortcut(QKeySequence(ControlModifier | Key_S));
	connect(saveAct, &QAction::triggered, this, &MainWindowGUI::saveProject);

	saveAsAct = new QAction(this);
	saveAsAct->setIcon(QPixmap(filesaveicon));
	saveAsAct->setShortcut(QKeySequence(ControlModifier | ShiftModifier | Key_S));
	connect(saveAsAct, &QAction::triggered, this, &MainWindowGUI::saveProjectAs);

	videoAct = new QAction(this);
	videoAct->setShortcut(QKeySequence(ControlModifier | AltModifier | Key_V));
	videoAct->setIcon(QPixmap(videoexport));
	connect(videoAct, &QAction::triggered, this, &MainWindowGUI::exportToVideo);

	cinerellaAct = new QAction(this);
	cinerellaAct->setShortcut(QKeySequence(ControlModifier | AltModifier | Key_C));
	cinerellaAct->setEnabled(false);
	connect(cinerellaAct, &QAction::triggered, this, &MainWindowGUI::exportToCinerella);

	quitAct = new QAction(this);
	quitAct->setIcon(QPixmap(quiticon));
	quitAct->setShortcut(QKeySequence(ControlModifier | Key_Q));
	connect(quitAct, &QAction::triggered, stApp, &QCoreApplication::quit);

	//Edit menu
	undoAct = new QAction(this);
	undoAct->setIcon(QPixmap(undoicon));
	undoAct->setShortcut(QKeySequence(ControlModifier | Key_Z));
	connect(undoAct, &QAction::triggered, editMenuHandler, &EditMenuHandler::undo);

	redoAct = new QAction(this);
	redoAct->setIcon(QPixmap(redoicon));
	redoAct->setShortcut(QKeySequence(ControlModifier | ShiftModifier | Key_Z));
	connect(redoAct, &QAction::triggered, editMenuHandler, &EditMenuHandler::redo);

	cutAct = new QAction(this);
	cutAct->setIcon(QPixmap(cuticon));
	cutAct->setShortcut(QKeySequence(ControlModifier | Key_X));
	connect(cutAct, &QAction::triggered, editMenuHandler, &EditMenuHandler::cut);

	copyAct = new QAction(this);
	copyAct->setIcon(QPixmap(copyicon));
	copyAct->setShortcut(QKeySequence(ControlModifier | Key_C));
	connect(copyAct, &QAction::triggered, editMenuHandler, &EditMenuHandler::copy);

	pasteAct = new QAction(this);
	pasteAct->setIcon(QPixmap(pasteicon));
	pasteAct->setShortcut(QKeySequence(ControlModifier | Key_V));
	connect(pasteAct, &QAction::triggered, editMenuHandler, &EditMenuHandler::paste);

	gotoFrameAct = new QAction(this);
	gotoFrameAct->setShortcut(QKeySequence(ControlModifier | Key_G));
	connect(gotoFrameAct, &QAction::triggered, gotoMenuWidget, &QWidget::show);

	configureAct = new QAction(this);
	configureAct->setIcon(QPixmap(configureicon));
	configureAct->setShortcut(QKeySequence(ControlModifier | Key_P));
	connect(configureAct, &QAction::triggered, this, &MainWindowGUI::showPreferencesMenu);

	//Help menu
	whatsthisAct = new QAction(this);
	whatsthisAct->setIcon(QPixmap(whatsthisicon));
	whatsthisAct->setShortcut(QKeySequence(ShiftModifier | Key_F1));
	connect(whatsthisAct, &QAction::triggered, this, &MainWindowGUI::whatsThis);

	helpAct = new QAction(this);
	helpAct->setShortcut(QKeySequence(Key_F1));
	connect(helpAct, &QAction::triggered, this, &MainWindowGUI::showHelpDialog);

	aboutAct = new QAction(this);
	aboutAct->setIcon(QPixmap(windowicon));
	connect(aboutAct, &QAction::triggered, this, &MainWindowGUI::showAboutDialog);
}


void MainWindowGUI::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	exportMenu = fileMenu->addMenu(tr("&Export"));
	mostRecentMenu = fileMenu->addMenu(tr("Open &Recent"));

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(undoAct);
	editMenu->addAction(redoAct);
	editMenu->addSeparator();
	editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);
	editMenu->addSeparator();
	editMenu->addAction(gotoFrameAct);

	undoAct->setEnabled(false);
	redoAct->setEnabled(false);
	cutAct->setEnabled(false);
	copyAct->setEnabled(false);
	updatePasteEnabled();
	gotoFrameAct->setEnabled(false);

	settingsMenu = menuBar()->addMenu(tr("&Settings"));
	languagesMenu = languageHandler->createLanguagesMenu(settingsMenu);

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(whatsthisAct);
	helpMenu->addAction(helpAct);
	helpMenu->addSeparator();
	helpMenu->addAction(aboutAct);
}


void MainWindowGUI::makeToolsMenu(QHBoxLayout *layout)
{
	toolsMenu = new ToolsMenu(runAnimationHandler, modelHandler, cameraHandler,
			frameBar);
	layout->addWidget(toolsMenu);

	connect(frameBar, &FrameBar::modelSizeChanged,
			toolsMenu, &ToolsMenu::fixNavigationButtons);
	connect(cameraHandler, &CameraHandler::cameraStateChanged,
			toolsMenu, &ToolsMenu::cameraOn);
}


void MainWindowGUI::makePreferencesMenu(QVBoxLayout *layout)
{
	framePreferencesMenu = new FramePreferencesMenu(nullptr, soundHandler, frameBar);
	layout->addWidget(framePreferencesMenu);
	frameBar->setPreferencesMenu(framePreferencesMenu);
	framePreferencesMenu->hide();

	preferencesMenu = new PreferencesMenu(this);
	preferencesMenu->hide();
}


void MainWindowGUI::makeGotoMenu(QVBoxLayout *layout)
{
	gotoMenuWidget = new QWidget;
	gotoMenuWidgetLayout = new QHBoxLayout;
	gotoMenuWidgetLayout->setSpacing(5);
	gotoMenuWidgetLayout->setContentsMargins(0, 0, 0, 0);

	gotoFrameLabel = new QLabel;

	gotoSpinner = new FlexibleSpinBox;
	gotoSpinner->setMaximumWidth(60);
	gotoSpinner->setRange(1, 1);

	connect(frameBar, qOverload<int, int>(&FrameBar::newActiveFrame),
			gotoSpinner, &FlexibleSpinBox::setIndexSecond);
	connect(frameBar, &FrameBar::modelSizeChanged,
			gotoSpinner, &FlexibleSpinBox::setMaximumValue);
	connect(frameBar, &FrameBar::newMaximumValue, gotoSpinner, &FlexibleSpinBox::setMaximumValue);
	connect(gotoSpinner, &FlexibleSpinBox::spinBoxTriggered, editMenuHandler, &EditMenuHandler::gotoFrame);
	connect(gotoSpinner, &FlexibleSpinBox::spinBoxCanceled,editMenuHandler, &EditMenuHandler::closeGotoMenu);

	gotoMenuCloseButton = new QPushButton;
	gotoMenuCloseButton->setIcon(QPixmap(closeicon));
	gotoMenuCloseButton->setFlat(true);
	gotoMenuCloseButton->setMinimumSize(16, 16);
	gotoMenuCloseButton->setMaximumSize(16, 16);
	connect(gotoMenuCloseButton, &QAbstractButton::clicked, gotoMenuWidget, &QWidget::hide);

	gotoMenuWidgetLayout->addWidget(gotoFrameLabel);
	gotoMenuWidgetLayout->addWidget(gotoSpinner);
	gotoMenuWidgetLayout->addStretch();
	gotoMenuWidgetLayout->addWidget(gotoMenuCloseButton);
	gotoMenuWidget->setLayout(gotoMenuWidgetLayout);

	layout->addWidget(gotoMenuWidget);
	gotoMenuWidget->hide();
	editMenuHandler->setGotoMenu(gotoMenuWidget);
}


void MainWindowGUI::makeStatusBar()
{
	numberDisplay = new QLabel;
	numberDisplay->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	numberDisplay->setContentsMargins(0, 0, 0, 0);
	connect(frameBar, qOverload<const QString &>(&FrameBar::newActiveFrame),
			numberDisplay, &QLabel::setText);
	this->statusBar()->addPermanentWidget(numberDisplay);
}


void MainWindowGUI::makeViews(QHBoxLayout *layout)
{
	frameView = new FrameView;
	frameView->setObjectName("FrameView");
	frameView->setMinimumSize(400, 300);
	frameView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->addWidget(frameView);

	connect( frameView, &FrameView::cameraReady, cameraHandler, &CameraHandler::switchToVideoView);
	connect(cameraHandler, &CameraHandler::capturedFrame, this, &MainWindowGUI::activateMenuOptions);

	cameraHandler->setFrameView(frameView);
	connect(frameBar, qOverload<int, int>(&FrameBar::newActiveFrame),
			frameView, &FrameView::updateNewActiveFrame);
	connect(stApp, &QCoreApplication::aboutToQuit, frameView, &FrameView::off);
}


void MainWindowGUI::retranslateStrings()
{
	Logger::get().logDebug("Retranslating strings");

	//The actions caption texts
	newAct->setText(tr("&New"));
	openAct->setText(tr("&Open"));
	saveAct->setText(tr("&Save"));
	saveAsAct->setText(tr("Save &As"));
	videoAct->setText(tr("Video"));
	cinerellaAct->setText(tr("Cinelerra"));
	quitAct->setText(tr("&Quit"));
	undoAct->setText(tr("&Undo"));
	redoAct->setText(tr("Re&do"));
	cutAct->setText(tr("Cu&t"));
	copyAct->setText(tr("&Copy"));
	pasteAct->setText(tr("&Paste"));
	gotoFrameAct->setText(tr("&Go to frame"));
	configureAct->setText(tr("&Configure Stopmotion"));
	whatsthisAct->setText(tr("What's &This"));
	helpAct->setText(tr("&Help"));
	aboutAct->setText(tr("&About"));

	//Other widgets
	numberDisplay->setText(tr("Frame number: ") +
			QString("%1").arg(frameBar->getActiveFrame() + 1));
	gotoFrameLabel->setText(tr("Go to frame:"));

	//Tooltip and whatsthis texts
	retranslateHelpText();

	//The menus
	menuBar()->clear();
	menuBar()->addMenu(fileMenu);

	fileMenu->clear();
	fileMenu->setTitle(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addMenu(mostRecentMenu);

	createMostRecentMenu();

	fileMenu->addSeparator();
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addMenu(exportMenu);

	exportMenu->clear();
	exportMenu->setTitle(tr("&Export"));
	exportMenu->addAction(videoAct);
	exportMenu->addAction(cinerellaAct);

	fileMenu->addSeparator();
	fileMenu->addAction(quitAct);

	menuBar()->addMenu(editMenu);
	menuBar()->addMenu(settingsMenu);

	settingsMenu->clear();
	settingsMenu->setTitle(tr("&Settings"));
	languagesMenu->setIcon(QPixmap(languages));
	settingsMenu->addMenu(languagesMenu);
	settingsMenu->addAction(configureAct);

	editMenu->setTitle(tr("&Edit"));

	helpMenu->setTitle(tr("&Help"));
	menuBar()->addMenu(helpMenu);

	//The submenus
	toolsMenu->retranslateStrings();
	framePreferencesMenu->retranslateStrings();
	preferencesMenu->retranslateStrings();
}


void MainWindowGUI::retranslateHelpText()
{
	QString infoText;

	//File menu
	infoText =
			tr("<h4>New</h4> "
			"<p>Creates a <em>new</em> project.</p>");
	newAct->setWhatsThis(infoText);
	//The prepend part is a trick to keep the accelerator in the tooltip
	infoText =
			newAct->toolTip().prepend(tr("New project"));
	newAct->setToolTip(infoText);

	infoText =
			tr("<h4>Open</h4> "
			"<p><em>Opens</em> a Stopmotion project file.</p>");
	openAct->setWhatsThis(infoText);
	infoText =
			openAct->toolTip().prepend(tr("Open project"));
	openAct->setToolTip(infoText);

	infoText =
			tr("<h4>Save</h4> "
			"<p><em>Saves</em> the current animation as a Stopmotion "
			"project file. <BR>If this project has been saved before it will "
			"automatically be saved to the previously selected file.</p>");
	saveAct->setWhatsThis(infoText);
	infoText =
			saveAct->toolTip().prepend(tr("Save project"));
	saveAct->setToolTip(infoText);

	infoText =
			tr("<h4>Save As</h4> "
			"<p><em>Saves</em> the current animation as a Stopmotion "
			"project file.</p>");
	saveAsAct->setWhatsThis(infoText);
	infoText =
			saveAsAct->toolTip().prepend(tr("Save project As"));
	saveAsAct->setToolTip(infoText);

	infoText =
			tr("<h4>Video</h4> "
			"<p>Exports the current project as <em>video</em>.</p>"
			"You will be given a wizard to guide you.");
	videoAct->setWhatsThis(infoText);
	videoAct->setToolTip(infoText);

	infoText =
			tr("<h4>Cinerella</h4> "
			"<p>Exports the current animation as a <em>Cinerella</em> project.</p>"
			"You will be given a wizard to guide you.");
	cinerellaAct->setWhatsThis(infoText);
	cinerellaAct->setToolTip(infoText);

	infoText =
			tr("<h4>Quit</h4> "
			"<p><em>Quits</em> the program.</p>");
	quitAct->setWhatsThis(infoText);
	infoText =
			quitAct->toolTip().prepend(tr("Quit"));
	quitAct->setToolTip(infoText);


	//Edit menu
	infoText =
			tr("<h4>Undo</h4> "
			"<p><em>Undoes</em> your last operation. You can press undo "
			"several time to undo earlier operations.</p>");
	undoAct->setWhatsThis(infoText);
	infoText =
			undoAct->toolTip().prepend(tr("Undo"));
 	undoAct->setToolTip(infoText);

	infoText =
			tr("<h4>Redo</h4> "
			"<p><em>Redoes</em> your last operation. You can press redo "
			"several times to redo several operations.</p>");
	redoAct->setWhatsThis(infoText);
	infoText =
			redoAct->toolTip().prepend(tr("Redo"));
	redoAct->setToolTip(infoText);

	infoText =
			tr("<h4>Cut</h4> "
			"<p><em>Cuts</em> the selected frames out of the animation and adds them "
			"to the clipboard so that you can paste them in somewhere else.</p>");
	cutAct->setWhatsThis(infoText);
	infoText =
			cutAct->toolTip().prepend(tr("Cut"));
	cutAct->setToolTip(infoText);

	infoText =
			tr("<h4>Copy</h4> "
			"<p><em>Copies</em> the selected frames to the clipboard. You can "
			"then paste them in another place.</p>");
	copyAct->setWhatsThis(infoText);
	infoText =
			copyAct->toolTip().prepend(tr("Copy"));
	copyAct->setToolTip(infoText);

	infoText =
			tr("<h4>Paste</h4> "
			"<p><em>Pastes</em> the frames which are currently in the clipboard "
			"into the selected location.</p> <p>You can copy/cut images from another "
			"programs and then use this option to paste them into this animation.</p>");
	pasteAct->setWhatsThis(infoText);
	infoText =
			pasteAct->toolTip().prepend(tr("Paste"));
	pasteAct->setToolTip(infoText);

	infoText =
			tr("<h4>Go to frame</h4> "
			"<p>This will bring up a popup-menu at the bottom where you can choose "
			"a frame you want to <em>go to</em>.</p>");
	gotoFrameAct->setWhatsThis(infoText);
	infoText =
			gotoFrameAct->toolTip().prepend(tr("Go to frame"));
	gotoFrameAct->setToolTip(infoText);

	infoText =
			tr("<h4>Configure Stopmotion</h4> "
			"<p>This will open a window where you can <em>configure</em> "
			"Stopmotion with various input and output devices.</p>");
	configureAct->setWhatsThis(infoText);
	infoText =
			configureAct->toolTip().prepend(tr("Configure Stopmotion"));
	configureAct->setToolTip(infoText);


	//Help menu
	infoText =
		tr("<h4>What's This</h4> "
		"<p>This will give you a WhatsThis mouse cursor which can be used to "
		"bring up helpful information like this.</p>");
	whatsthisAct->setWhatsThis(infoText);
	infoText =
			whatsthisAct->toolTip().prepend(tr("What's This"));
	whatsthisAct->setToolTip(infoText);

	infoText =
			tr("<h4>Help</h4> "
			"<p>This button will bring up a dialog with the Stopmotion manual</p>");
	helpAct->setWhatsThis(infoText);
	infoText =
			aboutAct->toolTip().prepend(tr("Help"));
	helpAct->setToolTip(infoText);

	infoText =
			tr("<h4>About</h4> "
			"<p>This will display a small information box where you can read "
			"general information as well as the names of the developers "
			"behind this excellent piece of software.</p>");
	aboutAct->setWhatsThis(infoText);
	infoText =
			aboutAct->toolTip().prepend(tr("About"));
	aboutAct->setToolTip(infoText);


	//Other widgets
	infoText =
			tr("<h4>Frame number</h4><p>This area displays the number "
			"of the currently selected frame</p>");
	numberDisplay->setToolTip(infoText );
	numberDisplay->setWhatsThis(infoText );

	//Various menues
	infoText =
			tr("<h4>FrameView</h4><p> In this area you can see the "
			"selected frame. You can also play "
			"animations in this window by pressing the "
			"<b>Play</b> button.</p>");
	frameView->setWhatsThis(infoText );

	infoText =
			tr("<h4>Go to frame menu</h4> "
			"<p>Here you can specify a framenumber and the program will jump "
			"to the specified frame</p> ");
    gotoMenuWidget->setWhatsThis(infoText );

	infoText =
			tr("<h4>Frame preferences menu</h4> "
			"<p>In this menu you can set preferences for the "
			"selected frame/frames, such as <b>subtitles</b>, "
			"<b>sound effects</b>, etc.</p>");
	framePreferencesMenu->setWhatsThis(infoText );

	infoText =
			tr("<h4>Tool menu</h4> "
			"<p>This is the tool menu where most of the buttons and widgets "
			"you will need when working on stop motion animations are located.</p>");
	toolsMenu->setWhatsThis(infoText );

	infoText =
			tr("<h4>FrameBar</h4> "
			"<p>In this area you can see the frames and scenes "
			"in the animations and build the animation "
			"by moving the them around.</p><p>You can "
			"switch to the next and the previous frame "
			"using the <b>arrow buttons</b> or <b>x</b> "
			"and <b>z</b></p> ");
	frameBar->setWhatsThis(infoText );
}

MainWindowGUI::SaveDialogResult MainWindowGUI::saveIfNecessary() {
	bool b = DomainFacade::getFacade()->isUnsavedChanges();
	if (b) {
		QMessageBox::StandardButton save = QMessageBox::question(
				this, tr("Unsaved changes"),
				tr("There are unsaved changes. Do you want to save?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (save == QMessageBox::Save) {
			if (saveProject())
				return saveDialogSave;
			// User requested a save but cancelled the "Save As" dialog.
			// This counts as a cancel.
			return saveDialogCancel;
		}
		else if (save == QMessageBox::Cancel)
			return saveDialogCancel;
		// else save == QMessageBox::Discard
	}
	return saveDialogDiscard;
}

void MainWindowGUI::newProject() {
	if (saveDialogCancel != saveIfNecessary()) {
		DomainFacade::getFacade()->newProject();
		// It would be better if this was a signal emission, but it seems that
		// connect(this, ...) is problematic for some reason.
		frameView->workspaceCleared();
		saveAct->setEnabled(false);
		setTitle(false);
		DomainFacade::getFacade()->clearHistory();
		modelSizeChanged(0);
		toolsMenu->fixNavigationButtons(0);
	}
}

void MainWindowGUI::openProject() {
	if (saveDialogCancel != saveIfNecessary()) {
		QString file = QFileDialog::
			getOpenFileName(this,
					tr("Choose project file"),
					lastVisitedDir,
					"Stopmotion (*.sto)");
		if ( !file.isNull() ) {
			doOpenProject( file.toLocal8Bit().constData() );
		}
	}
}

void MainWindowGUI::updateNewActiveFrame(int, int frame) {
	if (frame < 0) {
		cutAct->setEnabled(false);
		copyAct->setEnabled(false);
	} else {
		cutAct->setEnabled(true);
		copyAct->setEnabled(true);
	}
}

void MainWindowGUI::updatePasteEnabled() {
	const QMimeData *mimeData = QApplication::clipboard()->mimeData();
	pasteAct->setEnabled(mimeData && mimeData->hasUrls());
}

void MainWindowGUI::doOpenProject(const char* projectFile) {
	assert(projectFile != nullptr);
	DomainFacade::getFacade()->openProject(projectFile);
	frameView->workspaceCleared();
	saveAsAct->setEnabled(true);
	saveAct->setEnabled(false);
	setTitle(false);
	updateMostRecentMenu();
	int size = DomainFacade::getFacade()->getModelSize();
	if (size > 0) {
		activateMenuOptions();
		modelSizeChanged(size);
		toolsMenu->fixNavigationButtons(size);
	}
}

void MainWindowGUI::openProject( const char * projectFile ) {
	if (saveDialogCancel != saveIfNecessary()) {
		doOpenProject(projectFile);
	}
}

void MainWindowGUI::openMostRecent() {
	Preference prop("mostRecent", "");
	openProject(prop.get());
}


void MainWindowGUI::openSecondMostRecent()
{
	Preference prop("secondMostRecent", "");
	openProject(prop.get());
}


void MainWindowGUI::openThirdMostRecent()
{
	Preference prop("thirdMostRecent", "");
	openProject(prop.get());
}


bool MainWindowGUI::saveProjectAs() {
	QString file = QFileDialog::getSaveFileName(this,
			tr("Save As"), lastVisitedDir, "Stopmotion (*.sto)");
	if ( file.isNull() )
		return false;
	DomainFacade::getFacade()->saveProject(file.toLocal8Bit());
	saveAct->setEnabled(false);
	setTitle(false);
	updateMostRecentMenu();
	return true;
}


bool MainWindowGUI::saveProject() {
	const char *file = DomainFacade::getFacade()->getProjectFile();
	if (file) {
		DomainFacade::getFacade()->saveProject(file);
		return true;
	}
	return saveProjectAs();
}


void MainWindowGUI::exportToVideo()
{
	PreferencesTool *prefs = PreferencesTool::get();
	int active = prefs->getPreference("activeEncoder", -1);
	if ( active == -1 ) {
		QMessageBox::StandardButton ret = QMessageBox::warning(this,
			tr("Warning"),
			tr("Cannot find any registered encoder to be used for\n"
			   "video export. This can be set in the preferences\n"
			   "menu. Export to video will not be possible until you\n"
			   "have set an encoder to use. Do you want to set it now?"),
			QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::Yes) {
			showPreferencesMenu();
		}
	}
	else {
		bool isCanceled = false;
		std::stringstream key;
		std::string k;
		VideoEncoder enc;

		key << "startEncoder" << active;
		k = key.str();
		Preference start(k.c_str(), "");
		enc.setStartCommand(start.get());

		key.str("");
		key << "stopEncoder" << active;
		k = key.str();
		Preference stop(k.c_str(), "");
		enc.setStopCommand(stop.get());

		key.str("");
		key << "outputFile" << active;
		k = key.str();
		Preference output(k.c_str());
		if (output.get() && *output.get()) {
			enc.setOutputFile(output.get());
		} else {
			QString file = QFileDialog::
				getSaveFileName(this,
						tr("Export to video file"),
						lastVisitedDir);
			if ( file.isEmpty() ) {
				isCanceled = true;
			} else {
				enc.setOutputFile( file.toLocal8Bit().constData() );
			}
		}

		if (enc.isValid() && !isCanceled) {
			DomainFacade::getFacade()->exportToVideo(&enc,
					frameView->getPlaybackSpeed());
		}
		else if (!isCanceled){
			QMessageBox::StandardButton ret = QMessageBox::warning(this,
					tr("Warning"),
					tr("The registered encoder is not valid. Do you want\n"
					"to check your settings in the preferences menu?"),
					QMessageBox::Yes | QMessageBox::No);
			if (ret == QMessageBox::Yes) {
				showPreferencesMenu();
			}
		}
	}
}

void MainWindowGUI::exportToCinerella()
{
	QString file = QFileDialog::
		getSaveFileName(this,
				tr("Export to file"), lastVisitedDir, "Cinerella (*.XXX)");

	if ( !file.isNull() ) {
		DomainFacade::getFacade()->exportToCinerella( file.toLocal8Bit().constData() );
	}
}


void MainWindowGUI::whatsThis()
{
	QWhatsThis::enterWhatsThisMode();
}


void MainWindowGUI::dragEnterEvent ( QDragEnterEvent * event)
{
	Logger::get().logDebug("Drag entering application");
	if ( event->mimeData()->hasUrls() ) {
		event->accept();
	}
	else {
		event->ignore();
	}
}


void MainWindowGUI::dropEvent(QDropEvent *event)
{
	Logger::get().logDebug("Drop inside the application");
	if ( event->mimeData()->hasUrls() ) {
		QStringList fileNames;
		QList<QUrl> urls = event->mimeData()->urls();
		int numFrames = urls.size();
		for (int i = 0; i < numFrames; ++i) {
			fileNames.append(urls[i].toLocalFile());
			modelHandler->addFrames(fileNames);
		}

	}
}


/**
 * @todo change so that CTRL+A instead of just A selects all the frames.
 */
void MainWindowGUI::keyPressEvent( QKeyEvent *k )
{
	switch ( k->key() )
	{
		case Key_Shift:
		{
			frameBar->setSelecting(true);
			break;
		}
		case Key_A:
		{
			DomainFacade* facade = DomainFacade::getFacade();
			int scene = frameBar->getActiveScene();
			if(0 <= scene) {
				int sceneSize = facade->getSceneSize(scene);
				if( sceneSize > 1) {
					frameBar->updateNewActiveFrame(scene, 0);
					frameBar->setSelection(sceneSize - 1);
				}
			}
			break;
		}
		default:
		{
			k->ignore();
			break;
		}
	}
}

void MainWindowGUI::updateCanUndo(bool newCanUndo) {
	undoAct->setEnabled(newCanUndo);
	saveAct->setEnabled(newCanUndo);
	setTitle(newCanUndo);
}

void MainWindowGUI::updateCanRedo(bool newCanRedo) {
	redoAct->setEnabled(newCanRedo);
}

void MainWindowGUI::keyReleaseEvent ( QKeyEvent * k )
{
	switch ( k->key() )
	{
		case Key_Shift:
		{
			frameBar->setSelecting(false);
			break;
		}
	}
}


void MainWindowGUI::setTitle(bool modified) {
	std::string title;
	if (modified)
		title = "* ";
	const char* name = DomainFacade::getFacade()->getProjectFile();
	if (name) {
		const char* lastDot = strrchr(name, '.');
		const char* lastSlash = strrchr(name, '/');
		title.append(lastSlash? lastSlash + 1 : name,
				lastDot? lastDot : name + strlen(name));
	} else {
		title.append("Stopmotion");
	}
	setWindowTitle(title.c_str());
}


void MainWindowGUI::showAboutDialog()
{
	AboutDialog *aboutDialog = new AboutDialog(this);
	aboutDialog->show();
}


void MainWindowGUI::showHelpDialog()
{
	HelpWindow *help = new HelpWindow(this);
	help->move(40, 40);
	help->show();
}



void MainWindowGUI::showPreferencesMenu()
{
	preferencesMenu->display();
}


void MainWindowGUI::mousePressEvent( QMouseEvent * )
{
	gotoMenuWidget->hide();
}


void MainWindowGUI::modelSizeChanged( int modelSize ) {
	if (modelSize == 0) {
		gotoFrameAct->setEnabled(false);
	} else {
		gotoFrameAct->setEnabled(true);
		saveAsAct->setEnabled(true);
	}
}


void MainWindowGUI::activateMenuOptions() {
	DomainFacade* facade = DomainFacade::getFacade();
	bool canUndo = facade->canUndo();
	undoAct->setEnabled(canUndo);
	redoAct->setEnabled(facade->canRedo());
	setTitle(canUndo);
}

void MainWindowGUI::createMostRecentMenu() {
	mostRecentMenu->clear();
	mostRecentMenu->setTitle(tr("Open &Recent"));
	Preference first("mostRecent", "");
	if (first.get() && access(first.get(), R_OK) == 0) {
		mostRecentAct->setVisible(true);
		mostRecentAct->setText(QString::fromLocal8Bit(first.get()));
	} else {
		mostRecentAct->setVisible(false);
	}
	Preference second("secondMostRecent", "");
	if (second.get() && access(second.get(), R_OK) == 0) {
		secondMostRecentAct->setVisible(true);
		secondMostRecentAct->setText(QString::fromLocal8Bit(second.get()));
	} else {
		secondMostRecentAct->setVisible(false);
	}
	Preference third("thirdMostRecent", "");
	if (third.get() && access(third.get(), R_OK) == 0) {
		thirdMostRecentAct->setVisible(true);
		thirdMostRecentAct->setText(QString::fromLocal8Bit(third.get()));
	} else {
		thirdMostRecentAct->setVisible(false);
	}
	mostRecentMenu->addAction(mostRecentAct);
	mostRecentMenu->addAction(secondMostRecentAct);
	mostRecentMenu->addAction(thirdMostRecentAct);
}

void MainWindowGUI::updateMostRecentMenu() {
	DomainFacade::getFacade()->setMostRecentProject();
	createMostRecentMenu();
}
