/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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

#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "src/technical/video/videoencoder.h"
#include "videoview.h"
#include "flexiblelineedit.h"
#include "licence.h"
#include "preferencesmenu.h"
#include "toolsmenu.h"

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

#include <stdlib.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qwizard.h>
#include <qevent.h>
#include <qdragobject.h> 
#include <qmessagebox.h>
#include <qapplication.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qiconset.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qtabdialog.h>
#include <qtextedit.h>
#include <qaccel.h>

#include <iostream>
using namespace std;


MainWindowGUI::MainWindowGUI(QApplication *stApp) 
		: QMainWindow( 0, "stopmotion", WDestructiveClose )
{
	centerWidget        = NULL;
	workArea            = NULL;
	frameViewStack      = NULL;
	frameView           = NULL;
	frameBar            = NULL;
	newAct              = NULL;
	openAct             = NULL;
	mostRecentAct       = NULL;
	secondMostRecentAct = NULL;
	thirdMostRecentAct  = NULL;
	saveAct             = NULL;
	saveAsAct           = NULL;
	videoAct            = NULL;
	cinerellaAct        = NULL;
	quitAct             = NULL;
	undoAct             = NULL;
	redoAct             = NULL;
	cutAct              = NULL;
	copyAct             = NULL;
	pasteAct            = NULL;
	gotoFrameAct        = NULL;
// 	disableAct          = NULL;
	configureAct             = NULL;
 	whatsthisAct        = NULL;
	aboutAct            = NULL;
	
	fileMenu            = NULL;
	exportMenu          = NULL;
	mostRecentMenu      = NULL;
	editMenu            = NULL;
	toolsMenu           = NULL;
	preferencesMenu     = NULL;
	gotoMenu            = NULL;
	helpMenu            = NULL;
	lastVisitedDir      = NULL;
	numberDisplay       = NULL;
	gotoSpinner         = NULL;
	cameraHandler       = NULL;
	editMenuHandler     = NULL;
	runAnimationHandler = NULL;
	modelHandler        = NULL;
	languageHandler     = NULL;
	soundHandler        = NULL;
	changeMonitor       = NULL; 
	
	lastVisitedDir = new char[256];
	strcpy( lastVisitedDir, getenv("PWD") );
	centerWidget = new QSplitter(Qt::Vertical, this);
	centerWidget->setChildrenCollapsible(false);
	//centerWidget->setResizeMode( QWidget * w, ResizeMode mode )
	
	makeFrameBar(centerWidget);
	//centerWidget->setResizeMode(frameBar, QSplitter::KeepSize);
	
	setupDirectoryMonitoring();
	createHandlers(stApp);
	createAccelerators();
	
	bottomSplitter = new QVBox(centerWidget);
	
	makePreferencesMenu(bottomSplitter);
	
	//Initializes and sets up the workarea consisting of the toolsmenu and the frameview.
	workArea = new QHBox(bottomSplitter);
// 	centerWidget->setResizeMode(workArea, QSplitter::KeepSize);
	
	makeToolsMenu(workArea);
	makeViews(workArea);
	makeGotoMenu(centerWidget);
	
	setCentralWidget(centerWidget);
	
	makeStatusBar();
	
	//Initializes and sets up the menue system.
	createActions();
	createMenus();
	
	//This slot will activate/deactivate menu options based on the changes in the model.
	QObject::connect( frameBar, SIGNAL( modelSizeChanged(int) ),
			this, SLOT( modelSizeChanged(int) ) );
	
	//Mainwindow preferences.
	setIcon( QPixmap(windowicon) );
	setAcceptDrops(true);
	statusBar()->message( tr("Ready to rumble ;-)"), 2000 );
	statusBar()->setSizeGripEnabled(false);
	
	//Sets all the text in the program.
	retranslateStrings();
}


MainWindowGUI::~MainWindowGUI()
{
	delete [] lastVisitedDir;
}


