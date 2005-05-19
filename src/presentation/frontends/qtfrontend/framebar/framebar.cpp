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
#include "framebar.h"

#include "src/domain/domainfacade.h"
#include "framethumbview.h"
#include "scenethumbview.h"

#include <qlabel.h>
#include <qimage.h>
#include <qframe.h>
#include <qdragobject.h> 


FrameBar::FrameBar(QWidget *parent)
		: QScrollView(parent)
{
	setMaximumHeight(115);
	//setMinimumHeight(115);
	
	//I have no idea why there is - 20 as the scrollbar is 30 pixels high, but
	//it works...
	FRAMEBAR_HEIGHT = this->height() - 20;
	
	preferencesMenu = 0;
	activeFrame = -1;
	activeScene = -1;
	movingScene = 0;
	selecting = false;
	selectionFrame = -1;
	
	Logger::get().logDebug("FrameBar is attatched to the model");
	DomainFacade::getFacade()->attatch(this);
	
	
	setHScrollBarMode ( AlwaysOn );
	enableClipper(TRUE);
	viewport()->setAcceptDrops(true);
	setDragAutoScroll(true);
}


FrameBar::~FrameBar()
{
}


void FrameBar::updateAdd(const vector<char*>& frames, unsigned int index, Frontend *frontend)
{
	Logger::get().logDebug("Adding in framebar");
	addFrames(frames, index, frontend);
	
	emit modelSizeChanged(DomainFacade::getFacade()->getModelSize());
}


void FrameBar::updateRemove(unsigned int fromFrame, unsigned int toFrame)
{
	Logger::get().logDebug("Recieving notification about the removal of a frame in the model");
	removeFrames(fromFrame, toFrame);
	
	emit modelSizeChanged(DomainFacade::getFacade()->getModelSize());
}


void FrameBar::updateMove(unsigned int fromFrame, unsigned int toFrame, unsigned int movePosition)
{
	moveFrames(fromFrame, toFrame, movePosition);
}


void FrameBar::updateNewActiveFrame(int frameNumber)
{
	setActiveFrame(frameNumber);
	
	if( preferencesMenu->isShown()  ) {
		if(frameNumber >= 0) {
			showPreferencesMenu();
		}
		else {
			preferencesMenu->close();
		}
	}
	
	//For writing the frame number in the frame number display
	emit newActiveFrame(QString("Frame number: %1").arg(frameNumber+1));
	//For setting the value in the gotSpinbox in the gotomenu
	emit newActiveFrame( frameNumber + 1);
}


void FrameBar::updateClear()
{
	int size = thumbViews.size();
	for(unsigned int i = 0; i < (unsigned int)size; i++) {
		delete thumbViews[0];
		thumbViews.erase(thumbViews.begin());
	}
	activeFrame = -1;
	activeScene = -1;
}


void FrameBar::updatePlayFrame(int)
{

}


void FrameBar::updateAnimationChanged(int frameNumber)
{
	thumbViews[frameNumber+activeScene+1]->setPixmap( QPixmap( QImage(
			QString(DomainFacade::getFacade()->getFrame(frameNumber)->getImagePath()) ).
			scale(FRAMEBAR_HEIGHT-3, FRAMEBAR_HEIGHT) ) );
	thumbViews[frameNumber]->repaint();
}


void FrameBar::addFrames( const vector< char * > & frames, unsigned int index, 
		Frontend * frontend )
{
	Logger::get().logDebug("Adding frames in framebar");
	frontend->setProgressInfo("Adding frames to project ...");
	
	unsigned int i;
	unsigned int size = thumbViews.size();
	unsigned int framesSize = frames.size();
	
	
	//Move the frames behind the place we are inserting the new ones.
	unsigned int start = index+activeScene+1;
	for(i=start; i<size; i++) {
		moveChild( thumbViews[i], childX(thumbViews[i]) +  framesSize*FRAMEBAR_HEIGHT, 0 );
	}
	
	unsigned int stop = size-DomainFacade::getFacade()->getNumberOfScenes()+activeScene+1;	
	unsigned int moveDistance = framesSize-(activeScene+1);
	for(i=start; i<stop; i++) {
		thumbViews[i]->setNumber(i+moveDistance);
	}
	
	
	// Adds the new frames to the framebar
	ThumbView *thumb;
	bool operationCanceled = false;
	for(i=0; i<framesSize; i++) {
		thumb = new FrameThumbView(this, this->viewport(), index+i);
		
		thumb->setMinimumHeight(FRAMEBAR_HEIGHT);
		thumb->setMaximumHeight(FRAMEBAR_HEIGHT);
		thumb->setMinimumWidth(FRAMEBAR_HEIGHT);
		thumb->setMaximumWidth(FRAMEBAR_HEIGHT);
		thumb->setPixmap( QPixmap(QImage(frames[i]).
				scale(FRAMEBAR_HEIGHT-3, FRAMEBAR_HEIGHT)) );
		
		//Sets the note icon on the respective frames.
		if(DomainFacade::getFacade()->getFrame(i)->getNumberOfSounds() > 0 ) {
			thumb->setHasSounds(true);
		}
		
		thumbViews.insert(thumbViews.begin() + index+activeScene+1+i, thumb);
		this->addChild(thumb, (index+activeScene+1+i)*FRAMEBAR_HEIGHT);
		
		thumb->show();
		
		frontend->updateProgress(framesSize + i);
		if ( (i % 10) == 0 ) {
			frontend->processEvents();
		}
		
		if( frontend->isOperationAborted() ) {
			operationCanceled = true;
			break;
		}
	}
	
	if(operationCanceled) {
		unsigned int j;
		unsigned int k;
		for(j=index+1+i, k = index; j<i+size+1  ; j++, k++) {
			moveChild( thumbViews[j], childX(thumbViews[j]) -  framesSize*FRAMEBAR_HEIGHT, 0 );
			thumbViews[j]->setNumber(k);
		}
		
		for(j=index; j<=index+i; j++) {
			delete thumbViews[index];
		}
		thumbViews.erase( thumbViews.begin() + index, thumbViews.begin() + index + i);
	}
	else {
		this->resizeContents(thumbViews.size()*FRAMEBAR_HEIGHT, FRAMEBAR_HEIGHT);
	}
}


