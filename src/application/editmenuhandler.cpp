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
#include "src/application/editmenuhandler.h"

#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/mainwindowgui.h"
#include "src/presentation/frontends/qtfrontend/menuframe.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"
#include "modelhandler.h"

#include <QApplication>
#include <QClipboard>
#include <QDrag> 
#include <QStringList>
#include <QList>
#include <QUrl>
#include <QStatusBar>


EditMenuHandler::EditMenuHandler ( QObject *parent, QStatusBar *sb, 
		FrameBar *frameBar, const char *name ) 
	: QObject(parent), statusBar(sb), frameBar(frameBar)
{
	gotoMenu = 0;
	setObjectName(name);
}


void EditMenuHandler::setGotoMenu( QWidget * gotoMenu )
{
	this->gotoMenu = gotoMenu;
}


void EditMenuHandler::gotoFrame(int frameNumber)
{
	frameBar->updateNewActiveFrame(frameBar->getActiveScene(), frameNumber);
	closeGotoMenu();
}


void EditMenuHandler::closeGotoMenu()
{
	gotoMenu->hide();
}


void EditMenuHandler::undo()
{
	DomainFacade::getFacade()->undo();
}


void EditMenuHandler::redo()
{
	DomainFacade::getFacade()->redo();
}


void EditMenuHandler::copy()
{
	QList<QUrl> urls;

	int selectionFrame = frameBar->getSelectionAnchor();
	int activeScene = frameBar->getActiveScene();
	int activeFrame = frameBar->getActiveFrame();
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;

	DomainFacade* facade = DomainFacade::getFacade();
	for (int i = lowend; i <= highend; ++i) {
		const char* imagePath = facade->getImagePath(activeScene, i);
		if (imagePath)
			urls.append(QUrl::fromLocalFile(imagePath));
	}

	//QDrag *drag = new QDrag((MainWindowGUI*)this->parent());
	QMimeData *mimeData = new QMimeData;

	mimeData->setUrls(urls);
	//drag->setMimeData(mimeData);

	//drag->start(Qt::MoveAction);
	QApplication::clipboard()->setMimeData(mimeData);
}


void EditMenuHandler::paste()
{
	const QMimeData *mimeData = QApplication::clipboard()->mimeData();
	if ( mimeData->hasUrls() ) {
		QStringList fileNames;
		QList<QUrl> urls = mimeData->urls();
		int numFrames = urls.size();
		for (int i = 0; i < numFrames; ++i) {
			fileNames.append(urls[i].toLocalFile());
		}
		emit addFrames(fileNames);
	}
}
