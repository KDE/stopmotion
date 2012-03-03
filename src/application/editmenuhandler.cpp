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
#include "editmenuhandler.h"

#include "src/domain/domainfacade.h"
#include "mainwindowgui.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h> 


EditMenuHandler::EditMenuHandler ( QObject *parent, QStatusBar *sb, 
		FrameBar *frameBar, const char *name ) 
		: QObject(parent, name), statusBar(sb), frameBar(frameBar)
{
	gotoMenu = NULL;
}


void EditMenuHandler::setGotoMenu( MenuFrame * gotoMenu )
{
	this->gotoMenu = gotoMenu;
}


void EditMenuHandler::gotoFrame(int frameNumber)
{
	DomainFacade::getFacade()->setActiveFrame(frameNumber);
	this->closeGotoMenu();
}


void EditMenuHandler::closeGotoMenu()
{
	gotoMenu->close( (QWidget*)this->parent() );
}


void EditMenuHandler::undo()
{
	DomainFacade::getFacade()->undo();
}


void EditMenuHandler::redo()
{
	DomainFacade::getFacade()->redo();
}


void EditMenuHandler::cut()
{
	/*copy();
	
	int selectionFrame = frameBar->getSelectionFrame();
	int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	
	DomainFacade::getFacade()->removeFrames(lowend, highend);*/
}


void EditMenuHandler::copy()
{
	QStrList lst;
	QString t;
	
	int selectionFrame = frameBar->getSelectionFrame();
	int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	
	for(int i=lowend; i<=highend; i++) {
		t = DomainFacade::getFacade()->getFrame(i)->getImagePath();
		
		t.prepend("file:");
		t.append("\n");
		
		lst.append(t);
	}
	
	QUriDrag *drag = new QUriDrag( lst, (QWidget*)this->parent() );
	QApplication::clipboard()->setData(drag);
}


void EditMenuHandler::paste()
{
	QString text;
	if ( QTextDrag::decode(QApplication::clipboard()->data(), text) );
	
	QStringList uris;
	QStringList fileNames;
	fileNames = QStringList::split("\n", text);
	
	QStringList temp;
	QStringList::Iterator it = fileNames.begin();
	while(it != fileNames.end() ) {
		QString fileName = *it;
		temp = QStringList::split("file:", fileName);
		fileName = temp.last();
		//fileName.prepend("/");
		if(fileName[0] == '/') {
			uris.push_back(fileName);
		}
		++it;
	}
	emit addFrames(uris);
}
