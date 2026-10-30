/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "framethumbview.h"

#include "framebar.h"
#include "logger.h"
#include "thumbdragger.h"
#include "thumbview.h"
#include "filenamesfromurlsiterator.h"
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

class QWidget;


FrameThumbView::FrameThumbView(FrameBar *frameBar, QWidget *parent, int number,
		const char *name)
		: ThumbView(frameBar, parent, number, name) {
	stringNumber = QString("%1").arg(number + 1);
	textWidth = 5 + stringNumber.length() * 8;
	selected = false;
	hasSounds = false;
}


FrameThumbView::~FrameThumbView() {
}


void FrameThumbView::mousePressEvent( QMouseEvent * e ) {
	if (e->button() == Qt::LeftButton) {
		if ( !getFrameBar()->isSelecting() ) {
			int selectionFrame = getFrameBar()->getSelectionAnchor();
			int activeScene = getFrameBar()->getActiveScene();
			int activeFrame = getFrameBar()->getActiveFrame();
			int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
			int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
			
			// If the user presses inside the selection area this shouldn't trigger
			// setActiveFrame before the mouse button is released. The reason for this is
			// to give the user a chance to drag the items. See mouseReleaseEvent(...)
			if (getNumber() > highend || getNumber() < lowend) {
				getFrameBar()->updateNewActiveFrame(activeScene, getNumber());
			}
			dragPos = e->pos();
		} else {
			getFrameBar()->setSelection(getNumber());
		}
	}
}


void FrameThumbView::mouseReleaseEvent( QMouseEvent * e ) {
	Logger::get().logDebug("Releasing mouse button inside thumbview");
	if (e->button() == Qt::LeftButton) {
		if ( !getFrameBar()->isSelecting() ) {
			int selectionFrame = getFrameBar()->getSelectionAnchor();
			int activeScene = getFrameBar()->getActiveScene();
			int activeFrame = getFrameBar()->getActiveFrame();
			int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
			int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
			if (getNumber() <= highend || getNumber() >= lowend) {
				getFrameBar()->updateNewActiveFrame(activeScene, getNumber());
			}
		}
	}
}


void FrameThumbView::mouseMoveEvent(QMouseEvent *me) {
	if (me->buttons() & Qt::LeftButton) {
		int distance = (me->pos() - dragPos).manhattanLength();
		if (distance > QApplication::startDragDistance()) {
			startDrag();
		}
	}
	QLabel::mouseMoveEvent(me);
}


void FrameThumbView::mouseDoubleClickEvent( QMouseEvent * ) {
	getFrameBar()->showPreferencesMenu();
}


void FrameThumbView::paintEvent (QPaintEvent * paintEvent) {
	QLabel::paintEvent(paintEvent);
	QPainter painter( this );
	
	if (selected) {
		painter.fillRect( 4, 5, textWidth, 14, QBrush(Qt::white) );
		painter.setPen( Qt::black );
	} else {
		painter.fillRect( 4, 5, textWidth, 14, QBrush(Qt::black) );
		painter.setPen( Qt::white );
	}
		
	painter.drawText(5, 17, stringNumber);
	
	if (this->hasSounds) {
		painter.drawPixmap( width() - 32, 0, QPixmap(note) );
	}
}
	

void FrameThumbView::startDrag() {
	// If the drag ends on a scene this tells the scene that it is frames that
	// are being moved.
	getFrameBar()->setMovingScene(-1);

	QList<QUrl> urls;

	int selectionFrame = getFrameBar()->getSelectionAnchor();
	int activeScene = getFrameBar()->getActiveScene();
	int activeFrame = getFrameBar()->getActiveFrame();
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
	DomainFacade* facade = DomainFacade::getFacade();
	for (int i = lowend; i <= highend; ++i) {
		const char* imagePath = facade->getImagePath(activeScene, i);
		if (imagePath)
			urls.append(QUrl::fromLocalFile(imagePath));
	}

	QDrag *drag = new ThumbDragger(this);
	QMimeData *mimeData = new QMimeData;

	mimeData->setUrls(urls);
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap());
	drag->exec(Qt::MoveAction);
}


void FrameThumbView::setHasSounds( bool hasSounds ) {
	this->hasSounds = hasSounds;
	update();
}


void FrameThumbView::setNumber( int number ) {
	ThumbView::setNumber(number);
	stringNumber = QString("%1").arg(number + 1);
	textWidth = 5 + stringNumber.length() * 8;
	update();
}


void FrameThumbView::setSelected(bool selected) {
	this->selected = selected;
	if (selected) {
		setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		setLineWidth(5);
		setMidLineWidth(6);
	} else {
		setFrameShape(QFrame::NoFrame);
	}
}

/**
 * Moves the frames dropped to just after this frame if the frames are being
 * moved to the right, or just before this frame if the frames are being moved
 * to the left. This slightly bizarre behaviour feels right to users who expect
 * a moved frame to appear where they dropped it.
 */
void FrameThumbView::contentsDropped(QDropEvent * event) {
	DomainFacade* facade = DomainFacade::getFacade();
	int activeScene = getFrameBar()->getActiveScene();
	if (event->source() == nullptr) {
		Logger::get().logDebug("Adding picture(s)");
		if ( event->mimeData()->hasUrls() ) {
			QList<QUrl> urls = event->mimeData()->urls();
			FileNamesFromUrlsIterator fNames(urls.begin(), urls.end());
			DomainFacade::getFacade()->addFrames(activeScene, getNumber(), fNames);
		}
	} else if (getFrameBar()->getMovingScene() == -1) {
		Logger::get().logDebug("Moving picture");
		int selectionFrame = getFrameBar()->getSelectionAnchor();
		int activeFrame = getFrameBar()->getActiveFrame();
		int highend = (selectionFrame > activeFrame)?
				selectionFrame : activeFrame;
		int lowend = (selectionFrame < activeFrame )?
				selectionFrame : activeFrame;
		int destination = activeFrame < getNumber()? getNumber() + 1 : getNumber();
		facade->moveFrames(activeScene, lowend,
				highend - lowend + 1, activeScene, destination);
	}
}