void MainWindowGUI::createHandlers(QApplication *stApp)
{
	languageHandler = new LanguageHandler( this, this->statusBar(), stApp );
	QObject::connect( languageHandler, SIGNAL(languageChanged()), 
			this, SLOT(retranslateStrings()) );
	
	cameraHandler = new CameraHandler( this, this->statusBar() );
	
	runAnimationHandler = new RunAnimationHandler( this, this->statusBar() );
	
	modelHandler = new ModelHandler( this, this->statusBar(), frameBar, changeMonitor, lastVisitedDir );
	QObject::connect( modelHandler, SIGNAL(modelChanged()), 
			this, SLOT(activateMenuOptions()) );
	
	editMenuHandler = new EditMenuHandler( this, this->statusBar(), frameBar );
	QObject::connect( editMenuHandler, SIGNAL(addFrames(const QStringList &)),
			modelHandler, SLOT(addFrames(const QStringList &)) );
			
	soundHandler = new SoundHandler( this, this->statusBar(), 
			this->lastVisitedDir );
	QObject::connect( soundHandler, SIGNAL( soundsChanged() ),
			frameBar, SLOT( frameSoundsChanged() ) );
}


void MainWindowGUI::setupDirectoryMonitoring()
{
	changeMonitor = new ExternalChangeMonitor();
	changeMonitor->startMonitoring();
}


void MainWindowGUI::createAccelerators()
{
	QAccel *nextFrameAccel = new QAccel( this );
	nextFrameAccel->connectItem( nextFrameAccel->insertItem(Key_L), 
			runAnimationHandler, SLOT(selectNextFrame()) );
	nextFrameAccel->connectItem( nextFrameAccel->insertItem(Key_Right), 
			runAnimationHandler, SLOT(selectNextFrame()) );
	
	QAccel *previousFrameAccel = new QAccel( this );
	previousFrameAccel->connectItem( previousFrameAccel->insertItem(Key_J), 
			runAnimationHandler, SLOT(selectPreviousFrame()) );
	previousFrameAccel->connectItem( previousFrameAccel->insertItem(Key_Left), 
			runAnimationHandler, SLOT(selectPreviousFrame()) );
	
	QAccel *toggleCameraAccel = new QAccel( this );
	toggleCameraAccel->connectItem( toggleCameraAccel->insertItem(Key_C), 
			cameraHandler, SLOT(toggleCamera()) );
	
	QAccel *captureAccel = new QAccel( this );
	captureAccel->connectItem( captureAccel->insertItem(Key_Space), 
			cameraHandler, SLOT(captureFrame()) );
	
		QAccel *addFrameAccel = new QAccel( this );
	addFrameAccel->connectItem( addFrameAccel->insertItem(CTRL+Key_F),
			modelHandler, SLOT(chooseFrame()) );
	
	QAccel *newSceneAccel = new QAccel( this );
	newSceneAccel->connectItem( newSceneAccel->insertItem(CTRL+Key_E),
			modelHandler, SLOT(newScene()) );
	
	QAccel *removeFramesAccel = new QAccel( this );
	removeFramesAccel->connectItem( removeFramesAccel->insertItem(Key_Delete), 
			modelHandler, SLOT(removeFrames()) );
	
	QAccel *removeSceneAccel = new QAccel( this );
	removeSceneAccel->connectItem( removeSceneAccel->insertItem(SHIFT+Key_Delete), 
			modelHandler, SLOT(removeScene()) );
}


