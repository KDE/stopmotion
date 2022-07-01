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
#include "framebar.h"

#include "framepreferencesmenu.h"
#include "framethumbview.h"
#include "logger.h"
#include "scenethumbview.h"
#include "thumbview.h"
#include "src/domain/domainfacade.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFrame>
#include <QImage>
#include <QImageReader>
#include <QMimeData>
#include <QPixmap>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

static QImage tryReadImage(const char *filename) {
    if (!filename) {
        qWarning() << "Couldn't read image: Invalid file name";
        return QImage();
    }

    QImageReader imageReader(QString::fromLocal8Bit(filename));
    const QImage image = imageReader.read();
    if (!image.isNull())
        return image;

    if (imageReader.error() != QImageReader::UnsupportedFormatError
        && imageReader.error() != QImageReader::InvalidDataError) {
        qWarning() << "Couldn't read image:" << imageReader.errorString();
        return image;
    }

    // At this point we most likely failed to read the image because the suffix
    // of the file was different from the actual image format.
    // Loop through all the supported formats and see if we can find a match.
    const QList<QByteArray> supportedImageFormats = QImageReader::supportedImageFormats();
    for (int i = 0; i < supportedImageFormats.size(); ++i) {
        QImageReader anotherImageReader(filename);
        anotherImageReader.setFormat(supportedImageFormats.at(i));
        const QImage anotherImage = anotherImageReader.read();
        if (!anotherImage.isNull())
            return anotherImage;
    }

    qWarning() << "Couldn't read image:" << imageReader.errorString();
    return image;
}

FrameBar::FrameBar(QWidget *parent)
		: QScrollArea(parent) {
	preferencesMenu = 0;
	activeFrame = -1;
	activeScene = -1;
	activeSceneSize = 0;
	movingScene = 0;
	openingScene = false;
	selecting = false;
	selectionFrame = -1;
	scrollDirection = 0;

	lowerScrollAreaX = this->x() + FRAME_WIDTH;
	upperScrollAreaX = this->width() - FRAME_WIDTH;

	lowerAccelScrollAreaX = lowerScrollAreaX - (FRAME_WIDTH >> 1);
	upperAccelScrollAreaX = upperScrollAreaX + (FRAME_WIDTH >> 1);

	minScrollAreaX = lowerScrollAreaX - FRAME_WIDTH + 20;
	maxScrollAreaX = upperScrollAreaX + FRAME_WIDTH - 20;

	minScrollAreaY = this->y() + 20;
	maxScrollAreaY = this->y() + FRAME_HEIGHT - 20;

	scrollTimer = new QTimer(this);
	connect(scrollTimer, SIGNAL(timeout()), this, SLOT(scroll()));
	scrollBar = horizontalScrollBar();

	mainWidget = new QWidget;
	mainWidget->setMinimumHeight(FRAME_HEIGHT);
	mainWidget->setMaximumHeight(FRAME_HEIGHT);
	mainWidget->setMinimumWidth(FRAME_WIDTH);
	mainWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

	setWidget(mainWidget);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	setMaximumHeight(FRAME_HEIGHT + horizontalScrollBar()->height() - 5);
	setMinimumHeight(FRAME_HEIGHT + horizontalScrollBar()->height() - 5);
	setBackgroundRole(QPalette::Dark);
	setAcceptDrops(true);

	Logger::get().logDebug("FrameBar is attached to the model");
	DomainFacade::getFacade()->attach(this);
}

FrameBar::~FrameBar() {
}

void FrameBar::updateAdd(int scene, int index, int numFrames) {
	if (scene == activeScene) {
		Logger::get().logDebug("Adding in framebar");
		addFrames(index, numFrames);
		emit modelSizeChanged(DomainFacade::getFacade()->getModelSize());
	}
	updateNewActiveFrame(scene, index + numFrames - 1);
	setSelection(index);
	doScroll();
}

void FrameBar::updateRemove(int scene, int fromFrame, int toFrame) {
	if (scene == activeScene) {
		Logger::get().logDebug("Receiving notification about the removal of a frame in the model");
		removeFrames(fromFrame, toFrame);
		emit modelSizeChanged(DomainFacade::getFacade()->getModelSize());
	}
	updateNewActiveFrame(scene, fromFrame - 1);
}

