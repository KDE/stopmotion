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
#include "scenethumbview.h"

#include "graphics/icons/clapper.xpm"
#include "src/domain/domainfacade.h"

#include <qpainter.h>
#include <qimage.h>
#include <qapplication.h>
#include <qdragobject.h> 
#include <sstream>
#include <qpushbutton.h>
#include <qiconset.h>
#include <iostream>


SceneThumbView::SceneThumbView(FrameBar *frameBar, QWidget *parent, int number, 
	const char * name) 
 : ThumbView(frameBar, parent, number, name)
{
	this->isOpened = false;
	
	arrowButton = new SceneArrowButton(this);
	int width = this->width();
	arrowButton->setGeometry( width-width/4, width/9, width/6, width/6 );
	QObject::connect( arrowButton, SIGNAL(clicked()), this, SLOT(closeScene()) );
	
	f.setPointSize(15);
	centerIcon = QPixmap(clapper);
}


SceneThumbView::~SceneThumbView()
{
}


void SceneThumbView::setOpened( bool isOpened )
{
	this->isOpened = isOpened;
	arrowButton->setOpened(isOpened);
	
	if (!isOpened && (DomainFacade::getFacade()->getSceneSize(number) > 0) ) {
		centerIcon = QPixmap( QImage(
				DomainFacade::getFacade()->getFrame(0, number)->getImagePath()).
				scale(width()/2, height()/2) );
	}
	else {
		centerIcon = QPixmap(clapper);
	}
	
	this->repaint();
}


bool SceneThumbView::getIsOpened()
{
	return isOpened;
}


/**
 * @todo the width can be cached somewhere so that the function width() don't have
 * to be called for every frame and scene thumbview.
 */
void SceneThumbView::paintEvent ( QPaintEvent * )
{
	int width = this->width();
	
	QPainter paint( this );
	paint.setPen( Qt::black );
	paint.setFont(f);
	paint.drawText( 7, (width/4), QString("%1").arg(number+1));
	
	if (!isOpened && (DomainFacade::getFacade()->getSceneSize(number) > 0) ) {
		paint.drawPixmap(width/4, width/3, centerIcon);
	}
	else {
		paint.drawPixmap(width/2-16, width/2-10, centerIcon);
	}
}


void SceneThumbView::mousePressEvent(QMouseEvent *e)
{
	//For calculating the manhattenLength to avoid unvanted
	//drags.
	dragPos = e->pos();
}


void SceneThumbView::mouseReleaseEvent( QMouseEvent * )
{
	if ((DomainFacade::getFacade()->getActiveSceneNumber() != this->number) &&
			(frameBar->isOpeningScene() == false) ) {
		frameBar->setOpeningScene(true);
		DomainFacade::getFacade()->setActiveScene(number);
		frameBar->setOpeningScene(false);
	}
}


void SceneThumbView::mouseMoveEvent(QMouseEvent * me)
{
	if (me->state() & LeftButton) {
		int distance = (me->pos() - dragPos).manhattanLength();
		if (distance > QApplication::startDragDistance()) {
			startDrag();
		}
	}
	QLabel::mouseMoveEvent(me);
}


void SceneThumbView::startDrag()
{
	Logger::get().logDebug("Starting drag of the scene");
	frameBar->setMovingScene(this->number);
	QStrList lst;
	QUriDrag *drag = new QUriDrag( lst, this );
	drag->drag();
}


void SceneThumbView::closeScene()
{
	if (frameBar->isOpeningScene() == false) {
		if (DomainFacade::getFacade()->getActiveSceneNumber() == this->number) {
			DomainFacade::getFacade()->setActiveScene(-1);
		}
		else {
			DomainFacade::getFacade()->setActiveScene(number);
		}
	}
}


void SceneThumbView::contentsDropped(QDropEvent *event)
{
	int movingScene = frameBar->getMovingScene();
	if ((event->source() != 0) && (movingScene != this->number) && 
			(movingScene != -1) ) {
		
		Logger::get().logDebug("Moving scene");
		DomainFacade::getFacade()->moveScene(movingScene, this->number);
	}
}


void SceneThumbView::resizeThumb(int height)
{
	setMinimumHeight(height);
	setMaximumHeight(height);
	setMinimumWidth(height);
	setMaximumWidth(height);
	
	int width = this->width();
	arrowButton->setGeometry( width-width/4, width/9, width/6, width/6 );
	
	f.setPointSize(width/6);
	
	if (!isOpened && (DomainFacade::getFacade()->getSceneSize(number) > 0) ) {
		centerIcon = QPixmap( QImage(DomainFacade::getFacade()->getFrame(0, number)->
					getImagePath()).scale(width/2, height/2) );
	}
}