void MainWindowGUI::createActions()
{
	//File menu
	newAct = new QAction(this);
	newAct->setIconSet(QIconSet(filenewicon));
	newAct->setAccel(CTRL+Key_N);
	QObject::connect(newAct, SIGNAL(activated()), this, SLOT(newProject()));
	
	openAct = new QAction(this);
	openAct->setIconSet(QIconSet(fileopenicon));
	openAct->setAccel(CTRL+Key_O);
	QObject::connect(openAct, SIGNAL(activated()), this, SLOT(openProject()));
	
	mostRecentAct = new QAction(this);
	mostRecentAct->setIconSet(QIconSet(windowicon));
	connect(mostRecentAct, SIGNAL(activated()), this, 
			SLOT(openMostRecent()));
	
	secondMostRecentAct = new QAction(this);
	secondMostRecentAct->setIconSet(QIconSet(windowicon));
	connect(secondMostRecentAct, SIGNAL(activated()), this, 
			SLOT(openSecondMostRecent()));
	
	thirdMostRecentAct = new QAction(this);
	thirdMostRecentAct->setIconSet(QIconSet(windowicon));
	connect(thirdMostRecentAct, SIGNAL(activated()), this, 
			SLOT(openThirdMostRecent()));
	
	saveAct = new QAction(this);
	saveAct->setIconSet(QIconSet(filesaveasicon));
	saveAct->setAccel(CTRL+Key_S);
	QObject::connect(saveAct, SIGNAL(activated()), this, SLOT(saveProject()));
	
	saveAsAct = new QAction(this);
	saveAsAct->setIconSet(QIconSet(filesaveicon));
	saveAsAct->setAccel(CTRL+SHIFT+Key_S);
	QObject::connect(saveAsAct, SIGNAL(activated()), this, SLOT(saveProjectAs()));
	
	videoAct = new QAction(this);
	videoAct->setAccel(CTRL+ALT+Key_V);
	videoAct->setIconSet(QIconSet(videoexport));
	QObject::connect(videoAct, SIGNAL(activated()), this, SLOT(exportToVideo()));
	
	cinerellaAct = new QAction(this);
	cinerellaAct->setAccel(CTRL+ALT+Key_C);
	cinerellaAct->setEnabled(false);
	QObject::connect(cinerellaAct, SIGNAL(activated()), this, SLOT(exportToCinerella()));
	
	quitAct = new QAction(this);
	quitAct->setIconSet(QIconSet(quiticon));
	quitAct->setAccel(CTRL+Key_Q);
	QObject::connect(quitAct, SIGNAL(activated()), qApp, SLOT(quit()));
	
	
	//Edit menu
	undoAct = new QAction(this);
	undoAct->setIconSet(QIconSet(undoicon));
	undoAct->setAccel(CTRL+Key_Z);
	QObject::connect(undoAct, SIGNAL(activated()), editMenuHandler, SLOT(undo()));
	
	redoAct = new QAction(this);
	redoAct->setIconSet(QIconSet(redoicon));
	redoAct->setAccel(CTRL+SHIFT+Key_Z);
	QObject::connect(redoAct, SIGNAL(activated()), editMenuHandler, SLOT(redo()));
	
	cutAct = new QAction(this);
	cutAct->setIconSet(QIconSet(cuticon));
	cutAct->setAccel(CTRL+Key_X);
	QObject::connect(cutAct, SIGNAL(activated()), editMenuHandler, SLOT(cut()));
	
	copyAct = new QAction(this);
	copyAct->setIconSet(QIconSet(copyicon));
	copyAct->setAccel(CTRL+Key_C);
	QObject::connect(copyAct, SIGNAL(activated()), editMenuHandler, SLOT(copy()));
	
	pasteAct = new QAction(this);
	pasteAct->setIconSet(QIconSet(pasteicon));
	pasteAct->setAccel(CTRL+Key_V);
	QObject::connect(pasteAct, SIGNAL(activated()), editMenuHandler, SLOT(paste()));
	
	gotoFrameAct = new QAction(this);
	gotoFrameAct->setAccel(CTRL+Key_G);
	QObject::connect(gotoFrameAct, SIGNAL(activated()), gotoMenu, SLOT(open()));
	
	
	//Settings menu	
// 	disableAct = new QAction(this);
	//disableAct->setAccel(CTRL+Key_G);
	//QObject::connect(disableAct, SIGNAL(activated()), gotoMenu, SLOT(open()));
// 	disableAct->setEnabled(false);
	
	configureAct = new QAction(this);
	configureAct->setIconSet(QIconSet(configureicon));
	configureAct->setAccel(CTRL+Key_P);
	QObject::connect(configureAct, SIGNAL(activated()), this, SLOT(showPreferencesMenu()));
	

	//Help menu
	whatsthisAct = new QAction(this);
	whatsthisAct->setIconSet(QIconSet(whatsthisicon));
	whatsthisAct->setAccel(SHIFT+Key_F1);
	QObject::connect(whatsthisAct, SIGNAL(activated()), this, SLOT(whatsThis()));
	
	aboutAct = new QAction(this);
	QObject::connect(aboutAct, SIGNAL(activated()), this, SLOT(showAboutDialog()));
}


