/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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

// This widget is created from a widget in an example program for Qt. Available at
// http://www.libsdl.org/cvs/qtSDL.tar.gz

#include "src/foundation/preferencestool.h"
#include "src/technical/grabber/commandlinegrabber.h"
#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/imagegrabthread.h"
#include "src/presentation/frontends/qtfrontend/qtfrontend.h"

#include <SDL/SDL_image.h>
#include <QX11Info>
#include <QMessageBox>
#include <QApplication>
#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#endif

#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include <sstream>

enum { IMAGE_CACHE_SIZE = 10 };

FrameView::FrameView(QWidget *parent, const char *name, int playbackSpeed)
		: QWidget(parent), screen(0), videoSurface(0),
		  imageCache(IMAGE_CACHE_SIZE), grabThread(0), grabber(0),
		  capturedFile(WorkspaceFile::capturedImage) {
	facade = DomainFacade::getFacade();

	isPlayingVideo = false;
	widthConst  = 4;
	heightConst = 3;
	mode = imageModeMix;
	this->playbackSpeed = PreferencesTool::get()->getPreference("fps",
			playbackSpeed);
	activeScene = -1;
	activeFrame = -1;
	mixCount = 2;

	connect(&grabTimer, SIGNAL(timeout()), this, SLOT(redraw()));
	connect(&playbackTimer, SIGNAL(timeout()),this, SLOT(nextPlayBack()));

	setNormalRatio();
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setObjectName(name);

	Logger::get().logDebug("FrameView is attatched to the model and the model to FrameView");
}


FrameView::~FrameView() {
	// Turn off camera if it's on
	if (isPlayingVideo) {
		off();
	}

	if (videoSurface) {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
	}

	// Freeing resources allocated to SDL and shutdown
	SDL_Quit();

	delete grabber;
	grabber = 0;
}


void FrameView::setWidescreenRatio() {
	widthConst  = 16;
	heightConst = 9;
	float minWidth = 300 * (16 / 9);
	this->setMinimumSize( (int)minWidth, 300);
	this->update();
}


void FrameView::setNormalRatio() {
	widthConst  = 4;
	heightConst = 3;
	this->setMinimumSize(400, 300);
	this->update();
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
	}
	else {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
		this->update();
	}
}


void FrameView::workspaceCleared() {
	imageCache.clear();
}


void FrameView::resizeEvent(QResizeEvent*) {
	QApplication::syncX();

	// Set the new video mode with the new window size
	std::stringstream windowId;
	windowId << "0x" << std::hex << static_cast<long>(winId());
	std::string wid = windowId.str();
	setenv("SDL_WINDOWID", wid.c_str(), 1);
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		std::string msg("Unable to initialize SDL: ");
		msg.append( SDL_GetError() );
		Logger::get().logFatal(msg.c_str());
    }

	// The previously surface is automatically deleted by SDL
	screen = SDL_SetVideoMode(width(), height(), 0, SDL_DOUBLEBUF | SDL_SWSURFACE);
	if (!screen) {
		std::string msg("Unable to set video mode: ");
		msg.append( SDL_GetError() );
		Logger::get().logFatal(msg.c_str());
	}
}

