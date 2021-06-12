/***************************************************************************
 *   Copyright (C) 2005-2008 by fredrik                                         *
 *   fredrik@Elanor                                                        *
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
#ifndef SCENEARROWBUTTON_H
#define SCENEARROWBUTTON_H

#include <QWidget>
#include <QTimer>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEvent>
#include <QPaintEvent>


/**
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class SceneArrowButton : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Sets up the scenearrowbutton
	 * @param parent the parent of the widget.
	 */
	SceneArrowButton( QWidget *parent=0 );
	
	/**
	 * Sets if the scene is opened or closed.
	 * @param isOpened true if the scene is opened.
	 */
	void setOpened(bool isOpened);
	
public slots:
	/**
	 * Slot for moving the icon to create an animation-like effect.
	 */
	void moveIcon();
	
protected:
	/** Overloaded function to paint the widget.
	 * @param p information about the paintEvent.
	 */
	virtual void paintEvent ( QPaintEvent *p);
	
	/**
	 * Overloaded event function to receive mouserelease-events.
	 * @param e information about the mouserelease-event.
	 */
	void mouseReleaseEvent( QMouseEvent * e );
	
	/**
	 * Overloaded event function for receiving information when the mouse
	 * enter the widget
	 * @param e information about the event
	 */
	void enterEvent ( QEvent * e);
	
	/**
	 * Overloaded event function for receiving information when the mouse
	 * leaves the widget
	 * @param e information about the event
	 */
	void leaveEvent ( QEvent * e);
	
private:
	bool isOpened;
	int iconX;
	
	QTimer *moveTimer;
	
signals:
	void clicked();
};

#endif