void MainWindowGUI::createMenus()
{
	fileMenu = new QPopupMenu(this);
	exportMenu = new QPopupMenu(fileMenu);
	mostRecentMenu = new QPopupMenu(fileMenu);
	
	editMenu = new QPopupMenu(this);
	undoAct->addTo(editMenu);
	undoAct->setEnabled(false);
	redoAct->addTo(editMenu);
	redoAct->setEnabled(false);
	editMenu->insertSeparator();
	cutAct->addTo(editMenu);
	cutAct->setEnabled(false);
	copyAct->addTo(editMenu);
	copyAct->setEnabled(false);
	pasteAct->addTo(editMenu);
	editMenu->insertSeparator();
	gotoFrameAct->addTo(editMenu);
	gotoFrameAct->setEnabled(false);
	
	settingsMenu = new QPopupMenu(this);
	languagesMenu = languageHandler->createLanguagesMenu(settingsMenu);
	
	helpMenu = new QPopupMenu(this);
	whatsthisAct->addTo(helpMenu);
	aboutAct->addTo(helpMenu);

}


void MainWindowGUI::makeFrameBar(QWidget * parent)
{
	frameBar = new FrameBar(parent);
}

  
void MainWindowGUI::makeToolsMenu(QWidget * parent)
{
	toolsMenu = new ToolsMenu(runAnimationHandler, modelHandler, 
			cameraHandler, parent);
	
	QObject::connect( frameBar, SIGNAL( modelSizeChanged(int) ),
			toolsMenu, SLOT( modelSizeChanged(int) ) );
	QObject::connect( cameraHandler, SIGNAL(cameraStateChanged(bool)),
			toolsMenu, SLOT(cameraOn(bool)) );
}


void MainWindowGUI::makePreferencesMenu( QWidget * parent )
{
	preferencesMenu = new FramePreferencesMenu(parent, soundHandler);
	frameBar->setPreferencesMenu(preferencesMenu);
	preferencesMenu->hide();
}


void MainWindowGUI::makeGotoMenu( QWidget * parent )
{
	gotoMenu = new MenuFrame( parent );
	editMenuHandler->setGotoMenu(gotoMenu);
	QGridLayout *grid = new QGridLayout( gotoMenu, 0, 2, 2 );
	
	gotoFrameLabel = new QLabel(gotoMenu);
	gotoFrameLabel->setMaximumWidth(gotoFrameLabel->width());
	
	gotoSpinner = new FlexibleSpinBox(gotoMenu);
	gotoSpinner->setMaximumWidth(60);
	gotoSpinner->setMinValue(1);
	gotoSpinner->setEnabled(false);
	QObject::connect( frameBar, SIGNAL( newActiveFrame(int) ), 
			gotoSpinner, SLOT( setValue(int) ) );
	QObject::connect( frameBar, SIGNAL( modelSizeChanged(int) ),
			gotoSpinner, SLOT( setMaximumValue(int) ) );
	gotoMenu->hide();
	QObject::connect( gotoSpinner, SIGNAL( spinBoxTriggered(int) ), 
	        editMenuHandler, SLOT( gotoFrame(int) ) );
	QObject::connect( gotoSpinner, SIGNAL( spinBoxCanceled() ),
			editMenuHandler, SLOT( closeGotoMenu() ) );
	gotoMenu->setFocusWidget(gotoSpinner);
	
	QSpacerItem *space = new QSpacerItem(0, 0);
	
	QPushButton *closeButton = new QPushButton(gotoMenu);
	closeButton->setPixmap(closeicon);
	closeButton->setFlat(true);
	closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	QObject::connect( closeButton, SIGNAL(clicked()), 
			gotoMenu, SLOT(close()) );
	
	grid->addWidget( gotoFrameLabel, 0, 0 );
	grid->addWidget( gotoSpinner, 0, 1 );
	grid->addItem( space, 0, 2 );
	grid->addWidget( closeButton, 0, 3 );
}


void MainWindowGUI::makeStatusBar()
{
	numberDisplay = new QLabel(this);
	this->statusBar()->addWidget(numberDisplay, 0, true);
	QObject::connect( frameBar, SIGNAL(newActiveFrame(const QString &)), 
			numberDisplay, SLOT(setText(const QString &)) );
}


