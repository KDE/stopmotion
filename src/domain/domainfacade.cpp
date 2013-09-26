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
#include "domainfacade.h"

#include "animation/animation.h"
#include "src/foundation/logger.h"
#include "src/domain/undo/undoadd.h"
#include "src/domain/undo/undomove.h"
#include "src/domain/undo/undoremove.h"
#include "src/domain/undo/undoaddscene.h"
#include "src/domain/undo/undoremovescene.h"
#include "src/domain/undo/undomovescene.h"


DomainFacade* DomainFacade::domainFacade = 0;


DomainFacade::DomainFacade() {
	animationModel = new Animation();
	domainFacade = NULL;
}


DomainFacade::~DomainFacade() {
	delete animationModel;
	animationModel = NULL;
}


DomainFacade* DomainFacade::getFacade()
{
	if(domainFacade == NULL) {
		domainFacade = new DomainFacade();
	}
	return domainFacade;
}


void DomainFacade::attatch(Observer *o)
{
	animationModel->attatch(o);
}


void DomainFacade::detatch(Observer *o)
{
	animationModel->detatch(o);
}


void DomainFacade::registerFrontend(Frontend *frontend)
{
	animationModel->registerFrontend(frontend);
}


Frontend* DomainFacade::getFrontend()
{
	return animationModel->getFrontend();
}


void DomainFacade::setActiveFrame(int frameNumber)
{
	animationModel->setActiveFrame(frameNumber);
}


int DomainFacade::getActiveFrameNumber()
{
	return animationModel->getActiveFrameNumber();
}


void DomainFacade::addFrames(const vector<const char*>& frameNames) {
	if ( !(animationModel->getActiveSceneNumber() < 0 && 
			animationModel->getNumberOfScenes() > 0) ) {
		Logger::get().logDebug("Adding frames in the domainfacade");
		animationModel->addFrames(frameNames,
				animationModel->getActiveFrameNumber() + 1);
	}
}


void DomainFacade::removeFrames(unsigned int fromFrame, unsigned int toFrame) {
	Logger::get().logDebug("Removing frames in the domainfacade");
	animationModel->removeFrames(fromFrame, toFrame);
}


void DomainFacade::moveFrames(unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition) {
	animationModel->moveFrames(fromFrame, toFrame, movePosition);
}


int DomainFacade::addSound(unsigned int frameNumber, const char *filename)
{
	Logger::get().logDebug("Adding sound in domainfacade");
	return animationModel->addSound(frameNumber, filename);
}


void DomainFacade::removeSound(unsigned int frameNumber, unsigned int soundNumber)
{
	animationModel->removeSound(frameNumber, soundNumber);
}


void DomainFacade::setSoundName(unsigned int frameNumber, unsigned int soundNumber, char * soundName)
{
	animationModel->setSoundName(frameNumber, soundNumber, soundName);
}


void DomainFacade::playFrame(int frameNumber)
{
	animationModel->playFrame(frameNumber);
}


bool DomainFacade::openProject(const char *filename)
{
	return animationModel->openProject(filename);
}


bool DomainFacade::saveProject(const char *directory)
{
	return animationModel->saveProject(directory);
}


bool DomainFacade::newProject()
{
	animationModel->clear();
	return true;
}


bool DomainFacade::isUnsavedChanges()
{
	return animationModel->isUnsavedChanges();
}


Frame* DomainFacade::getFrame(unsigned int frameNumber, unsigned int sceneNumber)
{
	return animationModel->getFrame(frameNumber, sceneNumber);
}


Frame * DomainFacade::getFrame( unsigned int frameNumber )
{
	return animationModel->getFrame(frameNumber);
}


unsigned int DomainFacade::getModelSize()
{
	return animationModel->getModelSize();
}


unsigned int DomainFacade::getSceneSize(int sceneNumber)
{
	return animationModel->getSceneSize(sceneNumber);
}


unsigned int DomainFacade::getNumberOfScenes( )
{
	return animationModel->getNumberOfScenes();
}


const char* DomainFacade::getProjectFile()
{
	return animationModel->getProjectFile();
}

const char* DomainFacade::getProjectPath()
{
	return animationModel->getProjectPath();
}

void DomainFacade::undo() {
	animationModel->undo();
}


void DomainFacade::redo() {
	animationModel->redo();
}


void DomainFacade::clearHistory() {
	animationModel->clearHistory();
}


void DomainFacade::setActiveScene( int sceneNumber )
{
	animationModel->setActiveScene(sceneNumber);
}


void DomainFacade::newScene(int index) {
	animationModel->newScene(index);
}


void DomainFacade::removeScene(int sceneNumber) {
	animationModel->removeScene(sceneNumber);
}


void DomainFacade::moveScene(int sceneNumber, int movePosition) {
	animationModel->moveScene(sceneNumber, movePosition);
}


int DomainFacade::getActiveSceneNumber()
{
	return animationModel->getActiveSceneNumber();
}


bool DomainFacade::initAudioDevice()
{
	return animationModel->initAudioDevice();
}


void DomainFacade::shutdownAudioDevice()
{
	return animationModel->shutdownAudioDevice();
}


bool DomainFacade::exportToVideo(VideoEncoder *encoder)
{
	return animationModel->exportToVideo(encoder);
}


bool DomainFacade::exportToCinerella(const char *file)
{
	return animationModel->exportToCinerella(file);
}


void DomainFacade::animationChanged(const char * alteredFile)
{
	animationModel->animationChanged(alteredFile);
}


const vector<GrabberDevice> DomainFacade::getGrabberDevices()
{
	return Util::getGrabberDevices();
}

