/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EDITOBSERVER_H_
#define EDITOBSERVER_H_

#include "src/presentation/observer.h"

class QFileSystemWatcher;

/**
 * Tells a {@c QFileSystemWatcher} when a file is replaced (indicating that
 * it is being edited).
 */
class EditObserver: public Observer {
	QFileSystemWatcher* fsw;
	EditObserver(const EditObserver&) = delete;
	EditObserver& operator=(const EditObserver&) = delete;
public:
	EditObserver(QFileSystemWatcher* watcher);
	~EditObserver() override;
	void updateAdd(int scene, int index, int numFrames) override;
	void updateRemove(int scene, int fromFrame, int toFrame) override;
	void updateMove(int fromScene, int fromFrame, int count,
			int toScene, int toFrame) override;
	void updateClear() override;
	void updateNewScene(int index) override;
	void updateRemoveScene(int sceneNumber) override;
	void updateMoveScene(int sceneNumber, int movePosition) override;
	void updateAnimationChanged(int sceneNumber, int frameNumber) override;
	void updateSoundChanged(int sceneNumber, int frameNumber) override;
	void resync() override;
};

#endif /* EDITOBSERVER_H_ */
