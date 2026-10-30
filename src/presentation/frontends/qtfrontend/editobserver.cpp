/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "editobserver.h"
#include "src/domain/domainfacade.h"

#include <QFileSystemWatcher>

EditObserver::EditObserver(QFileSystemWatcher* watcher) : fsw(watcher) {
}

EditObserver::~EditObserver() {
}

void EditObserver::updateAdd(int, int, int) {
}

void EditObserver::updateRemove(int, int, int) {
}

void EditObserver::updateMove(int, int, int, int, int) {
}

void EditObserver::updateClear() {
}

void EditObserver::updateNewScene(int) {
}

void EditObserver::updateRemoveScene(int) {
}

void EditObserver::updateMoveScene(int, int) {
}

void EditObserver::updateAnimationChanged(int sceneNumber, int frameNumber) {
	const char *path = DomainFacade::getFacade()->getImagePath(sceneNumber,
				frameNumber);
	if (path && fsw) {
		fsw->addPath(path);
	}
}

void EditObserver::updateSoundChanged(int, int) {
}

void EditObserver::resync() {
}