void FrameBar::updateMove(int fromScene, int fromFrame, int count,
		int toScene, int toFrame) {
	if (toScene == activeScene) {
		if (fromScene == activeScene) {
			changeSelectionHighlight(fromFrame + count - 1, fromFrame);
			Logger::get().logDebug("Moving in framebar");
			moveFrames(fromFrame, fromFrame + count - 1, toFrame);
			if (fromFrame < toFrame)
				toFrame -= count;
		} else {
			changeSelectionHighlight(-1, -1);
			Logger::get().logDebug("Moving to framebar");
			addFrames(toFrame, count);
		}
	} else {
		Logger::get().logDebug("Moving to a different scene");
		setActiveScene(toScene);
	}
	Logger::get().logDebug("Setting new active frame in FrameBar");
	setActiveFrameAndSelection(toFrame + count - 1, toFrame);
	doScroll();
	doActiveFrameNotifications();
}

void FrameBar::updateSoundChanged(int sceneNumber, int frameNumber) {
	if (activeSceneSize <= frameNumber) {
		resync();
	} else {
		getFrameThumb(frameNumber, true);
	}
	setActiveScene(sceneNumber);
	setActiveFrameAndSelection(frameNumber, frameNumber);
	showPreferencesMenu();
}

void FrameBar::fixPreferencesMenu() {
	if ( preferencesMenu->isVisible() ) {
		if (activeFrame >= 0) {
			showPreferencesMenu();
		}
		else {
			preferencesMenu->close();
		}
	}
}

void FrameBar::doActiveFrameNotifications() {
	fixPreferencesMenu();
	// For writing the frame number in the frame number display
	emit newActiveFrame( QString(tr("Frame number: ")) + QString("%1").arg(activeFrame + 1) );
	emit newActiveFrame(activeScene, activeFrame);
}

void FrameBar::updateNewActiveScene(int sceneNumber) {
	setActiveScene(sceneNumber);
	setActiveFrame(activeSceneSize - 1);
	doActiveFrameNotifications();
}

void FrameBar::updateNewActiveFrame(int sceneNumber, int frameNumber) {
	setActiveScene(sceneNumber);
	setActiveFrame(frameNumber);
	doActiveFrameNotifications();
}

void FrameBar::clear() {
	int size = thumbViews.size();
	for (int i = 0; i < size; ++i) {
		thumbViews[i]->delRef();
	}
	thumbViews.clear();
	activeSceneSize = 0;
}

void FrameBar::updateClear() {
	clear();
	activeFrame = -1;
	activeScene = -1;
	emit newActiveFrame(activeScene, activeFrame);
}


void FrameBar::updateAnimationChanged(int sceneNumber, int frameNumber) {
	if (sceneNumber != activeScene)
		return;
	if (activeSceneSize <= frameNumber)
		resync();
	const char *path = DomainFacade::getFacade()->getImagePath(sceneNumber,
				frameNumber);
	if (!path)
		return;
	QImage scaled = tryReadImage(path).scaled(FRAME_WIDTH, FRAME_HEIGHT);
	ThumbView* thumb = getFrameThumb(frameNumber);
	thumb->setPixmap(QPixmap::fromImage(scaled));
	thumb->update();
}

void FrameBar::fileChanged(const QString& path) {
	DomainFacade* facade = DomainFacade::getFacade();
	int sceneCount = facade->getNumberOfScenes();
	if (activeScene < 0 || sceneCount <= activeScene)
		return;
	int sceneSize = facade->getSceneSize(activeScene);
	const char* pathStr = path.toLocal8Bit();
	for (int i = 0; i != sceneSize; ++i) {
		const char* ip = facade->getImagePath(activeScene, i);
		// This is a bit slow because we have to search through
		// the /home/tim/.stopmotion/ bit of the path each time.
		if (strcmp(pathStr, ip) == 0) {
			ThumbView* thumb = getFrameThumb(i, false);
			setThumbImage(thumb, pathStr);
		}
	}
}

void FrameBar::fixSize() {
	mainWidget->resize((FRAME_WIDTH + SPACE) * thumbViews.size() - SPACE,
			FRAME_HEIGHT);
}

