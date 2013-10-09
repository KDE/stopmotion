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

// This widget is created from a widget in an example program for Qt. Available at
// http://www.libsdl.org/cvs/qtSDL.tar.gz

#include "src/presentation/frontends/qtfrontend/frameview.h"
#include "src/technical/grabber/commandlinegrabber.h"
#include "src/foundation/preferencestool.h"

#include <QSize>
#include <QX11Info>
#include <QResizeEvent>
#include <QMessageBox>
#include <QPaintEvent>
#include <QApplication>
#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#endif

#include <cstdio>
#include <stdlib.h>
#include <cstring>

	
const int FrameView::alphaLut[5] = { 128, 64, 43, 32, 26 };


FrameView::FrameView(QWidget *parent, const char *name, int playbackSpeed)
		: QWidget(parent), screen(0), videoSurface(0), grabThread(0),
		  grabber(0), activeScene(0)
{
	char tmp[256];
	snprintf(tmp, 256, "%s/.stopmotion/capturedfile.jpg", getenv("HOME"));
	capturedImg = new char[strlen(tmp) + 1];
	strcpy(capturedImg, tmp);
	
	facade = DomainFacade::getFacade();
	
	isPlayingVideo = false;
	widthConst  = 4;
	heightConst = 3;
	mode = 0;
	this->playbackSpeed = PreferencesTool::get()->getPreference("fps", playbackSpeed);
	activeFrame = 0;
	mixCount = 2;
	lastMixCount = 2;
	lastViewMode = 0;
	numImagesInBuffer = 0;
	
	connect(&grabTimer, SIGNAL(timeout()), this, SLOT(redraw()));
	connect(&playbackTimer, SIGNAL(timeout()),this, SLOT(nextPlayBack()));
	
	setNormalRatio();
	setAttribute(Qt::WA_PaintOnScreen);	
	setAttribute(Qt::WA_NoSystemBackground);
	setObjectName(name);
	
	facade->attatch(this);
	Logger::get().logDebug("FrameView is attatched to the model and the model to FrameView");
}


FrameView::~FrameView()
{
	// Turn off camera if it's on
	if (isPlayingVideo) {
		off();
	}

	if (videoSurface) {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
	}

	for (unsigned int i = 0; i < imageBuffer.size(); ++i) {
		SDL_FreeSurface(imageBuffer[i]);
	}
	imageBuffer.clear();
	numImagesInBuffer = 0;
	
	// Freeing resources allocated to SDL and shutdown
	SDL_Quit();
	
	delete grabber;
	grabber = 0;
	
	delete [] capturedImg;
	capturedImg = 0;
}


void FrameView::setWidescreenRatio()
{
	widthConst  = 16;
	heightConst = 9;
	float minWidth = 300 * (16 / 9);
	this->setMinimumSize( (int)minWidth, 300);
	this->update();
}


void FrameView::setNormalRatio()
{
	widthConst  = 4;
	heightConst = 3;
	this->setMinimumSize(400, 300);
	this->update();
}


void FrameView::initCompleted()
{
	emit cameraReady();
}


void FrameView::updateAdd(FrameIterator& frames, int, int, Frontend*) {
	if (isPlayingVideo && !frames.isAtEnd()) {
		addToImageBuffer( IMG_Load(frames.getName()) );
	}
}


void FrameView::updateRemove(unsigned int, unsigned int) {}
void FrameView::updateMove(unsigned int, unsigned int, unsigned int) {}


void FrameView::updateNewActiveFrame(int frameNumber)
{
	if (frameNumber > -1) {
		setActiveFrame(frameNumber);
	}
	else {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
		this->update();
	}	
}


void FrameView::updatePlayFrame(int frameNumber)
{
	if (frameNumber > -1) {
		setActiveFrame(frameNumber);
	}
	else {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
		this->update();
	}
}


void FrameView::resizeEvent(QResizeEvent*)
{	
	QApplication::syncX();

	// Set the new video mode with the new window size
	char variable[64];
	sprintf(variable, "SDL_WINDOWID=0x%lx", winId());
	putenv(variable);
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		char msg[80] = "Unable to init SDL: ";
		strcat( msg, SDL_GetError() );
		Logger::get().logFatal(msg);
    }

	// The previously surface is automatically deleted by SDL
	screen = SDL_SetVideoMode(width(), height(), 0, SDL_DOUBLEBUF | SDL_SWSURFACE);
	if (!screen) {
		char msg[80] = "Unable to set vide mode: ";
		strcat( msg, SDL_GetError() );
		Logger::get().logFatal(msg);
	}
}