void FrameBar::removeFrames(unsigned int fromFrame, unsigned int toFrame)
{
	fromFrame += activeScene+1;
	toFrame += activeScene+1;
	
	//The frames to be deleted is between other frames
	unsigned int size = thumbViews.size();
	if( toFrame < thumbViews.size() - 1) {
		//Move all frames behind the deleted frames forward.
		for(unsigned int k=toFrame+1; k<size; k++) {
			moveChild( thumbViews[k], childX(thumbViews[k]) - 
					(toFrame-fromFrame+1)*FRAMEBAR_HEIGHT, 0 );
		}
		
		for(unsigned int k=toFrame+1; k<size-DomainFacade::getFacade()->
				getNumberOfScenes()+activeScene+1; k++) {
				
			thumbViews[k]->setNumber( k-(toFrame-fromFrame+2)-activeScene );
		}
	}
	
	
	for(unsigned int i=fromFrame; i<=toFrame; i++) {
		delete thumbViews[fromFrame];
		thumbViews.erase( thumbViews.begin() + fromFrame );
	}

	this->resizeContents(thumbViews.size()*FRAMEBAR_HEIGHT, FRAMEBAR_HEIGHT);
}


void FrameBar::moveFrames(unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition)
{
	fromFrame += activeScene+1;
	toFrame += activeScene+1;
	movePosition += activeScene+1;
	
	
	if(movePosition < fromFrame) {
		for(unsigned int i=movePosition; i<fromFrame; i++) {
			moveChild( thumbViews[i], childX(thumbViews[i]) + 
					FRAMEBAR_HEIGHT*(toFrame-fromFrame+1), 0 );
			thumbViews[i]->setNumber(i+(toFrame-fromFrame)-activeScene);
		}
		
		for(unsigned int j=fromFrame; j<=toFrame; j++) {
			moveChild( thumbViews[j], childX(thumbViews[j]) - 
					FRAMEBAR_HEIGHT*(fromFrame-movePosition), 0 );
			moveThumbView(j, j-(fromFrame-movePosition));
		}
	}
	else if(movePosition > fromFrame) {
		for(unsigned int i=toFrame+1; i<=movePosition; i++) {
			moveChild( thumbViews[i], childX(thumbViews[i]) - 
					FRAMEBAR_HEIGHT*(toFrame-fromFrame+1), 0 );
			thumbViews[i]->setNumber(i-(toFrame-fromFrame+2)-activeScene);
		}
		
		//toFrame-j instead of just j because i have to do a reversed loop (j--) 
		//but can't do it directly on unsigned ints.
		for(unsigned int j=fromFrame, k=toFrame; j<=toFrame; j++, k--) {
			moveChild( thumbViews[k], childX(thumbViews[k]) +
					FRAMEBAR_HEIGHT*(movePosition-toFrame), 0 );
			moveThumbView(k, k+(movePosition-toFrame));
		}
	}
}


void FrameBar::moveThumbView(unsigned int fromPosition, unsigned int toPosition)
{
	ThumbView *f = thumbViews[fromPosition];
	f->setNumber(toPosition-(activeScene+1));
	thumbViews.erase(thumbViews.begin() + fromPosition);
	thumbViews.insert(thumbViews.begin() + toPosition, f );
}