void MainWindowGUI::makeViews(QWidget *parent)
{
	QObject::connect(cameraHandler, SIGNAL(capturedFrame()), 
			this, SLOT(activateMenuOptions()));
	
	frameViewStack = new QWidgetStack(parent);
	
	frameView = new FrameView(frameViewStack);
	frameView->setBackgroundMode(NoBackground);
	
	frameViewStack->raiseWidget(frameView);
	DomainFacade::getFacade()->attatch(frameView);
	
	frameViewStack->setMinimumSize(400, 300);
	frameViewStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	cameraHandler->setWidgetStack(frameViewStack);
	cameraHandler->setFrameView(frameView);
	
	VideoView *videoView = frameView;
	
	QObject::connect( videoView, SIGNAL(cameraReady()),
	 		cameraHandler, SLOT(switchToVideoView()) );
	
	cameraHandler->setVideoView(videoView);
}


void MainWindowGUI::retranslateStrings()
{
	Logger::get().logDebug("Retranslating strings");
	
	//The actions caption texts
	newAct->setMenuText(		tr("&New"));
	openAct->setMenuText(		tr("&Open"));
	saveAct->setMenuText(		tr("&Save"));
	saveAsAct->setMenuText(		tr("Save &As"));
	videoAct->setMenuText(		tr("Video"));
	cinerellaAct->setMenuText(	tr("Cinelerra"));
	quitAct->setMenuText(		tr("&Quit"));
	
	undoAct->setMenuText(		tr("&Undo"));
	redoAct->setMenuText(		tr("Re&do"));
	cutAct->setMenuText(		tr("Cu&t"));
	copyAct->setMenuText(		tr("&Copy"));
	pasteAct->setMenuText(		tr("&Paste"));
	gotoFrameAct->setMenuText(	tr("&Go to frame"));
	
// 	disableAct->setMenuText(	tr("Disable sound"));
	configureAct->setMenuText(		tr("&Configure Stopmotion"));
	
	whatsthisAct->setMenuText(	tr("What's &This"));
	aboutAct->setMenuText(		tr("&About"));
	
	//Other widgets
	numberDisplay->setText(		tr("Frame number: 0"));
	gotoFrameLabel->setText(	tr("Go to frame:"));
	
	//Tooltip and whatsthis texts
	retranslateHelpText();
	
	
	//The menus
	menuBar()->clear(); 
	//Have to build the file and setting menues here because
	//of the submenues (export and languages).
	menuBar()->insertItem( tr("&File"), fileMenu);
	fileMenu->clear();
	newAct->addTo(fileMenu);
	openAct->addTo(fileMenu);
	fileMenu->insertItem(QIconSet(fileopenicon), tr("Open &Recent"), mostRecentMenu);
	updateMostRecentMenu();
	fileMenu->insertSeparator();
	saveAct->addTo(fileMenu);
	saveAsAct->addTo(fileMenu);
	fileMenu->insertItem(tr("&Export"), exportMenu);
	exportMenu->clear();
	videoAct->addTo(exportMenu);
	cinerellaAct->addTo(exportMenu);
	fileMenu->insertSeparator();
	quitAct->addTo(fileMenu);
	
	menuBar()->insertItem( tr("&Edit"), editMenu);
	
	menuBar()->insertItem( tr("&Settings"), settingsMenu);
	settingsMenu->clear();
// 	disableAct->addTo(settingsMenu);
 	settingsMenu->insertItem(QIconSet(languages), tr("&Languages"), languagesMenu);
	configureAct->addTo(settingsMenu);
	
	menuBar()->insertItem( tr("&Help"), helpMenu);
	
	//The submenus
	toolsMenu->retranslateStrings();
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
			"automaticly be saved to the previously selected file.</p>");
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
	
	
	//Settings menu
	/*infoText =
			tr("<h4>Disable sound</h4> "
			"<p>This will disable the sound from playing when you run the "
			"animation</p>");
	disableAct->setWhatsThis(infoText);
	infoText = 
			disableAct->toolTip().prepend(tr("Quit"));
	disableAct->setToolTip(infoText);*/
	
	infoText =
			tr("<h4>Configure Stopmotion</h4> "
			"<p>This will opens a window where you can <em>configure</em> "
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
			tr("<h4>Frame number</h4><p>This area displays the number" 
			"of the currently selected frame</p>");
	QToolTip::add( numberDisplay, infoText );
	QWhatsThis::add( numberDisplay, infoText );
	
	//Various menues
	infoText = 
			tr("<h4>FrameView</h4><p> In this area you can see the "
			"selected frame. You can also play "
			"animations in this window by pressing the "
			"<b>Play</b> button.</p>");
	QWhatsThis::add( frameView, infoText );
	
	infoText = 
			tr("<h4>Go to frame menu</h4> "
			"<p>Here you can specify a framenumber and the program will jump "
			"to the specified frame</p> ");
    QWhatsThis::add( gotoMenu, infoText );
	
	infoText = 
			tr("<h4>Frame preferences menu</h4> "
			"<p>In this menu you can set preferences for the "
			"selected frame/frames, such as <b>subtitles</b>, "
			"<b>sound effects</b>, etc.</p>");
	QWhatsThis::add( preferencesMenu, infoText );
	
	infoText = 
			tr("<h4>Tool menu</h4> "
			"<p>This is the tool menu where most of the buttons and widgets "
			"you will need when working on stop motion animations are located.</p>");
	QWhatsThis::add( toolsMenu, infoText );
	
	infoText = 
			tr("<h4>FrameBar</h4> "
			"<p>In this area you can see the frames and scenes "
			"in the animations and build the animation "
			"by moving the them around.</p><p>You can "
			"switch to the next and the previous frame "
			"using the <b>arrow buttons</b> or <b>x</b> "
			"and <b>z</b></p> ");
	QWhatsThis::add( frameBar, infoText );
}