void FrameView::paintEvent(QPaintEvent *)
{
	static SDL_Surface *frameSurface = 0;
	int activeFrame = -1;
	
#if defined(Q_WS_X11)
	// Make sure we're not conflicting with drawing from the Qt library
	XSync(QX11Info::display(), FALSE);
#endif
	if (screen) {
		SDL_FillRect(screen, 0, 0);
		
		if (videoSurface) {
			SDL_Rect dst;
			dst.x = (screen->w - videoSurface->w) >> 1;
			dst.y = (screen->h - videoSurface->h) >> 1;
			dst.w = videoSurface->w;
			dst.h = videoSurface->h;
			SDL_BlitSurface(videoSurface, 0, screen, &dst);
			
			if (isPlayingVideo) {
				activeFrame = facade->getActiveFrameNumber();
				switch (mode) {
					// Image mixing
					case 0:
					{
						if (activeFrame != this->activeFrame || lastMixCount != mixCount || 
								lastViewMode != mode) {
							this->activeFrame = activeFrame;
							lastMixCount = mixCount;
							lastViewMode = mode;
						}
						
						int offset = numImagesInBuffer - 1;
						for (int i = 0; i < numImagesInBuffer && i < mixCount; ++i) {
							frameSurface = imageBuffer[offset - i];
							if (frameSurface != 0) {
								SDL_Rect dst2;
								dst2.x = (screen->w - frameSurface->w) >> 1;
								dst2.y = (screen->h - frameSurface->h) >> 1;
								dst2.w = frameSurface->w;
								dst2.h = frameSurface->h;
								SDL_SetAlpha(frameSurface, SDL_SRCALPHA, alphaLut[i]);
								SDL_BlitSurface(frameSurface, 0 , screen, &dst2);
							}
						}
						break;
					}
					// Image differentiating
					case 1: 
					{
						if (activeFrame != this->activeFrame || lastViewMode != mode) {
							this->activeFrame = activeFrame;
							lastViewMode = mode;	
						}
							
						if (activeFrame >= 0) {
							SDL_Surface *tmp = differentiateSurfaces(videoSurface, imageBuffer.back());
							SDL_Rect dst;
							dst.x = (screen->w - tmp->w) >> 1;
							dst.y = (screen->h - tmp->h) >> 1;
							dst.w = tmp->w;
							dst.h = tmp->h;
							SDL_BlitSurface(tmp, 0, screen, &dst);
							SDL_FreeSurface(tmp);
						}
						break;
					}
				}
			}
		}
		SDL_Flip(screen);
	}
}


void FrameView::setActiveFrame(int frameNumber)
{
	Logger::get().logDebug("Setting new active frame in FrameView");
	const Frame *frame = facade->getFrame(frameNumber);
	if (frame) {
		const char *fileName = frame->getImagePath();

		if (videoSurface) {
			SDL_FreeSurface(videoSurface);
			videoSurface = 0;
		}
		Logger::get().logDebug("Loading image");

		videoSurface = IMG_Load(fileName);
		if (videoSurface == 0) {
			printf("IMG_Load: %s\n", IMG_GetError());
		}
		Logger::get().logDebug("Loading image finished");
		this->update();
	}
	else {
		Logger::get().logWarning("Active frame number inside frameview is not valid!");
	}
}


void FrameView::addToImageBuffer(SDL_Surface *const image)
{
	if (numImagesInBuffer == 5) {
		SDL_FreeSurface( imageBuffer.front() );
		imageBuffer.pop_front();
	}
	else {
		++numImagesInBuffer;
	}
	imageBuffer.push_back(image);
}


void FrameView::updateClear()
{
	SDL_FreeSurface(videoSurface);
	videoSurface = 0;
	this->update();
}


void FrameView::updateNewScene(int) {}
void FrameView::updateRemoveScene(int) {}
void FrameView::updateNewActiveScene(int scene, FrameIterator&, Frontend*) {
	activeScene = scene;
}


void FrameView::updateAnimationChanged(int sceneNumber, int frameNumber) {
	if (sceneNumber == activeScene)
		setActiveFrame(frameNumber);
}


void FrameView::updateMoveScene(int, int) {}