void FrameBar::setActiveFrame(int frameNumber)
{
	//If there is a frame to set as active
	if(frameNumber >= 0) {
		Logger::get().logDebug("Setting new active frame in FrameBar");
		
		int thumbNumber = frameNumber+activeScene+1;
		
		int from = activeFrame+activeScene+1;
		int to = selectionFrame+activeScene+1;
		int highend = (from<to) ? to : from;
		int lowend = (from>to) ? to : from;
		if(highend < (int)thumbViews.size()) {
			for(int i=lowend; i<=highend; i++) {
				thumbViews[i]->setSelected(false);
			}
		}
		thumbViews[thumbNumber]->setPixmap( QPixmap( 
				QImage(DomainFacade::getFacade()->getFrame(frameNumber)->getImagePath()).
				scale(FRAMEBAR_HEIGHT-3, FRAMEBAR_HEIGHT) ) );
		
		thumbViews[thumbNumber]->setSelected(true);
		
		this->center(frameNumber*FRAMEBAR_HEIGHT, 0, 0.0, 0.0);
	}
	activeFrame = frameNumber;
	selectionFrame = frameNumber;
	this->selecting = false;
}


void FrameBar::setSelecting(bool selecting)
{
	this->selecting = selecting;
}


bool FrameBar::isSelecting()
{
	return selecting;
}


/**
 *@todo Make the loop who clears the borders more effective so that it
 *      only clears the borders needing clearing and not everything. And there
 *      other loops don't have to paint the activeFrame anew.
 */
void FrameBar::setSelection(int selectionFrame)
{
	this->selectionFrame = selectionFrame;
	selectionFrame += activeScene+1;
	
	for(unsigned int i=0; i<thumbViews.size(); i++) {
		thumbViews[i]->setFrameShape(QFrame::NoFrame);
	}
	
	int activeFrame = this->activeFrame+activeScene+1;
	if(selectionFrame >= activeFrame) {
		for(int i=activeFrame; i<=selectionFrame; i++) {
			thumbViews[i]->setSelected(true);
		}
	}
	else if(this->selectionFrame < activeFrame) {
		for(int i=selectionFrame; i<=activeFrame; i++) {
			thumbViews[i]->setSelected(true);
		}
	}
}


int FrameBar::getSelectionFrame()
{
	return selectionFrame;
}


void FrameBar::setPreferencesMenu( FramePreferencesMenu * preferencesMenu )
{
	this->preferencesMenu = preferencesMenu;
}


void FrameBar::showPreferencesMenu()
{
	preferencesMenu->open();
}


void FrameBar::frameSoundsChanged()
{
	int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
	int activeThumb = activeFrame + DomainFacade::getFacade()->
			getActiveSceneNumber() + 1;
	
	if(DomainFacade::getFacade()->getFrame(activeFrame)->getNumberOfSounds() > 0 ) {
		thumbViews[activeThumb]->setHasSounds(true);
	}
	else {
		thumbViews[activeThumb]->setHasSounds(false);
	}
}


void FrameBar::updateNewScene( int index )
{
	this->newScene(index);
}


void FrameBar::newScene(int index)
{
	if(index > 0)  {
		for(unsigned int i=index+(DomainFacade::getFacade()->getSceneSize(index-1)); 
				i<thumbViews.size(); i++) {
			moveChild( thumbViews[i], childX(thumbViews[i]) + FRAMEBAR_HEIGHT, 0 );
			thumbViews[i]->setNumber(thumbViews[i]->getNumber()+1);
			thumbViews[i]->repaint();
		}
	}
	
	ThumbView *thumb = new SceneThumbView(this, this->viewport(), index, "scene");
	
	thumb->setMinimumHeight(FRAMEBAR_HEIGHT);
	thumb->setMaximumHeight(FRAMEBAR_HEIGHT);
	thumb->setMinimumWidth(FRAMEBAR_HEIGHT);
	thumb->setMaximumWidth(FRAMEBAR_HEIGHT);
	
	if(DomainFacade::getFacade()->getActiveSceneNumber() >= 0) {
	index = (index > 0) ? index + DomainFacade::getFacade()->getSceneSize(index-1) :
			index;
	}
	
	thumbViews.insert(thumbViews.begin() + index, thumb);
	this->addChild(thumb, (index)*FRAMEBAR_HEIGHT);
	thumb->show();
	this->resizeContents(thumbViews.size()*FRAMEBAR_HEIGHT, FRAMEBAR_HEIGHT);
	
	emit modelSizeChanged(DomainFacade::getFacade()->getModelSize());
}


void FrameBar::updateNewActiveScene(int sceneNumber, vector<char*> framePaths,
		Frontend *frontend)
{
	this->setActiveScene(sceneNumber, framePaths, frontend);
}


void FrameBar::updateRemoveScene( int sceneNumber )
{
	this->removeScene(sceneNumber);
}