void MainWindowGUI::newProject() 
{
	bool b = DomainFacade::getFacade()->isUnsavedChanges();
	if (b) {
		int save = QMessageBox::question(this,
			tr("Unsaved changes"),
			tr("There are unsaved changes. Do you want to save?"),
			tr("&Yes"), tr("&No"), // button 0, button 1, ...
			QString::null, 0, 1 );
		if (save == 0) { // user pressed button 0, which is 'yes'
			saveProject();
		}
	}
	DomainFacade::getFacade()->newProject();
	fileMenu->setItemEnabled(SAVE, false);
	
	DomainFacade::getFacade()->clearHistory();
	changeMonitor->changeWorkDirectory("");
	
	modelSizeChanged(0);
	toolsMenu->modelSizeChanged(0);
}


void MainWindowGUI::openProject()
{
	QString file = QFileDialog::getOpenFileName(lastVisitedDir, "Stopmotion (*.sto)", this);
	if ( !file.isNull() ) {
		DomainFacade::getFacade()->openProject( file.ascii() );
		fileMenu->setItemEnabled(SAVE_AS, true);
		fileMenu->setItemEnabled(SAVE, true);
		setMostRecentProject();
		int size = DomainFacade::getFacade()->getModelSize();
		if(size > 0) {
			activateMenuOptions();
			modelSizeChanged(size);
			toolsMenu->modelSizeChanged(size);
		}
	}
}


void MainWindowGUI::openMostRecent()
{
	PreferencesTool *pref = PreferencesTool::get();
	DomainFacade::getFacade()->openProject( pref->getPreference("mostRecent", "") );
	fileMenu->setItemEnabled(SAVE_AS, true);
	fileMenu->setItemEnabled(SAVE, true);
	setMostRecentProject();
}


void MainWindowGUI::openSecondMostRecent()
{
	PreferencesTool *pref = PreferencesTool::get();
	DomainFacade::getFacade()->openProject( pref->getPreference("secondMostRecent", "") );
	fileMenu->setItemEnabled(SAVE_AS, true);
	fileMenu->setItemEnabled(SAVE, true);
	setMostRecentProject();
}


void MainWindowGUI::openThirdMostRecent()
{
	PreferencesTool *pref = PreferencesTool::get();
	DomainFacade::getFacade()->openProject( pref->getPreference("thirdMostRecent", "") );
	fileMenu->setItemEnabled(SAVE_AS, true);
	fileMenu->setItemEnabled(SAVE, true);
	setMostRecentProject();
}


void MainWindowGUI::saveProjectAs()
{
	QString file = QFileDialog::getSaveFileName(lastVisitedDir, "Stopmotion (*.sto)",
			this, "Save file dialog", tr("Save File"));

	if ( !file.isNull() ) {
		DomainFacade::getFacade()->saveProject( file.ascii() );
		fileMenu->setItemEnabled(SAVE, true);
		setMostRecentProject();
	}
}


void MainWindowGUI::saveProject()
{
	const char *file = DomainFacade::getFacade()->getProjectFile();
	if (file) {
		DomainFacade::getFacade()->saveProject(file);
		changeMonitor->changeWorkDirectory(file);
	}
	else {
		saveProjectAs();
	}
}


