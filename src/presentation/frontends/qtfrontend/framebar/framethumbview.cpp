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
#include "framethumbview.h"

#include "src/domain/domainfacade.h"
#include "graphics/icons/note.xpm"

#include <QApplication>
#include <QPainter>
#include <QStringList>
#include <QMouseEvent>
#include <QPixmap>
#include <QPaintEvent>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QList>
#include <QUrl>


FrameThumbView::FrameThumbView(FrameBar *frameBar, QWidget *parent, int number, const char *name)
		: ThumbView(frameBar, parent, number, name)
{
	stringNumber = QString("%1").arg(number + 1);
	textWidth = 5 + stringNumber.length() * 8;
	selected = false;
	hasSounds = false;
}


FrameThumbView::~FrameThumbView()
{
}


void FrameThumbView::mousePressEvent( QMouseEvent * e )
{
	if (e->button() == Qt::LeftButton) {
		if ( !frameBar->isSelecting() ) {
			int selectionFrame = frameBar->getSelectionFrame();
			int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
			int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
			int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
			
			// If the user presses inside the selection area this shouldn't trigger
			// setActiveFrame before the mouse button is released. The reason for this is
			// to give the user a chance to drag the items. See mouseReleaseEvent(...)
			if (number > highend || number < lowend) {
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
	if (e->button() == Qt::LeftButton) {
		if ( !frameBar->isSelecting() ) {
			int selectionFrame = frameBar->getSelectionFrame();
			int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
			int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
			int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
			if (number <= highend || number >= lowend) {
				DomainFacade::getFacade()->setActiveFrame(number);
			}
		}
	}
}


void FrameThumbView::mouseMoveEvent(QMouseEvent *me)
{
	if (me->buttons() & Qt::LeftButton) {
		int distance = (me->pos() - dragPos).manhattanLength();
		if (distance > QApplication::startDragDistance()) {
			startDrag();
		}
	}
	QLabel::mouseMoveEvent(me);
}


void FrameThumbView::mouseDoubleClickEvent( QMouseEvent * )
{
	frameBar->showPreferencesMenu();
}


void FrameThumbView::paintEvent (QPaintEvent * paintEvent)
{
	QLabel::paintEvent(paintEvent);
	QPainter painter( this );
	
	if (selected) {
		painter.fillRect( 4, 5, textWidth, 14, QBrush(Qt::white) );
		painter.setPen( Qt::black );
	}
	else {
		painter.fillRect( 4, 5, textWidth, 14, QBrush(Qt::black) );
		painter.setPen( Qt::white );
	}
		
	painter.drawText(5, 17, stringNumber);
	
	if (this->hasSounds) {
		painter.drawPixmap( width() - 32, 0, QPixmap(note) );
	}
}
	

void FrameThumbView::startDrag()
{
	// If the drag ends on a scene this tells the scene that it is frames who are 
	// being moved.
	frameBar->setMovingScene(-1);
	
	QList<QUrl> urls;
	
	int selectionFrame = frameBar->getSelectionFrame();
	int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
	for (int i = lowend; i <= highend; ++i) {
		urls.append(QUrl::fromLocalFile(DomainFacade::getFacade()->getFrame(i)->getImagePath()));
	}
	
	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;

	mimeData->setUrls(urls);
	drag->setMimeData(mimeData);
	drag->setPixmap(*pixmap());

	//Qt::DropAction dropAction = drag->start(Qt::CopyAction | Qt::MoveAction);
	drag->start(Qt::MoveAction);
	
}


void FrameThumbView::setHasSounds( bool hasSounds )
{
	this->hasSounds = hasSounds;
	update();
}


void FrameThumbView::setNumber( int number )
{
	ThumbView::setNumber(number);
	stringNumber = QString("%1").arg(number + 1);
	textWidth = 5 + stringNumber.length() * 8;
	update();
}


void FrameThumbView::setSelected(bool selected)
{
	this->selected = selected;
	if (selected) {
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
	if ( (event->source() != 0) && (frameBar->getMovingScene() == -1) ) {
		Logger::get().logDebug("Moving picture");
		int selectionFrame = frameBar->getSelectionFrame();
		int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
		unsigned int highend = (selectionFrame > activeFrame) ? selectionFrame : activeFrame;
		unsigned int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
		DomainFacade::getFacade()->moveFrames(lowend, highend, this->number);
	}
	else {
		Logger::get().logDebug("Adding picture(s)");
		DomainFacade::getFacade()->setActiveFrame(this->number);
		
		if ( event->mimeData()->hasUrls() ) {
			QStringList fileNames;
			QList<QUrl> urls = event->mimeData()->urls();
			int numFrames = urls.size();
			for (int i = 0; i < numFrames; ++i) {
				fileNames.append(urls[i].toLocalFile());
			}
			std::vector<char*> fNames;
			QStringList::Iterator it = fileNames.begin();
			while (it != fileNames.end() ) {
				QString fileName = *it;
				const char *f = fileName.toStdString().c_str();
				fNames.push_back( const_cast<char*>(f) );
				++it;
			}
			DomainFacade::getFacade()->addFrames(fNames);
		}
	}
}

