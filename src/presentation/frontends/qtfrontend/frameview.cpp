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

#include "frameview.h"

#include <algorithm>
#include <cstring>

#include <QApplication>
#include <QMessageBox>
#include <QPainter>

#include "imagecache.h"
#include "logger.h"
#include "frontends/frontend.h"
#include "workspacefile.h"
#include "technical/grabber/imagegrabber.h"
#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "src/presentation/frontends/qtfrontend/imagegrabthread.h"
#include "src/technical/grabber/commandlinegrabber.h"

enum { IMAGE_CACHE_SIZE = 10 };

namespace {
void getMaximumScaledRectangle(QRect& out, int w, int h, const QRect& window) {
	int vw = window.width();
	int vh = window.height();
	int dw = vw;
	int dh = vh;
	// Find out whether to scale by width or height.
	// Width if w/dw < h/dh <=> w*dh < h*dw
	if (w*dh < h*dw) {
		dw = (w * dh + h/2)/h;
		int left = (vw - dw)/2;
		out.setLeft(left);
		out.setRight(left + dw);
		out.setTop(window.top());
		out.setBottom(window.bottom());
	} else {
		dh = (h * dw + w/2)/w;
		int top = (vh - dh)/2;
		out.setTop(top);
		out.setBottom(top + dh);
		out.setLeft(window.left());
		out.setRight(window.right());
	}
}
void drawBorders(QPainter& painter, const QRect& inner, const QRect& outer) {
	int topBorder = inner.top() - outer.top();
	if (0 < topBorder) {
		painter.drawRect(outer.left(), outer.top(), outer.width(), topBorder);
	}
	int bottomBorder = outer.bottom() - inner.bottom();
	if (0 < bottomBorder) {
		painter.drawRect(outer.left(), inner.bottom(), outer.width(), bottomBorder);
	}
	int leftBorder = inner.left() - outer.left();
	if (0 < leftBorder) {
		painter.drawRect(outer.left(), inner.top(), leftBorder, inner.height());
	}
	int rightBorder = outer.right() - inner.right();
	if (0 < rightBorder) {
		painter.drawRect(inner.right(), inner.top(), rightBorder, inner.height());
	}
}
}

FrameView::FrameView(QWidget *parent, const char *name, int playbackSpeed)
		: QWidget(parent),
		  imageCache(IMAGE_CACHE_SIZE), grabThread(0), grabber(0),
		  capturedFile(WorkspaceFile::capturedImage) {
	facade = DomainFacade::getFacade();

	isPlayingVideo = false;
	mode = imageModeMix;
	this->playbackSpeed = PreferencesTool::get()->getPreference("fps",
			playbackSpeed);
	activeScene = -1;
	activeFrame = -1;
	mixCount = 2;
	playbackModeFrame = -1;

	connect(&grabTimer, SIGNAL(timeout()), this, SLOT(redraw()));
	connect(&playbackTimer, SIGNAL(timeout()),this, SLOT(nextPlayBack()));

	setMinimumSize(400, 300);
	setAttribute(Qt::WA_NoSystemBackground);
	setObjectName(name);
	update();

	Logger::get().logDebug("FrameView is attached to the model and the model to FrameView");
}


FrameView::~FrameView() {
	// Turn off camera if it's on
	if (isPlayingVideo) {
		off();
	}
	delete grabber;
	grabber = 0;
}


void FrameView::initCompleted() {
	emit cameraReady();
}

void FrameView::updateNewActiveFrame(int sceneNumber, int frameNumber) {
	setActiveFrame(sceneNumber, frameNumber);
}


void FrameView::updatePlayFrame(int sceneNumber, int frameNumber) {
	if (frameNumber > -1) {
		setActiveFrame(sceneNumber, frameNumber);
	} else {
		this->update();
	}
}


void FrameView::workspaceCleared() {
	imageCache.clear();
}


void FrameView::resizeEvent(QResizeEvent*) {
	update();
}

