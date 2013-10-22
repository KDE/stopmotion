/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
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
#include "domainfacade.h"

#include "animation/animation.h"
#include "src/foundation/logger.h"
#include "src/presentation/frontends/frontend.h"

DomainFacade* DomainFacade::domainFacade = 0;

const char* DomainFacade::getImagePath(int scene, int frame) {
	return animationModel->getFrame2(scene, frame)->getImagePath();
}

int DomainFacade::soundCount(int scene, int frame) const {
	return animationModel->soundCount(scene, frame);
}

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


void DomainFacade::attach(Observer *o)
{
	animationModel->attach(o);
}


void DomainFacade::detach(Observer *o)
{
	animationModel->detach(o);
}


void DomainFacade::registerFrontend(Frontend *frontend)
{
	animationModel->registerFrontend(frontend);
}


Frontend* DomainFacade::getFrontend()
{
	return animationModel->getFrontend();
}


void DomainFacade::addFrames(int scene, int frame,
		const vector<const char*>& frameNames) {
	Logger::get().logDebug("Adding frames in the domainfacade");
	animationModel->addFrames(scene, frame, frameNames);
}


void DomainFacade::removeFrames(int scene, int frame, int count) {
	Logger::get().logDebug("Removing frames in the domainfacade");
	animationModel->removeFrames(scene, frame, count);
}


void DomainFacade::moveFrames(int fromScene, int fromFrame,
		int count, int toScene, int toFrame) {
	animationModel->moveFrames(fromScene, fromFrame, count, toScene, toFrame);
}


int DomainFacade::addSound(int scene, int frame, const char *filename) {
	Logger::get().logDebug("Adding sound in domainfacade");
	return animationModel->addSound(scene, frame, filename);
}


void DomainFacade::removeSound(int sceneNumber, int frameNumber,
		int soundNumber) {
	animationModel->removeSound(sceneNumber, frameNumber, soundNumber);
}


void DomainFacade::setSoundName(int sceneNumber, int frameNumber,
		int soundNumber, const char* soundName) {
	animationModel->setSoundName(sceneNumber, frameNumber, soundNumber,
			soundName);
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


int DomainFacade::getModelSize() const {
	return animationModel->frameCount();
}


int DomainFacade::getSceneSize(int sceneNumber) const {
	return animationModel->frameCount(sceneNumber);
}


int DomainFacade::getNumberOfScenes() const {
	return animationModel->sceneCount();
}


int DomainFacade::getNumberOfSounds(int scene, int frame) const {
	return animationModel->soundCount(scene, frame);
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


void DomainFacade::newScene(int index) {
	animationModel->newScene(index);
}


void DomainFacade::removeScene(int sceneNumber) {
	animationModel->removeScene(sceneNumber);
}


void DomainFacade::moveScene(int sceneNumber, int movePosition) {
	animationModel->moveScene(sceneNumber, movePosition);
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

const char* DomainFacade::getSoundName(int sceneNumber, int frameNumber,
		int soundNumber) const {
	return animationModel->getFrame2(sceneNumber, frameNumber)
			->getSoundName(soundNumber);
}
