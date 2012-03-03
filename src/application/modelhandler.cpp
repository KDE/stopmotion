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
#include "modelhandler.h"

#include "mainwindowgui.h"
#include "src/domain/domainfacade.h"
#include "filedialog.h"
#include "picturepreview.h"


ModelHandler::ModelHandler ( QObject *parent, QStatusBar *sb, FrameBar *frameBar, 
		ExternalChangeMonitor *changeMonitor, char *lastVisitedDir, const char *name ) 
		: QObject(parent, name), frameBar(frameBar), statusBar(sb), 
		lastVisitedDir(lastVisitedDir), changeMonitor(changeMonitor)
{
	fileDialog = NULL;
	removeFramesButton = NULL;
}


ModelHandler::~ModelHandler()
{
}


void ModelHandler::setRemoveFramesButton( QPushButton * removeFramesButton )
{
	this->removeFramesButton = removeFramesButton;
}


void ModelHandler::chooseFrame()
{
	fileDialog = new FileDialog(lastVisitedDir, NULL, (MainWindowGUI*)parent());
	PicturePreview* p = new PicturePreview(fileDialog);
	
	fileDialog->setContentsPreviewEnabled( TRUE );
	fileDialog->setContentsPreview( p, p );
	fileDialog->setMode(QFileDialog::ExistingFiles);
	fileDialog->setPreviewMode( QFileDialog::Contents );
	fileDialog->setCaption( tr("Choose frames to add") );
	fileDialog->setFilters( QString(
					"Images (*.png *.jpg *.jpeg  *.gif *.PNG *.JPG *.JPEG *.GIF)\n"
					"Joint Photographic Ex. Gr. (*.jpg *.jpeg *.JPG *.JPEG)\n"
					"Portable Network Graphics (*.png *.PNG)\n"
					"GIMP native (*.xcf *.XCF)\n"
					"Tagged Image File Format (*.tif *.TIF)\n"
					"Windows Bitmap (*.bmp *.BMP)\n"
					"TrueVision Targa (*.tga *.TGA)\n"
					"Portable Anymap (*.pnm *.PNM)\n"
					"X11 Pixmap (*.xpm *.XPM)\n"
					"ZSoft IBM PC Paintbrush (*.pcx *.PCX)\n"
					"CompuServe Graph. Interch. Format (*.gif *.GIF)\n"
					"Interleaved Bitmap (*.lbm *.iff *.LBM *.IFF)\n"
					"All files (*)\n") );
	
	QObject::connect( fileDialog, SIGNAL(filesSelected (const QStringList &)), 
			this, SLOT(addFrames(const QStringList &)) );
	QObject::connect( fileDialog, SIGNAL(fileSelected (const QString &)), 
			this, SLOT(addFrame(const QString &)) );
	
	fileDialog->show();
}


void ModelHandler::addFrames( const QStringList & fileNames)
{
	Logger::get().logDebug("addFrames in modelhandler");
	QStringList names = fileNames;
	
	changeMonitor->suspendMonitor();
	// the fileDialog pointer is NULL when adding of frames is
	// done by drag 'n drop
	if ( fileDialog != NULL ) {
		fileDialog->hide();
		strcpy( lastVisitedDir, ( fileDialog->dirPath() ).ascii() );
	}
	
	if ( !names.isEmpty() ) {
		vector<char*> fNames;
		QStringList::Iterator it = names.begin();
		while(it != names.end() ) {
			QString fileName = *it;
			char *f = (char*)fileName.ascii();
			fNames.push_back( f );
			++it;
		}
		// trim to size :)
		vector<char*>(fNames).swap(fNames);
		DomainFacade::getFacade()->addFrames(fNames);
		//this->activateMenuOptions();
		emit modelChanged();
	}
	
	changeMonitor->resumeMonitor();
}


void ModelHandler::addFrame( const QString &fileName )
{
	if ( fileDialog != NULL ) {
		fileDialog->hide();
		strcpy( lastVisitedDir, ( fileDialog->dirPath() ).ascii() );
	}
	
	QStringList fileNames;
	fileNames.push_back(fileName);
	this->addFrames(fileNames);
}


void ModelHandler::removeFrames()
{
	if(removeFramesButton->isEnabled()) {
		int selectionFrame = frameBar->getSelectionFrame();
		int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
		int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
		int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	
		DomainFacade::getFacade()->removeFrames(lowend, highend);
		statusBar->message( tr("Removed the selected frame"), 2000 );
	}
}


void ModelHandler::newScene()
{
	int activeScene = DomainFacade::getFacade()->getActiveSceneNumber();
	
	if(activeScene >= 0) {
		DomainFacade::getFacade()->newScene(activeScene+1);
	}
	else {
		int numScenes = DomainFacade::getFacade()->getNumberOfScenes();
		if(numScenes > 0) {
			DomainFacade::getFacade()->newScene(numScenes);
		}
		else {
			DomainFacade::getFacade()->newScene(activeScene+1);
		}
	}
	emit modelChanged();
}


void ModelHandler::removeScene()
{
	DomainFacade::getFacade()->removeScene(DomainFacade::getFacade()->
			getActiveSceneNumber());
}