void MainWindowGUI::exportToVideo()
{
	PreferencesTool *prefs = PreferencesTool::get();
	int active = prefs->getPreference("activeEncoder", -1);
	if ( active == -1 ) {
		int ret = QMessageBox::warning(this,
			tr("Warning"),
			tr("Cannot find any registered encoder to be used for\n"
			   "video export. This can be setted in the preferences\n"
			   "menu. Export to video will not be possible until you\n"
			   "have setted an encoder to use. Do you want to set it now?"),
			tr("&Yes"), tr("&No"), // button 0, button 1, ...
			QString::null, 0, 1 );
		if (ret == 0) {
			showPreferencesMenu();
		}
	}
	else {
		bool isCanceled = false;
		char tmp[256];
		VideoEncoder *enc = new VideoEncoder();
		sprintf(tmp, "startEncoder%d", active);
		enc->setStartCommand( prefs->getPreference(tmp, "") );
		sprintf(tmp, "stopEncoder%d", active);
		enc->setStopCommand( prefs->getPreference(tmp, "") );
		sprintf(tmp, "outputFile%d", active);
		const char *output = prefs->getPreference(tmp, "");
		if (strcmp(output, "") == 0) {
			QString file = QFileDialog::getSaveFileName(lastVisitedDir, NULL,
			this, "Export to video", tr("Export to video file"));
			if ( file.isEmpty() ) {
				isCanceled = true;	
			}
			else {
				enc->setOutputFile( file.ascii() );
			}
		}
		else {
			enc->setOutputFile(output);
		}
		if ( enc->isValid() && isCanceled == false ) {
			DomainFacade::getFacade()->exportToVideo(enc);
		}
		else if (isCanceled == false){
			int ret = QMessageBox::warning(this,
					tr("Warning"),
					tr("The registered encoder is not valid. Do you want\n"
					"to check your settings in the preferences menu?"),
					tr("&Yes"), tr("&No"), // button 0, button 1, ...
					QString::null, 0, 1 );
			if (ret == 0) {
				showPreferencesMenu();
			}
		}
	}
}


void MainWindowGUI::exportToCinerella()
{
	QString file = QFileDialog::getSaveFileName(lastVisitedDir, "Cinerella (*.XXX)",
			this, "Export to Cinerella", tr("Export to file"));
	
	if ( !file.isNull() ) {
		DomainFacade::getFacade()->exportToCinerella( file.ascii() );
	}
}


void MainWindowGUI::dragEnterEvent ( QDragEnterEvent * event)
{
	Logger::get().logDebug("Drag entering application");
	event->accept( QUriDrag::canDecode(event) );
}


