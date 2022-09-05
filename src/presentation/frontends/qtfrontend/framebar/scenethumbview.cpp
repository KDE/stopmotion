/***************************************************************************
 *   Copyright (C) 2005-2017 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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

#include "logger.h"
#include "thumbdragger.h"
#include "filenamesfromurlsiterator.h"
#include "framebar.h"
#include "scenearrowbutton.h"
#include "thumbview.h"
#include "graphics/icons/clapper.xpm"
#include "src/domain/domainfacade.h"

#include <QApplication>
#include <QDropEvent>
#include <QIcon>
#include <QImage>
#include <QList>
#include <QMimeData>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>
#include <QUrl>

class QWidget;

SceneThumbView::SceneThumbView(FrameBar *frameBar, QWidget *parent, int number,
		const char * name)
		: ThumbView(frameBar, parent, number, name) {
	this->isOpened = false;

	arrowButton = new SceneArrowButton(this);
	int width = getFrameBar()->getFrameWidth();
	int height = getFrameBar()->getFrameHeight();
	arrowButton->setGeometry( width - width / 4, height / 9, width / 6, width / 6 );
	arrowButton->show();
	QObject::connect( arrowButton, SIGNAL(clicked()), this, SLOT(closeScene()) );

	f.setPointSize(12);
	centerIcon = QPixmap(clapper);
}


SceneThumbView::~SceneThumbView() {
}


void SceneThumbView::setOpened( bool isOpened ) {
	this->isOpened = isOpened;
	arrowButton->setOpened(isOpened);
	DomainFacade* facade = DomainFacade::getFacade();
	if (!isOpened && (facade->getSceneSize(getNumber()) > 0)) {
		const char *path = facade->getImagePath(getNumber(), 0);
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
	paint.drawText( 7, width / 4, QString("%1").arg(getNumber() + 1) );

	if (!isOpened && (DomainFacade::getFacade()->getSceneSize(getNumber()) > 0) ) {
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
	getFrameBar()->updateNewActiveScene(getNumber());
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
	getFrameBar()->setMovingScene(getNumber());

	QDrag *drag = new ThumbDragger(this);
	QMimeData *mimeData = new QMimeData;
	QList<QUrl> urls;
	mimeData->setUrls(urls);
	drag->setMimeData(mimeData);
	drag->setPixmap(centerIcon);

	drag->exec(Qt::MoveAction);
}


void SceneThumbView::closeScene() {
	if ( getNumber() >= 0) {
		if (getFrameBar()->getActiveScene() == getNumber() && getNumber() > 0) {
			getFrameBar()->updateNewActiveScene(getNumber() - 1);
		} else {
			getFrameBar()->updateNewActiveScene(getNumber());
		}
	}
}


/**
 * Scenes should be dropped after this frame if moving right or before if
 * moving left. This makes sense to users, who expect the dropped scene to
 * appear where they dropped it. Frames should always move to the start of the
 * scene they are dropped in.
 */
void SceneThumbView::contentsDropped(QDropEvent *event) {
	DomainFacade* facade = DomainFacade::getFacade();
	int sceneNumber = getNumber();
	int movingScene = getFrameBar()->getMovingScene();
	int activeScene= getFrameBar()->getActiveScene();
	if (event->source() == 0) {
		if ( event->mimeData()->hasUrls() ) {
			QList<QUrl> urls = event->mimeData()->urls();
			FileNamesFromUrlsIterator fNames(urls.begin(), urls.end());
			DomainFacade::getFacade()->addFrames(getNumber(), 0, fNames);
		}
	} else if (movingScene == -1) {
		// moving frames into a scene
		int selectionFrame = getFrameBar()->getSelectionAnchor();
		int activeFrame = getFrameBar()->getActiveFrame();
		int highend = (selectionFrame > activeFrame)?
				selectionFrame : activeFrame;
		int lowend = (selectionFrame < activeFrame )?
				selectionFrame : activeFrame;
		bool movingRight = activeScene < sceneNumber;
		if (movingRight || sceneNumber == 0) {
			facade->moveFrames(activeScene, lowend,
					highend - lowend + 1, sceneNumber, 0);
		} else {
			int sceneDest = sceneNumber - 1;
			int sceneSize = DomainFacade::getFacade()->getSceneSize(sceneDest);
			facade->moveFrames(activeScene, lowend,
					highend - lowend + 1, sceneDest, sceneSize);
		}
	} else if (movingScene != sceneNumber) {
		Logger::get().logDebug("Moving scene");
		int destination = movingScene < sceneNumber?
				sceneNumber + 1 : sceneNumber;
		facade->moveScene(movingScene, destination);
	}
}
