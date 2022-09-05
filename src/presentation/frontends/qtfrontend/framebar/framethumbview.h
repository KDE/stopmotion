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
#ifndef FRAMETHUMBVIEW_H
#define FRAMETHUMBVIEW_H

#include <QPoint>
#include <QString>

#include "thumbview.h"

class FrameBar;
class QMouseEvent;
class QDropEvent;
class QPaintEvent;
class QWidget;

/**
 * Widget representing a frame in the animation.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FrameThumbView : public ThumbView {
public:
	/**
	 * Creates and sets up the framethumbview.
	 * @param frameBar the framebar for communicating with it.
	 * @param parent the parent widget.
	 * @param number the number of this widget in the framebar.
	 */
	FrameThumbView(FrameBar *frameBar, QWidget *parent = 0, int number = 0, const char *name = 0);
	
	/**
	 * Cleans up after the widget.
	 */
	~FrameThumbView();
	
	/**
	 * Overloaded event function to receive mousepress-events.
	 * @param e information about the mousepress-event.
	 */
	void mousePressEvent( QMouseEvent * e );
	
	/**
	 * Overloaded event function to receive mouserelease-events.
	 * @param e information about the mouserelease-event.
	 */
	void mouseReleaseEvent( QMouseEvent * e );
	
	/**
	 * Overloaded event function to receive mouseMoveEvents.
	 * @param me information about the mouseMoveEvent. 
	 */
	void mouseMoveEvent(QMouseEvent *me);
	
	/**
	 * Overloaded event function to receive events when the user double clicks
	 * in the thumbview. Double clicks here cause the preferences menu for this
	 * frame to be shown.
	 * @param e information about the mouseDoubleClickEvent.
	 */
	void mouseDoubleClickEvent ( QMouseEvent * e );
	
	/**
	 * Sets/changes the number of this widget.
	 * @param number the new number of this widget.
	 */
	void setNumber(int number);
	
	/**
	 * Sets whether this widget has sounds.
	 * @param hasSounds 
	 */
	void setHasSounds( bool hasSounds );
	
	/**
	 * Sets whether this thumbview should be selected.
	 * @param selected whether this frame is selected.
	 */
	void setSelected(bool selected);
	
	/**
	 * Notifies the framethumbview that a drop have happened inside its borders. 
	 * @param event information about the event.
	 */
	void contentsDropped(QDropEvent * event);
	
protected:
	/**
	 * Event function which paints the widget.
	 */
	void paintEvent ( QPaintEvent * );
	
private:	
	/** Coordinate for calculating when a drag should start */
	QPoint dragPos;

	QString stringNumber;
	
	/** The width the text should have. (Cached for efficiency reasons)*/
	int textWidth;
	
	/** Whether this frame has sounds attached to it */
	bool hasSounds;
	
	/** Specifies whether this frame is selected. */
	bool selected;
	
	/** Starts an uri drag of the picture in this label. */
	void startDrag();
};

#endif
