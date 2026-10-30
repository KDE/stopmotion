/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "scenearrowbutton.h"

#include "graphics/icons/openscene.xpm"
#include "graphics/icons/closescene.xpm"

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QPaintEvent>


SceneArrowButton::SceneArrowButton( QWidget *parent ) : QWidget(parent)
{
	this->isOpened = false;
	this->iconX = 2;
	moveTimer = new QTimer(this);
	QObject::connect( moveTimer, &QTimer::timeout, this, &SceneArrowButton::moveIcon );
}


void SceneArrowButton::setOpened(bool isOpened)
{
	this->isOpened = isOpened;
	update();
}


void SceneArrowButton::paintEvent( QPaintEvent * )
{
	QPainter paint( this );
	paint.setPen(Qt::gray);
	paint.drawRect(0, 0, width(), height());
	
	QPixmap arrowIcon;
	if (isOpened) {
		arrowIcon = QPixmap::fromImage( QImage(closescene).scaled(width(), height()) );
	}
	else {
		arrowIcon = QPixmap::fromImage( QImage(openscene).scaled(width(), height()) );
	}
	paint.drawPixmap(iconX, 0, arrowIcon);
}


void SceneArrowButton::moveIcon()
{
	this->iconX = (iconX == 0) ? 2 : 0;
	update();
}


void SceneArrowButton::mouseReleaseEvent( QMouseEvent * e )
{
	if (isOpened) {
		emit clicked();
		moveTimer->stop();
		this->iconX = 2;
		update();
	}
	else {
		e->ignore();
	}
}


void SceneArrowButton::enterEvent( QEvent * )
{
	if (isOpened) {
		moveTimer->start(400);
		moveTimer->setSingleShot(false);
	}
}


void SceneArrowButton::leaveEvent( QEvent * )
{
	moveTimer->stop();
	this->iconX = 2;
	update();
}
