/***************************************************************************
 *   Copyright (C) 2014 by Linuxstopmotion contributors;                   *
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
	EditObserver(const EditObserver&);
	EditObserver& operator=(const EditObserver&);
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