// TODO: Refactor this terrible ugly method. This one is really bad!!
bool FrameView::on()
{
	PreferencesTool *prefs = PreferencesTool::get();
	int activeCmd = prefs->getPreference("activedevice", 0);
	
	const char* prepoll = 
		prefs->getPreference(QString("importprepoll%1").arg(activeCmd).toLatin1().constData(), "");
	const char* startDeamon = 
		prefs->getPreference(QString("importstartdeamon%1").arg(activeCmd).toLatin1().constData(), "");
	const char* stopDeamon = 
		prefs->getPreference(QString("importstopdeamon%1").arg(activeCmd).toLatin1().constData(), "");
	
	int activeDev = prefs->getPreference("activeVideoDevice", -1);
	if (activeDev > -1) {
		const char *device = 
			prefs->getPreference(QString("device%1").arg(activeDev).toLatin1().constData(), "");
		QString pre = QString(prepoll).replace("$VIDEODEVICE", device);
		freeProperty(prepoll);
		prepoll = strdup(pre.toLatin1().constData());
		QString sd = QString(startDeamon).replace("$VIDEODEVICE", device);
		freeProperty(startDeamon);
		startDeamon = strdup(sd.toLatin1().constData());
		freeProperty(device);
	}
	else {
		QMessageBox::warning(this, tr("Warning"), tr(
			"No video device selected in the preferences menu."),
			QMessageBox::Ok,
			Qt::NoButton, 
			Qt::NoButton);
		return false;
	}

	bool isProcess = (strcmp(startDeamon, "") == 0) ? false : true;
	bool isCameraReady = true;
	this->grabber = new CommandLineGrabber(capturedImg, isProcess);
	if ( !grabber->setPrePollCommand(prepoll) ) {
		QMessageBox::warning(this, tr("Warning"), tr(
					"Pre poll command does not exists"),
					QMessageBox::Ok,
					Qt::NoButton, 
					Qt::NoButton);
		//return false;
		isCameraReady = false;
	}

	free(const_cast<char *>(prepoll));
	prepoll = 0;

	if ( !grabber->setStartCommand(startDeamon) ) {
		DomainFacade::getFacade()->getFrontend()->hideProgress();
		QMessageBox::warning(this, tr("Warning"), tr(
					"You do not have the given grabber installed on your system"),
					QMessageBox::Ok,
					Qt::NoButton, 
					Qt::NoButton);
		isCameraReady = false;
		//return false;
	}
	
	free(const_cast<char *>(startDeamon));
	startDeamon = 0;

	grabber->setStopCommand(stopDeamon);
	freeProperty(stopDeamon);

	if (isCameraReady) {
		this->initCompleted();
		this->isPlayingVideo = true;
	}
	else {
		return false;
	}
	
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
	
	// To get the draw function buffer for onionskinning
	lastMixCount = -1;
	return true;
}


void FrameView::off()
{
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
	
	for (unsigned int i = 0; i < imageBuffer.size(); ++i) {
		SDL_FreeSurface(imageBuffer[i]);
	}
	imageBuffer.clear();
	numImagesInBuffer = 0;
		
	this->isPlayingVideo = false;

	if (videoSurface != 0) {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
	}
	this->update();
}


void FrameView::redraw()
{
	if (videoSurface) {
		SDL_FreeSurface(videoSurface);
		videoSurface = 0;
	}
	videoSurface = IMG_Load(capturedImg);
	this->update();
}


void FrameView::nextPlayBack()
{
	static unsigned int i = 0;
	
	// Need to check that there is an active scene before checking
	// what its size is.

	if (facade->getActiveSceneNumber() >= 0) {
		if ((int)i < (mixCount) && i < (unsigned int)facade->getActiveFrameNumber() + 1) {
			
			const Frame *frame = 0;
			if (facade->getActiveFrameNumber() <= (mixCount)) {
				frame = facade->getFrame(i++);
			}
			else {
				frame = facade->getFrame(facade->getActiveFrameNumber() - mixCount + i++);
			}
				
			if (frame) {
				if (videoSurface) {
					SDL_FreeSurface(videoSurface);
					videoSurface = 0;
				}
				videoSurface = IMG_Load(frame->getImagePath());
			}
				
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


bool FrameView::setViewMode(int mode)
{
	// Going into playback mode.
	if (mode == 2 && this->mode != 2) {
		if ( grabber->isGrabberProcess() ) {
			grabTimer.stop();
			playbackTimer.start(1000/playbackSpeed);
		}
		else {
			return false;
		}
	}
	// Going out of playback mode.
	else if (mode != 2 && this->mode == 2) {
		if ( grabber->isGrabberProcess() ) {
			playbackTimer.stop();
			grabTimer.start(150);
		}
	}
	
	this->mode = mode;
	return true;
}

void FrameView::setMixCount(int mixCount)
{
	this->mixCount = mixCount;
}

int FrameView::getViewMode() const
{
	return mode;
}

void FrameView::setPlaybackSpeed(int playbackSpeed)
{
	this->playbackSpeed = playbackSpeed;
	if ( playbackTimer.isActive() ) {
		playbackTimer.setInterval(1000 / playbackSpeed);
	}
}


// The only thing left which is a little expensive is the MapRGB function.
SDL_Surface* FrameView::differentiateSurfaces(SDL_Surface *s1, SDL_Surface *s2)
{
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


void FrameView::freeProperty(const char *prop, const char *tag)
{
	if (strcmp(prop, tag) != 0) {
		xmlFree((xmlChar *)prop);
	}
}