void FrameBar::resync() {
	clear();
	DomainFacade* facade = DomainFacade::getFacade();
	int sceneCount = facade->getNumberOfScenes();
	int scene = 0 <= activeScene && activeScene < sceneCount?
			activeScene : -1;
	activeScene = -1;
	int sceneSize = 0 <= scene? facade->getSceneSize(scene) : 0;

	std::vector<ThumbView*>::size_type thumbCount = sceneCount + sceneSize;
	thumbViews.insert(thumbViews.begin(), thumbCount, 0);
	activeScene = scene;
	activeSceneSize = sceneSize;
	if (scene != activeScene || scene < 0 || sceneSize <= activeFrame) {
		activeFrame = -1;
		selectionFrame = -1;
	}
	for (int i = 0; i != sceneCount; ++i) {
		getSceneThumb(i, true);
	}
	for (int i = 0; i != activeSceneSize; ++i) {
		getFrameThumb(i, true);
	}
	emit newActiveFrame(activeScene, activeFrame);
	fixSize();
}

void FrameBar::insertFrames(int index, int numFrames) {
	if (index <= activeFrame)
		activeFrame += numFrames;
	if (index <= selectionFrame)
		selectionFrame += numFrames;
	std::vector<ThumbView*>::iterator pos = thumbViews.begin()
			+ (index + activeScene + 1);
	thumbViews.insert(pos,
			static_cast<std::vector<ThumbView*>::size_type>(numFrames), 0);
	activeSceneSize += numFrames;
	// fix all frames after the insertion point
	for (int i = index; i != activeSceneSize; ++i) {
		getFrameThumb(i, true);
	}
	int sceneCount = sceneThumbCount();
	for (int i = activeScene + 1; i != sceneCount; ++i) {
		getSceneThumb(i, true);
	}
}

void FrameBar::addFrames(int index, int numFrames) {
	//TODO as we're no longer allowing cancelling during this operation, we
	// should implement caching and lazy loading so that this isn't a problem.
	Logger::get().logDebug("Adding frames in framebar");
	if (activeSceneSize < index) {
		resync();
		return;
	}
	insertFrames(index, numFrames);
	fixSize();
}

void FrameBar::deleteFrames(int fromFrame, int frameCount) {
	std::vector<ThumbView*>::iterator start
			= thumbViews.begin() + fromFrame + activeScene + 1;
	std::vector<ThumbView*>::iterator end = start + frameCount;
	for (std::vector<ThumbView*>::iterator i = start; i != end; ++i) {
		(*i)->delRef();
		(*i) = 0;
	}
	thumbViews.erase(start, end);
	activeSceneSize -= frameCount;
	// fix the selection (we want the thumbnails that are currently
	// highlighted to be in the selection)
	if (activeFrame < 0)
		return;
	int startSelection = selectionFrame;
	int endSelection = activeFrame + 1;
	if (endSelection < startSelection) {
		startSelection = activeFrame;
		endSelection = selectionFrame + 1;
	}
	if (endSelection <= fromFrame)
		return;
	if (endSelection < fromFrame + frameCount)
		endSelection = fromFrame;
	else
		endSelection -= frameCount;
	if (fromFrame < startSelection) {
		if (startSelection < fromFrame + frameCount)
			startSelection = fromFrame;
		else
			startSelection -= frameCount;
	}
	if (endSelection <= startSelection) {
		// no selection remains
		activeFrame = -1;
		selectionFrame = -1;
	} else if (activeFrame < selectionFrame) {
		activeFrame = startSelection;
		selectionFrame = endSelection - 1;
	} else {
		selectionFrame = startSelection;
		activeFrame = endSelection - 1;
	}
}

void FrameBar::removeFrames(int fromFrame, int toFrame) {
	if (activeSceneSize <= toFrame) {
		resync();
		return;
	}
	int frameCount = toFrame - fromFrame + 1;
	deleteFrames(fromFrame, frameCount);
	for (int i = fromFrame; i != activeSceneSize; ++i) {
		getFrameThumb(i, true);
	}
	int sceneCount = sceneThumbCount();
	for (int i = activeScene + 1; i != sceneCount; ++i) {
		getSceneThumb(i, true);
	}
	fixSize();
}