void FrameBar::removeScene(int sceneNumber)
{	
	int delThumb = sceneNumber;
	
	if(sceneNumber > activeScene  && sceneNumber > 0) {
		delThumb += DomainFacade::getFacade()->getSceneSize(activeScene);
	}
	
	delete thumbViews[delThumb];
	thumbViews.erase(thumbViews.begin() + delThumb);
	
	unsigned int size = thumbViews.size();
	for(unsigned int i=delThumb; i<size; i++) {
		if( strcmp(thumbViews[i]->name(), "scene") == 0 ) {
			thumbViews[i]->setNumber(thumbViews[i]->getNumber()-1);

		}
					thumbViews[i]->repaint();
		moveChild( thumbViews[i], childX(thumbViews[i]) - FRAMEBAR_HEIGHT, 0 );
	}
	
	if(activeScene > sceneNumber) {
		activeScene -= 1;
	}
	
	this->resizeContents(thumbViews.size()*FRAMEBAR_HEIGHT, FRAMEBAR_HEIGHT);
}


void FrameBar::updateMoveScene( int sceneNumber, int movePosition )
{
	this->moveScene(sceneNumber, movePosition);
}


void FrameBar::moveScene(int sceneNumber, int movePosition)
{
	if(movePosition < sceneNumber) {
		for(int i=movePosition; i<sceneNumber; i++) {
			thumbViews[i]->setNumber(thumbViews[i]->getNumber()+1);
			moveChild( thumbViews[i], childX(thumbViews[i]) + FRAMEBAR_HEIGHT, 0 );
			thumbViews[i]->repaint();
		}
	}
	else {
		for(int i=sceneNumber+1; i<=movePosition; i++) {
			thumbViews[i]->setNumber(thumbViews[i]->getNumber()-1);
			moveChild( thumbViews[i], childX(thumbViews[i]) - FRAMEBAR_HEIGHT, 0 );
			thumbViews[i]->repaint();
		}
	}
	moveChild( thumbViews[sceneNumber], childX(thumbViews[sceneNumber]) 
			- FRAMEBAR_HEIGHT*(sceneNumber-movePosition), 0 );
	
	ThumbView *f = thumbViews[sceneNumber];
	f->setNumber(movePosition);
	thumbViews.erase(thumbViews.begin() + sceneNumber);
	thumbViews.insert(thumbViews.begin() + movePosition, f);
	f->repaint();
}


void FrameBar::setActiveScene( int sceneNumber, vector<char*> framePaths,
		Frontend *frontend)
{
	if( activeScene >= 0) {	
		this->removeFrames(0, DomainFacade::getFacade()->
				getSceneSize(activeScene)-1);
		thumbViews[activeScene]->setOpened(false);
	}
	
	this->activeScene = sceneNumber;
	
	if( sceneNumber >= 0 ) {
		thumbViews[activeScene]->setOpened(true);
		
		if(framePaths.size() > 0) {
			this->addFrames(framePaths, 0, frontend);
			setActiveFrame(0);
		}
		else {
			setActiveFrame(-1);
		}
	}
	this->resizeContents(thumbViews.size()*FRAMEBAR_HEIGHT, FRAMEBAR_HEIGHT);
}


int FrameBar::getMovingScene()
{
	return movingScene;
}


void FrameBar::setMovingScene(int movingScene)
{
	this->movingScene = movingScene;
}


void FrameBar::contentsDragEnterEvent(QDragEnterEvent * event)
{
	event->accept( QUriDrag::canDecode(event) );
}


void FrameBar::contentsDropEvent(QDropEvent * event)
{
	int index = (event->pos().x()/FRAMEBAR_HEIGHT);
	if(index < (int)thumbViews.size()) {
		thumbViews[index]->contentsDropped(event);
	}
}


//Call the widgets custom ThumbViews::resize() functions and then
//FrameBar::packWidgets() instead of working on the widgets from the outside. 
void FrameBar::resizeEvent(QResizeEvent * event)
{
	int diff = FRAMEBAR_HEIGHT-(this->height()-20);
	
	FRAMEBAR_HEIGHT = this->height() - 20;
	
	
	//Refresh the widgets
	unsigned int size = thumbViews.size();
	for(unsigned int i=0; i<size; ++i) {
		
		moveChild( thumbViews[i], childX(thumbViews[i]) - diff*i, 0 );
		
		thumbViews[i]->resizeThumb(FRAMEBAR_HEIGHT);
		thumbViews[i]->repaint();
	}
	resizeContents(thumbViews.size()*FRAMEBAR_HEIGHT, FRAMEBAR_HEIGHT);
	QScrollView::resizeEvent(event);
}

void FrameBar::setOpeningScene(bool openingScene)
{
	this->openingScene = openingScene;
}


bool FrameBar::isOpeningScene()
{
	return openingScene;
}