void MainWindowGUI::dropEvent(QDropEvent *event)
{
	Logger::get().logDebug("Drop inside the application");

	if( QUriDrag::canDecode(event) ) {
		QStringList fileNames;
		
		if( QUriDrag::decodeLocalFiles(event, fileNames) ) {
			Logger::get().logDebug("test");
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
			DomainFacade *facade = DomainFacade::getFacade();
			int activeScene = facade->getActiveSceneNumber();
			if( facade->getActiveSceneNumber() >= 0) {
				if( facade->getSceneSize(activeScene) > 1) {
					facade->setActiveFrame(0);
					frameBar->setSelection( facade->getSceneSize(activeScene)-1 );
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


/**
 * @todo Refactor to class
 */
void MainWindowGUI::showAboutDialog()
{
	QTabDialog *aboutDialog = new QTabDialog(this, "aboutdialog", false,
			WDestructiveClose);
	
	QLabel *aboutText = 
			new QLabel(tr(
						"<p>This is the stopmotion application for "
						"creating stopmotion animations.</p>"
						"<p>(c) 2005, Fredrik Berg Kj�lstad and Bj�rn Erik Nilsen</p>"), 
						aboutDialog);
	aboutDialog->addTab(aboutText, tr("&About"));
	
	QLabel *authorsText = 
			new QLabel(tr(
						"<p><b>Main developers</b><br>"
						"Fredrik Berg Kj�lstad &lt;fredrikbk@hotmail.com&gt;<br>"
						"Bj�rn Erik Nilsen &lt;bjoern_erik_nilsen@hotmail.com&gt;</p>"), 
						aboutDialog);
	aboutDialog->addTab(authorsText, tr("A&uthors"));
	
	QTextEdit *thanksToText =
			new QTextEdit( aboutDialog );
	thanksToText->setText( tr(
						"<p><b>Coordinating</b><br>"
						"Herman Robak &lt;herman@skolelinux.no&gt;<br>"
						"�yvind Kol�s &lt;pippin@gimp.org&gt;</p>"
						"<p><b>Testing</b><br>"
						"Tore Sinding Bekkedal &lt;toresbe@ifi.uio.no&gt;<br>"
						"Finn Arne Johansen &lt;faj@bzz.no&gt;<br>"
						"Halvor Borgen &lt;halvor.borgen@hig.no&gt;<br>"
						"Bj�rn Are Hansen &lt;post@bahansen.net&gt;<br>"
						"John Steinar Bild�y &lt;johnsbil@haldenfriskole.no&gt;<br>"
						"Ole-Anders Andreassen &lt;ole-anders.andreassen@sunndal.kommune.no&gt;<br>"
						"</p>"
						"<p><b>Translation</b><br></p>") );
	thanksToText->setReadOnly(true);
	thanksToText->setPaletteBackgroundColor(aboutDialog->paletteBackgroundColor());
	aboutDialog->addTab(thanksToText, tr("&Thanks To"));
	
	QTextEdit *licenceText = new QTextEdit( aboutDialog );
	licenceText->setReadOnly(true);
	licenceText->setHScrollBarMode( QScrollView::AlwaysOff );
	licenceText->setMinimumWidth(620);
	licenceText->setMinimumHeight(250);
	
	licenceText->setText( licence );
	
	aboutDialog->addTab(licenceText, tr("&Licence Agreement"));
	
	aboutDialog->show();
}


void MainWindowGUI::showPreferencesMenu()
{
	PreferencesMenu *preferencesMenu = new PreferencesMenu(this);
	preferencesMenu->show();
}


void MainWindowGUI::mousePressEvent( QMouseEvent * )
{
	gotoMenu->close(this);
}


void MainWindowGUI::modelSizeChanged( int modelSize )
{
	if(modelSize == 0) {
// 		editMenu->setItemEnabled(CUT, false);
		cutAct->setEnabled(false);
		copyAct->setEnabled(false);
		gotoFrameAct->setEnabled(false);
	}
}


void MainWindowGUI::activateMenuOptions()
{
	undoAct->setEnabled(true);
	redoAct->setEnabled(true);
	gotoFrameAct->setEnabled(true);
	saveAsAct->setEnabled(true);
	copyAct->setEnabled(true);
	//cutAct->setEnabled(true);
}


void MainWindowGUI::closeEvent(QCloseEvent *)
{
	qApp->quit();
}



void MainWindowGUI::setMostRecentProject()
{
	const char *first = DomainFacade::getFacade()->getProjectFile();
	if (first != NULL) {
		PreferencesTool *prefs = PreferencesTool::get();
		const char *third = prefs->getPreference("secondMostRecent", "");
		const char *second = prefs->getPreference("mostRecent", "");
		prefs->setPreference("mostRecent", first, false);
		prefs->setPreference("secondMostRecent", second, false);
		prefs->setPreference("thirdMostRecent", third, false);
		updateMostRecentMenu();
	}
}


void MainWindowGUI::updateMostRecentMenu( )
{
	mostRecentMenu->clear();
	PreferencesTool *pref = PreferencesTool::get();
	
	const char *first = pref->getPreference("mostRecent", "");
	if ( strcmp(first, "") != 0 && access(first, F_OK) == 0 ) {
		mostRecentAct->setMenuText(first);
		mostRecentAct->addTo(mostRecentMenu);
	}
	const char *second = pref->getPreference("secondMostRecent", "");
	if (strcmp(second, "") != 0 && access(second, F_OK) == 0 ) {
		secondMostRecentAct->setMenuText(second);
		secondMostRecentAct->addTo(mostRecentMenu);
	}
	const char *third = pref->getPreference("thirdMostRecent", "");
	if (strcmp(third, "") != 0 && access(third, F_OK) == 0 ) {
		thirdMostRecentAct->setMenuText(third);
		thirdMostRecentAct->addTo(mostRecentMenu);
	}
}