void FrameBar::moveFrames(int fromFrame, int toFrame, int movePosition) {
	int fromThumb = fromFrame + activeScene + 1;
	int toThumb = toFrame + activeScene + 2;
	int dest = movePosition + activeScene + 1;
	int updateStart = 0;
	int updateEnd = 0;
	if (movePosition < fromFrame) {
		std::rotate(thumbViews.begin() + dest,
				thumbViews.begin() + fromThumb,
				thumbViews.begin() + toThumb);
		updateStart = movePosition;
		updateEnd = toFrame + 1;
	} else if (toFrame < movePosition) {
		std::rotate(thumbViews.begin() + fromThumb,
				thumbViews.begin() + toThumb,
				thumbViews.begin() + dest);
		updateStart = fromFrame;
		updateEnd = movePosition;
	} else {
		return;
	}
	for (int i = updateStart; i != updateEnd; ++i) {
		getFrameThumb(i, true);
	}
}

void FrameBar::doScroll() {
	int thumbNumber = activeFrame < 0? activeScene
			: activeFrame + activeScene + 1;
	if (0 <= thumbNumber) {
		ensureVisible(thumbNumber * (FRAME_WIDTH + SPACE) + FRAME_WIDTH / 2,
				FRAME_HEIGHT / 2, FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
	}
}

void FrameBar::setActiveFrame(int frameNumber) {
	selecting = false;
	Logger::get().logDebug("Setting new active frame %d in FrameBar", activeFrame);
	setActiveFrameAndSelection(frameNumber, frameNumber);
	// If there is a frame to set as active
	if (frameNumber >= 0) {
		doScroll();
	}
}


void FrameBar::setSelecting(bool selecting) {
	this->selecting = selecting;
}


bool FrameBar::isSelecting() const {
	return selecting;
}

template<typename T> void swapContents(T& x, T& y) {
	T t = x;
	x = y;
	y = t;
}

void FrameBar::highlight(int start, int end, bool set) {
	int endFrame = getFrameThumbIndex(end);
	for (int i = getFrameThumbIndex(start); i != endFrame; ++i) {
		thumbViews[i]->setSelected(set);
	}
}

namespace {
struct SelectionEnd {
	// +1 to set, -1 to reset
	int toSet;
	int pos;
};
}

void FrameBar::changeSelectionHighlight(int af, int sf) {
	// put old selection in [a..b)
	SelectionEnd a = {-1, activeFrame};
	SelectionEnd b = {1, selectionFrame + 1};
	if (activeFrame == -1) {
		a.pos = 0;
		b.pos = 0;
	} else if (selectionFrame < activeFrame) {
		a.pos = selectionFrame;
		b.pos = activeFrame + 1;
	}
	activeFrame = af;
	selectionFrame = sf;
	// put new selection in [c..d)
	SelectionEnd c = {1, af};
	SelectionEnd d = {-1, sf + 1};
	if (af == -1) {
		c.pos = 0;
		d.pos = 0;
	} else if (sf < af) {
		c.pos = sf;
		d.pos = af + 1;
	}
	// put leftmost end in a
	if (c.pos < a.pos)
		swapContents(a, c);

	// put rightmost end in d
	if (d.pos < b.pos)
		swapContents(b, d);

	// put a,b,c,d in order
	if (c.pos < b.pos)
		swapContents(b, c);

	highlight(a.pos, b.pos, 0 < a.toSet);
	bool set = 0 < a.toSet + b.toSet + c.toSet;
	highlight(c.pos, d.pos, set);
}

void FrameBar::setActiveFrameAndSelection(int af, int sf) {
	if (activeSceneSize <= af || activeSceneSize <= sf) {
		resync();
		return;
	}
	if (af != activeFrame || sf != selectionFrame) {
		changeSelectionHighlight(af, sf);
		Logger::get().logDebug("Setting new active frame %d and scene %d in FrameBar",
				activeFrame, activeScene);
	}
	emit newActiveFrame(activeScene, activeFrame);
}

void FrameBar::setSelection(int sf) {
	setActiveFrameAndSelection(sf, selectionFrame);
}

int FrameBar::getSelectionAnchor() const {
	return selectionFrame;
}

void FrameBar::setPreferencesMenu(FramePreferencesMenu* preferencesMenu) {
	this->preferencesMenu = preferencesMenu;
}

void FrameBar::showPreferencesMenu() {
	preferencesMenu->open();
}

void FrameBar::updateNewScene(int index) {
	closeActiveScene();
	newScene(index);
	updateNewActiveFrame(index, -1);
}

void FrameBar::newScene(int index) {
	Logger::get().logDebug("Adding new scene thumb to framebar");

	int thumbIndex = index <= activeScene? index : index + activeSceneSize;
	thumbViews.insert(thumbViews.begin() + thumbIndex, 0);
	if (index <= activeScene) {
		++activeScene;
		for (int i = 0; i != activeSceneSize; ++i) {
			getFrameThumb(i, true);
		}
	}
	// get new scene and move and renumber all subsequent scenes
	int sceneCount = sceneThumbCount();
	for (int i = index; i != sceneCount; ++i) {
		getSceneThumb(i, true);
	}
	fixSize();
	emit modelSizeChanged(DomainFacade::getFacade()->getModelSize());
}

void FrameBar::updateRemoveScene(int sceneNumber) {
	removeScene(sceneNumber);
}

void FrameBar::removeScene(int sceneNumber) {
	if (sceneNumber < 0)
		return;
	if (sceneNumber == activeScene)
		closeActiveScene();
	std::vector<ThumbView*>::size_type delThumb = sceneNumber <= activeScene?
			sceneNumber + 1 : sceneNumber + activeSceneSize + 1;

	if (sceneNumber < activeScene)
		--activeScene;

	if (thumbViews.size() <= delThumb) {
		resync();
		return;
	}

	thumbViews[delThumb]->delRef();
	thumbViews.erase(thumbViews.begin() + delThumb);

	int sceneCount = sceneThumbCount();
	for (int i = sceneNumber + 1; i < sceneCount; ++i) {
		getSceneThumb(i, true);
	}
}

void FrameBar::updateMoveScene(int sceneNumber, int movePosition) {
	moveScene(sceneNumber, movePosition);
}

void FrameBar::moveScene(int sceneNumber, int movePosition) {
	int begin = sceneNumber;
	int mid = sceneNumber + 1;
	int end = movePosition;
	if (movePosition < sceneNumber) {
		begin = movePosition;
		end = sceneNumber + 1;
		mid = sceneNumber;
	} else if (end <= mid) {
		return;
	}
	int thumbBegin = activeScene < begin? begin + activeSceneSize : begin;
	int thumbMid = activeScene < mid? mid + activeSceneSize : mid;
	int thumbEnd = activeScene < end? end + activeSceneSize : end;
	if (static_cast<int>(thumbViews.size()) <= thumbEnd) {
		resync();
		return;
	}
	std::rotate(thumbViews.begin() + thumbBegin,
			thumbViews.begin() + thumbMid, thumbViews.begin() + thumbEnd);
	for (int i = begin; i != end; ++i) {
		getSceneThumb(i, true);
	}
	if (begin <= activeScene && activeScene < end) {
		for (int i = 0; i != activeSceneSize; i++) {
			getFrameThumb(i, true);
		}
	}
}

void FrameBar::closeActiveScene() {
	if (activeScene >= 0) {
		deleteFrames(0, activeSceneSize);
		activeSceneSize = 0;
		int s = activeScene;
		activeScene = -1;
		int sceneCount = sceneThumbCount();
		for (; s < sceneCount; ++s) {
			getSceneThumb(s, true);
		}
	}
}

void FrameBar::setActiveScene(int sceneNumber) {
	if (sceneNumber == activeScene)
		return;
	closeActiveScene();
	DomainFacade* anim = DomainFacade::getFacade();
	if (anim->getNumberOfScenes() <= sceneNumber) {
		return;
	}
	if (sceneNumber < 0) {
		activeScene = -1;
		activeSceneSize = 0;
	}

	activeScene = sceneNumber;
	activeFrame = -1;
	selectionFrame = -1;
	selecting = false;

	thumbViews[activeScene]->setOpened(true);
	int count = anim->getSceneSize(activeScene);
	insertFrames(0, count);
	fixSize();

	Logger::get().logDebug("Setting new active scene %d in FrameBar", activeScene);

	emit newActiveFrame(activeScene, activeFrame);

	doScroll();
	emit newMaximumValue(DomainFacade::getFacade()->getSceneSize(activeScene));
}


int FrameBar::getMovingScene() const {
	return movingScene;
}


void FrameBar::setMovingScene(int movingScene) {
	this->movingScene = movingScene;
}


// TODO: Check for other mime types as well
void FrameBar::dragEnterEvent(QDragEnterEvent *event) {
	if ( event->mimeData()->hasUrls() ) {
		event->accept();
	}
	else {
		event->ignore();
	}
}


void FrameBar::dropEvent(QDropEvent *event) {
	scrollTimer->stop();
	scrollDirection = 0;

	int index = (event->pos().x() + -mainWidget->pos().x()) / (FRAME_WIDTH + SPACE);
	if ( index < static_cast<int>(thumbViews.size() ) ) {
		thumbViews[index]->contentsDropped(event);
	}
}


void FrameBar::dragMoveEvent(QDragMoveEvent *event) {
	int dragPosX = event->pos().x();
	int dragPosY = event->pos().y();

	if (dragPosX < lowerScrollAreaX || dragPosX > upperScrollAreaX) {
		if ( !scrollTimer->isActive() ) {
			scrollDirection = dragPosX < lowerScrollAreaX ? -1 : 1;
			scrollTimer->start(50);
		}
		// Increase speed
		else if (dragPosX < lowerAccelScrollAreaX || dragPosX > upperAccelScrollAreaX ) {
			scrollTimer->setInterval(25);
		}
		// Decrease speed
		else {
			scrollTimer->setInterval(50);
		}
	}
	else {
		scrollTimer->stop();
		scrollDirection = 0;
	}

	// Stop scrolling if we're dragging outside framebar
	bool outsideHorizontal = dragPosX < minScrollAreaX || dragPosX > maxScrollAreaX;
	bool outsideVertical = dragPosY < minScrollAreaY || dragPosY > maxScrollAreaY;
	if (outsideHorizontal || outsideVertical) {
		scrollTimer->stop();
		scrollDirection = 0;
	}
}


void FrameBar::resizeEvent(QResizeEvent *event) {
	lowerScrollAreaX = this->x() + FRAME_WIDTH;
	upperScrollAreaX = this->width() - FRAME_WIDTH;

	lowerAccelScrollAreaX = lowerScrollAreaX - (FRAME_WIDTH >> 1);
	upperAccelScrollAreaX = upperScrollAreaX + (FRAME_WIDTH >> 1);

	minScrollAreaX = lowerScrollAreaX - FRAME_WIDTH + 20;
	maxScrollAreaX = upperScrollAreaX + FRAME_WIDTH - 20;

	minScrollAreaY = this->y() + 20;
	maxScrollAreaY = this->y() + FRAME_HEIGHT - 20;

	QScrollArea::resizeEvent(event);
}


void FrameBar::scroll() {
	if (scrollDirection == -1) {
		scrollBar->setSliderPosition(scrollBar->sliderPosition() - 15);
	}
	else if (scrollDirection == 1) {
		scrollBar->setSliderPosition(scrollBar->sliderPosition() + 15);
	}
}

int FrameBar::getFrameWidth() const {
	return FRAME_WIDTH;
}

int FrameBar::getFrameHeight() const {
	return FRAME_HEIGHT;
}

int FrameBar::getSpace() const {
	return SPACE;
}

int FrameBar::getActiveFrame() const {
	return activeFrame;
}

int FrameBar::getActiveScene() const {
	return activeScene;
}

void FrameBar::setThumbImage(ThumbView* thumb, const char* imagePath) {
	thumb->setPixmap(
			QPixmap::fromImage(
					tryReadImage(imagePath).scaled(FRAME_WIDTH, FRAME_HEIGHT)));
}

int FrameBar::getFrameThumbIndex(int index) {
	return activeScene + 1 + index;
}

ThumbView* FrameBar::getFrameThumb(int index, bool fix) {
	int thumbIndex = getFrameThumbIndex(index);
	ThumbView* thumb = thumbViews[thumbIndex];
	DomainFacade* facade = DomainFacade::getFacade();
	if (!thumb) {
		thumb = new FrameThumbView(this, 0, index);
		thumb->setMinimumSize(FRAME_WIDTH, FRAME_HEIGHT);
		thumb->setMaximumSize(FRAME_WIDTH, FRAME_HEIGHT);
		thumb->setScaledContents(true);
		const char* imagePath = facade->getImagePath(activeScene, index);
		if (imagePath) {
			setThumbImage(thumb, imagePath);
		}
		thumb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		thumb->setParent(mainWidget);
		thumb->show();
		thumbViews[thumbIndex] = thumb;
		fix = true;
	}
	if (fix) {
		thumb->move((FRAME_WIDTH + SPACE) * (index + activeScene + 1), 0);
		thumb->setNumber(index);
		thumb->setHasSounds(facade->getNumberOfSounds(activeScene, index));
		thumb->setSelected(0 <= activeFrame
				&& ((activeFrame <= index && index <= selectionFrame)
					|| (selectionFrame <= index && index <= activeFrame)));
	}
	return thumb;
}

ThumbView* FrameBar::getSceneThumb(int index, bool fix) {
	int thumbIndex = index <= activeScene? index : index + activeSceneSize;
	ThumbView* thumb = thumbViews[thumbIndex];
	if (!thumb) {
		thumb = new SceneThumbView(this, 0, index, "scene");
		thumb->setMinimumSize(FRAME_WIDTH, FRAME_HEIGHT);
		thumb->setMaximumSize(FRAME_WIDTH, FRAME_HEIGHT);
		thumb->setScaledContents(true);
		thumb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		thumb->setParent(mainWidget);
		thumb->show();
		thumbViews[thumbIndex] = thumb;
		fix = true;
	}
	if (fix) {
		thumb->move((FRAME_WIDTH + SPACE) * thumbIndex, 0);
		thumb->setNumber(index);
		thumb->setOpened(index == activeScene);
	}
	return thumb;
}

int FrameBar::sceneThumbCount() const {
	return thumbViews.size() - activeSceneSize;
}

void FrameBar::selectPreviousFrame() {
	if (activeFrame == -1) {
		if (0 < activeScene) {
			setActiveScene(activeScene - 1);
			int f = activeSceneSize - 1;
			setActiveFrameAndSelection(f, f);
		}
	} else {
		int f = activeFrame - 1;
		setActiveFrameAndSelection(f, f);
	}
	doScroll();
}

void FrameBar::selectNextFrame() {
	int f = activeFrame + 1;
	if (activeSceneSize <= f) {
		int s = activeScene + 1;
		if (s < sceneThumbCount()) {
			setActiveScene(s);
		}
	} else {
		setActiveFrameAndSelection(f, f);
	}
	doScroll();
}

void FrameBar::moveSelectionToPreviousFrame() {
	if (0 < activeFrame) {
		setSelection(activeFrame - 1);
	}
}

void FrameBar::moveSelectionToNextFrame() {
	if (activeFrame < activeSceneSize - 1) {
		setSelection(activeFrame + 1);
	}
}

void FrameBar::selectPreviousScene() {
	int s = activeScene - 1;
	if (0 <= s) {
		setActiveScene(s);
		int f = activeSceneSize - 1;
		setActiveFrameAndSelection(f, f);
	} else if (0 <= activeFrame) {
		setActiveFrameAndSelection(-1, -1);
	}
	doScroll();
}

void FrameBar::selectNextScene() {
	int s = activeScene + 1;
	if (s < sceneThumbCount()) {
		setActiveScene(s);
	} else {
		int f = activeSceneSize - 1;
		setActiveFrameAndSelection(f, f);
	}
	doScroll();
}

void FrameBar::setSelection(int scene, int selectionFrame, int activeFrame) {
	setActiveScene(scene);
	setActiveFrameAndSelection(activeFrame, selectionFrame);
}