void FrameView::drawOnionSkins() {
	static SDL_Surface *frameSurface = 0;
	DomainFacade* anim = DomainFacade::getFacade();
	SDL_Rect dst;
	dst.x = (screen->w - videoSurface->w) >> 1;
	dst.y = (screen->h - videoSurface->h) >> 1;
	dst.w = videoSurface->w;
	dst.h = videoSurface->h;
	SDL_BlitSurface(videoSurface, 0, screen, &dst);
	if (isPlayingVideo && 0 <= activeScene) {
		switch (mode) {
		case 0:
			// Image mixing
			for (int i = 0; i != std::min(mixCount, activeFrame + 1); ++i) {
				const char* path = anim->getImagePath(
						activeScene, activeFrame - i);
				frameSurface = imageCache.get(path);
				if (frameSurface != 0) {
					SDL_Rect dst2;
					dst2.x = (screen->w - frameSurface->w) >> 1;
					dst2.y = (screen->h - frameSurface->h) >> 1;
					dst2.w = frameSurface->w;
					dst2.h = frameSurface->h;
					int alpha = 256 / (i + 2);
					SDL_SetAlpha(frameSurface, SDL_SRCALPHA, alpha);
					SDL_BlitSurface(frameSurface, 0, screen, &dst2);
				}
			}
			break;
		case 1:
			// Image differentiating
			if (0 <= activeFrame) {
				const char* path = anim->getImagePath(activeScene, activeFrame);
				SDL_Surface *last = imageCache.get(path);
				SDL_Surface *tmp = differentiateSurfaces(videoSurface, last);
				SDL_Rect dst;
				dst.x = (screen->w - tmp->w) >> 1;
				dst.y = (screen->h - tmp->h) >> 1;
				dst.w = tmp->w;
				dst.h = tmp->h;
				SDL_BlitSurface(tmp, 0, screen, &dst);
				SDL_FreeSurface(tmp);
			}
			break;
		default:
			break;
		}
	}
}

void FrameView::paintEvent(QPaintEvent *) {
#if defined(Q_WS_X11)
	// Make sure we're not conflicting with drawing from the Qt library
	XSync(QX11Info::display(), FALSE);
#endif
	if (screen) {
		SDL_FillRect(screen, 0, 0);

		if (videoSurface) {
			drawOnionSkins();
		}
		SDL_Flip(screen);
	}
}


void FrameView::setActiveFrame(int sceneNumber, int frameNumber) {
	activeScene = sceneNumber;
	activeFrame = frameNumber;
	Logger::get().logDebug("Setting new active frame in FrameView");
	const char *fileName = 0 <= sceneNumber && 0 <=frameNumber?
			facade->getImagePath(sceneNumber, frameNumber) : 0;

	if (videoSurface) {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
	}
	if (fileName) {
		Logger::get().logDebug("Loading image");
		videoSurface = IMG_Load(fileName);
		if (videoSurface == 0) {
			printf("IMG_Load: %s\n", IMG_GetError());
		}
		Logger::get().logDebug("Loading image finished");
	} else {
		Logger::get().logDebug("Failed to get image path from animation");
	}
	this->update();
}


// TODO: Refactor this terrible ugly method. This one is really bad!!
bool FrameView::on() {
	PreferencesTool *prefs = PreferencesTool::get();
	int activeCmd = prefs->getPreference("activedevice", 0);

	Preference prepoll(QString("importprepoll%1")
			.arg(activeCmd).toLatin1().constData(), "");
	Preference startDaemon(QString("importstartdaemon%1")
			.arg(activeCmd).toLatin1().constData());
	Preference stopDaemon(QString("importstopdaemon%1")
			.arg(activeCmd).toLatin1().constData(), "");

	int activeDev = prefs->getPreference("activeVideoDevice", -1);
	if (activeDev <0) {
		QMessageBox::warning(this, tr("Warning"), tr(
			"No video device selected in the preferences menu."),
			QMessageBox::Ok,
			Qt::NoButton,
			Qt::NoButton);
		return false;
	}
	Preference device(QString("device%1")
			.arg(activeDev).toLatin1().constData(), "");
	QString pre = QString(prepoll.get()).replace("$VIDEODEVICE", device.get());
	bool isProcess = startDaemon.get() && *startDaemon.get() != '\0';

	bool isCameraReady = true;
	this->grabber = new CommandLineGrabber(capturedFile.path());
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
		QString sd = QString(startDaemon.get()).replace("$VIDEODEVICE", device.get());
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
	this->initCompleted();
	this->isPlayingVideo = true;

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
		delete grabber;
		grabber = 0;
	}

	if (grabThread != 0) {
		grabThread->terminate();
		grabThread->wait();
		delete grabThread;
		grabThread = 0;
	}

	this->isPlayingVideo = false;
	setActiveFrame(activeScene, activeFrame);
	this->update();
}