void FrameView::drawOnionSkins() {
	int w = cameraOutput.width();
	int h = cameraOutput.height();
	QRect dst;
	getMaximumScaledRectangle(dst, w, h, rect());
	QPainter painter(this);
	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBrush(QColor::fromRgb(0, 0, 0, 255));
	drawBorders(painter, dst, rect());
	DomainFacade* anim = DomainFacade::getFacade();
	if (isPlayingVideo && 0 <= activeScene) {
		switch (mode) {
		case imageModeMix:
		{
			painter.drawPixmap(dst, cameraOutput);
			const int frameCount = std::min(mixCount, activeFrame + 1);
			for (int i = 0; i != frameCount; ++i) {
				const char* path = anim->getImagePath(
						activeScene, activeFrame - i);
				QPixmap* image = imageCache.get(path);
				if (image) {
					painter.setOpacity(qreal(1)/(i + 2));
					painter.drawPixmap(dst, *image);
				}
			}
			break;
		}
		case imageModeDiff:
		{
			QSize differenceSize(cameraOutput.size());
			QRect differenceRect(QPoint(0, 0), differenceSize);
			QImage difference(differenceSize, QImage::Format_RGB888);
			QPainter differencePainter(&difference);
			differencePainter.drawPixmap(differenceRect, cameraOutput);
			if (0 <= activeFrame) {
				const char* path = anim->getImagePath(activeScene, activeFrame);
				QPixmap* image = imageCache.get(path);
				if (image) {
					differencePainter.setCompositionMode(
							QPainter::CompositionMode_Difference);
					differencePainter.drawPixmap(differenceRect, *image);
				}
			}
			differencePainter.end();
			painter.drawImage(dst, difference);
			break;
		}
		case imageModePlayback:
			if (playbackModeFrame < 0) {
				painter.drawPixmap(dst, cameraOutput);
			} else {
				const char* path = anim->getImagePath(activeScene, playbackModeFrame);
				QPixmap* image = imageCache.get(path);
				if (image) {
					painter.drawPixmap(dst, *image);
				}
			}
			break;
		default:
			painter.drawPixmap(dst, cameraOutput);
			break;
		}
	} else {
		painter.drawPixmap(dst, cameraOutput);
	}
}

void FrameView::paintEvent(QPaintEvent *) {
	if (isPlayingVideo) {
		if (!cameraOutput.isNull()) {
			drawOnionSkins();
		}
	} else {
		QPainter painter(this);
		painter.setBackgroundMode(Qt::OpaqueMode);
		painter.setBrush(QColor::fromRgb(0, 0, 0, 255));
		DomainFacade* anim = DomainFacade::getFacade();
		if (0 <= activeScene && 0 <= activeFrame) {
			const char* path = anim->getImagePath(activeScene, activeFrame);
			QPixmap* image = imageCache.get(path);
			if (image) {
				QRect destination;
				getMaximumScaledRectangle(destination, image->width(), image->height(), rect());
				drawBorders(painter, destination, rect());
				painter.drawPixmap(destination, *image);
				return;
			}
		}
		painter.drawRect(rect());
	}
}

void FrameView::setActiveFrame(int sceneNumber, int frameNumber) {
	activeScene = sceneNumber;
	activeFrame = frameNumber;
	Logger::get().logDebug("Setting new active frame %d and scene %d in FrameView",
			activeFrame, activeScene);
	update();
}


