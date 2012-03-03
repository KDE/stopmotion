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
#include "framethumbview.h"

#include "src/domain/domainfacade.h"
#include "mainwindowgui.h"
#include "graphics/icons/note.xpm"

#include <qapplication.h>
#include <qdragobject.h> 
#include <qpainter.h>
#include <sstream>

#include <iostream>


FrameThumbView::FrameThumbView(FrameBar *frameBar, QWidget *parent, int number, 
		const char * name) 
		: ThumbView(frameBar, parent, number, name), hasSounds(false)
{
	char temp[7] = {0};
	sprintf(temp, "%d", number+1);
	textWidth = 5+strlen(temp)*8;
	selected = false;
}


FrameThumbView::~FrameThumbView()
{
}


void FrameThumbView::mousePressEvent( QMouseEvent * e )
{
	//Lets the mainwindowgui also respond to the mousepressevent (hide menues)
	//((MainWindowGUI*)parentWidget())->mousePressEvent( e );
	if(e->button() == LeftButton) {
		if(!frameBar->isSelecting()) {
			//2 "unnecessary" int initialization here, but it is better than having 8 extra
			//retrieval funtions.
			int selectionFrame = frameBar->getSelectionFrame();
			int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
			int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
			int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
			
			//If the user presses inside the selection area this shouldn't trigger an
			//setActiveFrame before the mouse button is released. The reason for this is
			//to give the user a chance to drag the items. See mouseReleaseEvent(...)
			if( number > highend || number < lowend) {
				DomainFacade::getFacade()->setActiveFrame(number);
			}
			dragPos = e->pos();
		}
		else {
			frameBar->setSelection(this->number);
		}
	}
}


void FrameThumbView::mouseReleaseEvent( QMouseEvent * e )
{
	Logger::get().logDebug("Releasing mouse button inside thumbview");
	if(e->button() == LeftButton) {
		if(!frameBar->isSelecting()) {
			int selectionFrame = frameBar->getSelectionFrame();
			int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
			int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
			int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
			if( number <= highend || number >= lowend) {
				DomainFacade::getFacade()->setActiveFrame(number);
			}
		}
	}
}


void FrameThumbView::mouseMoveEvent(QMouseEvent *me)
{
	if(me->state() & LeftButton) {
		int distance = (me->pos() - dragPos).manhattanLength();
		if(distance > QApplication::startDragDistance()) {
			startDrag();
		}
	}
	QLabel::mouseMoveEvent(me);
}


void FrameThumbView::mouseDoubleClickEvent( QMouseEvent * )
{
	frameBar->showPreferencesMenu();
}


/**
 *@todo the string should perhaps be cached away
 */
void FrameThumbView::paintEvent ( QPaintEvent * paintEvent )
{
	QLabel::paintEvent(paintEvent);
	
	QPainter painter( this );
	
	if(selected) {
		painter.fillRect( 4, 5, textWidth, 14, QBrush(Qt::white) );
		painter.setPen( Qt::black );
	}
	else {
		painter.fillRect( 4, 5, textWidth, 14, QBrush(Qt::black) );
		painter.setPen( Qt::white );
	}
		
	stringstream ss;
	ss << number+1;
	painter.drawText( 5, 17, ss.str().c_str() );
	
	if(this->hasSounds) {
		QPixmap noteIcon = QPixmap(note);

		painter.drawPixmap( width()-32, 0, noteIcon);
	}
}


void FrameThumbView::startDrag()
{
	//If the drag ends on a scene this tells the scene that it is frames who are 
	//being moved.
	frameBar->setMovingScene(-1);
	
	QStrList lst;
	
	int selectionFrame = frameBar->getSelectionFrame();
	int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
	for(int i=lowend; i<=highend; i++) {
		lst.append( DomainFacade::getFacade()->getFrame(i)->getImagePath() );
	}
	
	QUriDrag *drag = new QUriDrag( lst, this );
	drag->setPixmap(*pixmap());
	drag->drag();
}


void FrameThumbView::setHasSounds( bool hasSounds )
{
	this->hasSounds = hasSounds;
	repaint();
}


void FrameThumbView::setNumber( int number )
{
	ThumbView::setNumber(number);
	
	char temp[8] = {0};
	sprintf(temp, "%d", number+1);
	textWidth = 5+strlen(temp)*8;
	
	repaint();
}


void FrameThumbView::setSelected(bool selected)
{
	this->selected = selected;
	if(selected) {
		setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		setLineWidth(5);
		setMidLineWidth(6);
	}
	else {
		setFrameShape(QFrame::NoFrame);
	}
}


/**
 *@todo FIXME this doesn't work between applications of this type because of the
 *if-check (event->source() != 0).
 */
void FrameThumbView::contentsDropped(QDropEvent * event)
{
	//if(event->spontaneous() == false) {
	if( (event->source() != 0) && (frameBar->getMovingScene() == -1) ) {
		Logger::get().logDebug("Moving picture");
		
		int selectionFrame = frameBar->getSelectionFrame();
		int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
		unsigned int highend = (selectionFrame > activeFrame ) 
				? selectionFrame : activeFrame;
		unsigned int lowend = (selectionFrame < activeFrame ) 
				? selectionFrame : activeFrame;
		
		DomainFacade::getFacade()->moveFrames(lowend, highend, this->number);
		
	}
	else {
		Logger::get().logDebug("Adding picture(s)");
		DomainFacade::getFacade()->setActiveFrame(this->number);
		
		if( QUriDrag::canDecode(event) ) {
				
			QStringList fileNames;
			if( QUriDrag::decodeLocalFiles(event, fileNames) ) {
				std::vector<char*> fNames;
				QStringList::Iterator it = fileNames.begin();
				while(it != fileNames.end() ) {
					QString fileName = *it;
					char *f = (char*)fileName.ascii();
					fNames.push_back( f );
					++it;
				}
				DomainFacade::getFacade()->addFrames(fNames);
			}
		}
	}
}


void FrameThumbView::resizeThumb(int height)
{
	QPixmap p = *pixmap();
	QImage img = p.convertToImage();
	setPixmap( QPixmap(QImage(img).
			scale(height-3, height)));
	
	
	setMinimumHeight(height);
	setMaximumHeight(height);
	setMinimumWidth(height);
	setMaximumWidth(height);
}