void FrameView::redraw() {
	if (videoSurface) {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
	}
	videoSurface = IMG_Load(capturedFile.path());
	this->update();
}


void FrameView::nextPlayBack() {
	//TODO re-write this vile function
	static int i = 0;

	// Need to check that there is an active scene before checking
	// what its size is.

	if (0 <= activeScene) {
		if (i < mixCount && i < activeFrame + 1) {
			const char *path = activeFrame <= mixCount?
				facade->getImagePath(activeScene, i)
				: facade->getImagePath(activeScene,
						activeFrame - mixCount + i);
			++i;

			if (videoSurface) {
				SDL_FreeSurface(videoSurface);
				videoSurface = 0;
			}
			if (path)
				videoSurface = IMG_Load(path);

			this->update();
			//Exit from function/skip redraw(). This is better than having a bool which is
			//set because this is a play function run "often".
			return;
		}
	}

	// This code is run if one of the two above tests fail. Can't be an else because
	// then I would have to have two such elses, and I think the return is better.
	i = 0;
	redraw();
}


bool FrameView::setViewMode(ImageMode mode) {
	if (mode == this->mode)
		return true;
	if (mode == imageModePlayback) {
		if ( grabber->isGrabberProcess() ) {
			grabTimer.stop();
			playbackTimer.start(1000/playbackSpeed);
		} else {
			return false;
		}
	} else if (mode != imageModePlayback) {
		if ( grabber->isGrabberProcess() ) {
			playbackTimer.stop();
			grabTimer.start(150);
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


// The only thing left which is a little expensive is the MapRGB function.
SDL_Surface* FrameView::differentiateSurfaces(SDL_Surface *s1, SDL_Surface *s2) {
	int width = s2->w;
	int height = s2->h;

	SDL_Surface *diffSurface = SDL_CreateRGBSurface(
			SDL_SWSURFACE, width, height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	// Lock the surfaces before working with the pixels
	SDL_LockSurface(s1);
	SDL_LockSurface(s2);
	SDL_LockSurface(diffSurface);

	// Pointers to the first byte of the first pixel on the input surfaces.
	Uint8 *p1 = static_cast<Uint8 *>(s1->pixels);
	Uint8 *p2 = static_cast<Uint8 *>(s2->pixels);

	// Pointers to the first pixel on the resulting surface
	Uint32 *pDiff =  static_cast<Uint32 *>(diffSurface->pixels);

	SDL_PixelFormat fDiff = *diffSurface->format;
	Uint32  differencePixel;
	Uint8 dr, dg, db;

	// Goes through the surfaces as one-dimensional arrays.
	int offset = 0, pixelOffset = 0;
	for (int i = 0; i < height; ++i) {
		for (int j = 1; j < width; ++j) {
			dr = abs(p1[offset    ] - p2[offset    ]);
			dg = abs(p1[offset + 1] - p2[offset + 1]);
			db = abs(p1[offset + 2] - p2[offset + 2]);
			differencePixel = SDL_MapRGB(&fDiff, dr, dg, db);
			pDiff[pixelOffset++] = differencePixel;
			offset += 3;
		}
		++pixelOffset;
		offset += 3;
	}

	// Unlock the surfaces for displaying them.
	SDL_UnlockSurface(s1);
	SDL_UnlockSurface(s2);
	SDL_UnlockSurface(diffSurface);

	return diffSurface;
}

void FrameView::fileChanged(const QString& path) {
	const char* p = path.toLocal8Bit();
	imageCache.drop(p);
	if (isPlayingVideo && 0 <= activeScene) {
		drawOnionSkins();
	}
}