// TODO: Refactor this terrible ugly method. This one is really bad!!
bool FrameView::on() {
	const char* DEVICE_TOKEN = "$VIDEODEVICE";
	PreferencesTool *prefs = PreferencesTool::get();
	int activeCmd = prefs->getPreference("activedevice", 0);

	Preference prepoll(QString("importprepoll%1")
			.arg(activeCmd).toLatin1().constData(), "");
	Preference startDaemon(QString("importstartdaemon%1")
			.arg(activeCmd).toLatin1().constData(), "");
	Preference stopDaemon(QString("importstopdaemon%1")
			.arg(activeCmd).toLatin1().constData(), "");

	bool prepollRequiresDevice = strstr(prepoll.get(), DEVICE_TOKEN);
	bool startDaemonRequiresDevice = strstr(startDaemon.get(), DEVICE_TOKEN);
	QString device;
	if (prepollRequiresDevice || startDaemonRequiresDevice) {
		int activeDev = prefs->getPreference("activeVideoDevice", -1);
		if (0 <= activeDev) {
			Preference deviceP(QString("device%1")
					.arg(activeDev).toLatin1().constData(), "");
			device = QString(deviceP.get()).trimmed();
		}
		if (device.isEmpty()) {
			QMessageBox::warning(this, tr("Warning"), tr(
				"No video device selected in the preferences menu."),
				QMessageBox::Ok,
				Qt::NoButton,
				Qt::NoButton);
			return false;
		}
	}
	QString pre = QString(prepoll.get()).replace(DEVICE_TOKEN, device);
	bool isProcess = startDaemon.get() && *startDaemon.get() != '\0';

	bool isCameraReady = true;
	grabber = new CommandLineGrabber(capturedFile.path());
	if ( !grabber->setPrePollCommand(pre.toLatin1().constData()) ) {
		QMessageBox::warning(this, tr("Warning"), tr(
					"Pre poll command does not exists"),
					QMessageBox::Ok,
					Qt::NoButton,
					Qt::NoButton);
		//return false;
		isCameraReady = false;
	}

	if (isProcess) {
		QString sd = QString(startDaemon.get()).replace(DEVICE_TOKEN, device);
		if ( !grabber->setStartCommand(sd.toLatin1().constData()) ) {
			DomainFacade::getFacade()->getFrontend()->hideProgress();
			QMessageBox::warning(this, tr("Warning"), tr(
						"You do not have the given grabber installed on your system"),
						QMessageBox::Ok,
						Qt::NoButton,
						Qt::NoButton);
			isCameraReady = false;
			//return false;
		}
	}

	grabber->setStopCommand(stopDaemon.get());

	if (!isCameraReady) {
		return false;
	}
	initCompleted();
	isPlayingVideo = true;

	if ( prefs->getPreference("numberofimports", 1) > 0 ) {
		// If the grabber is running in it's own process we use a timer.
		if (grabber->isGrabberProcess() == true) {
			if ( grabber->init() ) {
				grabTimer.start(150);
			}
			else {
				QMessageBox::warning(this, tr("Warning"), tr(
					"Grabbing failed. This may happen if you try\n"
					"to grab from an invalid device. Please check\n"
					"your grabber settings in the preferences menu."),
					QMessageBox::Ok,
					Qt::NoButton,
					Qt::NoButton);
				return false;
			}
		}
		// Otherwise a thread is needed
		else {
			grabThread = new ImageGrabThread(this, grabber);
			connect(grabThread, SIGNAL(grabbed()), this, SLOT(redraw()));
			grabThread->start();
			grabThread->wait(500);

			if (grabThread->wasGrabbingSuccess() == false) {
				QMessageBox::warning(this, tr("Warning"), tr(
					"Grabbing failed. This may happen if you try\n"
					"to grab from an invalid device. Please check\n"
					"your grabber settings in the preferences menu."),
					QMessageBox::Ok,
					Qt::NoButton,
					Qt::NoButton);
				return false;
			}
		}
	}
	else {
		QMessageBox::warning(this, tr("Warning"), tr(
				"You have to define an image grabber to use.\n"
				"This can be set in the preferences menu."),
				QMessageBox::Ok,
				Qt::NoButton,
				Qt::NoButton);
		return false;
	}
	return true;
}


void FrameView::off() {
	if ( grabber != 0 ) {
		if ( grabber->isGrabberProcess() ) {
			grabber->tearDown();
			grabTimer.stop();
			playbackTimer.stop();
		}
	}
	if (grabThread != 0) {
		grabThread->terminate();
		grabThread->wait();
		delete grabThread;
		grabThread = 0;
	}
	delete grabber;
	grabber = 0;

	isPlayingVideo = false;
	setActiveFrame(activeScene, activeFrame);
	update();
}


void FrameView::redraw() {
	cameraOutput.load(capturedFile.path());
	update();
}


void FrameView::nextPlayBack() {
	if (0 <= activeScene && 0 <= activeFrame) {
		int firstFrame = std::max(activeFrame - mixCount + 1, 0);
		if (playbackModeFrame < firstFrame) {
			// restart playback
			playbackModeFrame = firstFrame;
		} else {
			++playbackModeFrame;
		}
		if (activeFrame < playbackModeFrame) {
			// negative number means that the camera output should be shown
			playbackModeFrame = -1;
		}
		update();
		return;
	}
	// Set display image to camera output
	playbackModeFrame = -1;
	update();
}


bool FrameView::setViewMode(ImageMode mode) {
	if (mode == this->mode)
		return true;
	if (!grabber)
		return true;
	if (mode == imageModePlayback) {
		if ( grabber->isGrabberProcess() ) {
			playbackTimer.start(1000/playbackSpeed);
		} else {
			return false;
		}
	} else if (mode != imageModePlayback) {
		if ( grabber->isGrabberProcess() ) {
			playbackTimer.stop();
		}
	}
	this->mode = mode;
	return true;
}

void FrameView::setMixCount(int mixCount) {
	this->mixCount = mixCount;
}

int FrameView::getViewMode() const {
	return mode;
}

void FrameView::setPlaybackSpeed(int playbackSpeed) {
	this->playbackSpeed = playbackSpeed;
	if ( playbackTimer.isActive() ) {
		playbackTimer.setInterval(1000 / playbackSpeed);
	}
}

int FrameView::getPlaybackSpeed() const {
	return playbackSpeed;
}

void FrameView::fileChanged(const QString& path) {
	const char* p = path.toLocal8Bit();
	imageCache.drop(p);
	if (isPlayingVideo && 0 <= activeScene) {
		update();
	}
}
