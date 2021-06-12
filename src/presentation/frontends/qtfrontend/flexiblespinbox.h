
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
#ifndef FLEXIBLESPINBOX_H
#define FLEXIBLESPINBOX_H

#include <QSpinBox>

class QKeyEvent;

/**
 * An extended spin box class.
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FlexibleSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	
	/**
	 * Constructs and initializes the object.
	 * @param parent the parent widget
	 */
	FlexibleSpinBox(QWidget *parent = 0);
	
	/**
	 * Checks what kind of key event the user has requested and 
	 * does actions based on this. 
	 * @param k the key event
	 */
	void keyPressEvent( QKeyEvent * k );
	
public slots:
	/**
	 * Sets a limit for the maximum value. This is based on the
	 * the thumbnails displayed in the framebar. 
	 * @param maxValue maximum value (index of the last frame in framebar)
	 */
	void setMaximumValue(int maxValue);

	/**
	 * Sets the value to one more than @a index.
	 */
	void setIndexSecond(int, int index);

signals:
	/**
	 * Emits this signal when pressing enter.
	 * @param frameNumber the chosen frame number in this spinbox
	 */
	void spinBoxTriggered(int frameNumber);
	
	/**
	 * Emits this signal when getting an escape event (Key_Escape).
	 */
	void spinBoxCanceled();

protected:
	void showEvent(QShowEvent * event);
};

#endif
