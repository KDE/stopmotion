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
#include "src/presentation/frontends/qtfrontend/framebar/scenethumbview.h"

#include "graphics/icons/clapper.xpm"
#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/framebar/scenearrowbutton.h"

#include <QPainter>
#include <QImage>
#include <QApplication>
#include <QStringList>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QDropEvent>
#include <QPushButton>
#include <QIcon>
#include <QList>
#include <QUrl>


SceneThumbView::SceneThumbView(FrameBar *frameBar, QWidget *parent, int number,
		const char * name)
		: ThumbView(frameBar, parent, number, name) {
	this->isOpened = false;

	arrowButton = new SceneArrowButton(this);
	int width = frameBar->getFrameWidth();
	int height = frameBar->getFrameHeight();
	arrowButton->setGeometry( width - width / 4, height / 9, width / 6, width / 6 );
	arrowButton->show();
	QObject::connect( arrowButton, SIGNAL(clicked()), this, SLOT(closeScene()) );

	f.setPointSize(12);
	centerIcon = QPixmap(clapper);
}


SceneThumbView::~SceneThumbView() {}


void SceneThumbView::setOpened( bool isOpened ) {
	this->isOpened = isOpened;
	arrowButton->setOpened(isOpened);
	DomainFacade* facade = DomainFacade::getFacade();
	if (!isOpened && (facade->getSceneSize(number) > 0)) {
		const char *path = facade->getImagePath(number, 0);
		if (path) {
			QImage half = QImage(path).scaled(width() / 2, height() / 2);
			centerIcon = QPixmap::fromImage(half);
		}
	} else {
		centerIcon = QPixmap(clapper);
	}
	this->update();
}


bool SceneThumbView::getIsOpened() const {
	return isOpened;
}


/**
 * @todo the width can be cached somewhere so that the function width() don't have
 * to be called for every frame and scene thumbview.
 */
void SceneThumbView::paintEvent ( QPaintEvent * ) {
	int width = this->width();

	QPainter paint(this);
	paint.setPen(Qt::black);
	paint.setFont(f);
	paint.drawText( 7, width / 4, QString("%1").arg(number + 1) );

	if (!isOpened && (DomainFacade::getFacade()->getSceneSize(number) > 0) ) {
		paint.drawPixmap(width / 4, width / 3, centerIcon);
	}
	else {
		paint.drawPixmap(width / 2 - 16, width / 2 - 10, centerIcon);
	}
}


void SceneThumbView::mousePressEvent(QMouseEvent *e) {
	//For calculating the manhattan length to avoid unwanted drags.
	dragPos = e->pos();
}


void SceneThumbView::mouseReleaseEvent( QMouseEvent * ) {
	frameBar->updateNewActiveFrame(number, 0);
}


void SceneThumbView::mouseMoveEvent(QMouseEvent * me) {
	// it should probably be me->button() here...
	if (me->buttons() & Qt::LeftButton) {
		int distance = (me->pos() - dragPos).manhattanLength();
		if (distance > QApplication::startDragDistance()) {
			startDrag();
		}
	}
	QLabel::mouseMoveEvent(me);
}


void SceneThumbView::startDrag() {
	Logger::get().logDebug("Starting drag of the scene");
	frameBar->setMovingScene(this->number);

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;
	QList<QUrl> urls;
	mimeData->setUrls(urls);
	drag->setMimeData(mimeData);
	drag->setPixmap(centerIcon);

	//Qt::DropAction dropAction = drag->start(Qt::CopyAction | Qt::MoveAction);
	drag->start(Qt::MoveAction);
}


void SceneThumbView::closeScene() {
	if ( number >= 0) {
		if (frameBar->getActiveScene() == number && number > 0) {
			frameBar->updateNewActiveFrame(number - 1, 0);
		} else {
			frameBar->updateNewActiveFrame(number, 0);
		}
	}
}


void SceneThumbView::contentsDropped(QDropEvent *event) {
	int movingScene = frameBar->getMovingScene();
	if ((event->source() != 0) && (movingScene != this->number) && (movingScene != -1) ) {
		Logger::get().logDebug("Moving scene");
		DomainFacade::getFacade()->moveScene(movingScene, this->number);
	}
}

