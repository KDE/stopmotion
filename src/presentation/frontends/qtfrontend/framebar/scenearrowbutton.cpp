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
#include "scenearrowbutton.h"

#include "graphics/icons/openscene.xpm"
#include "graphics/icons/closescene.xpm"

#include <qpainter.h>
#include <qimage.h>


SceneArrowButton::SceneArrowButton( QWidget *parent ) : QWidget(parent)
{
	this->isOpened = false;
	this->iconX = 2;
	moveTimer = new QTimer(this);
	QObject::connect( moveTimer, SIGNAL(timeout()), this, SLOT(moveIcon()) );
}


void SceneArrowButton::setOpened(bool isOpened)
{
	this->isOpened = isOpened;
	this->repaint();
}


void SceneArrowButton::paintEvent( QPaintEvent * )
{
	QPainter paint( this );
	paint.setPen(Qt::gray);
	paint.drawRect(0, 0, width(), height());
	
	QPixmap arrowIcon;
	if (isOpened) {
		arrowIcon = QPixmap( QImage(closescene).scale(width(), height()) );
	}
	else {
		arrowIcon = QPixmap( QImage(openscene).scale(width(), height()) );
	}
	paint.drawPixmap(iconX, 0, arrowIcon);
}


void SceneArrowButton::moveIcon()
{
	this->iconX = (iconX == 0) ? 2 : 0;
	this->repaint();
}


void SceneArrowButton::mouseReleaseEvent( QMouseEvent * e )
{
	if (isOpened) {
		emit clicked();
		moveTimer->stop();
		this->iconX = 2;
		this->repaint();
	}
	else {
		e->ignore();
	}
}


void SceneArrowButton::enterEvent( QEvent * )
{
	if (isOpened) {
		moveTimer->start(400, false);
	}
}


void SceneArrowButton::leaveEvent( QEvent * )
{
	moveTimer->stop();
	this->iconX = 2;
	this->repaint();
}
