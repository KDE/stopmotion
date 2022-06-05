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
#include "src/application/soundhandler.h"

#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"

#include <QFileDialog>
#include <QInputDialog>


SoundHandler::SoundHandler(QObject *parent, QStatusBar *sb,
		const FrameBar* fb, const char* homeDir, const char *name)
	: QObject(parent), statusBar(sb), frameBar(fb), homeDir(homeDir) {
	soundsList = NULL;
	setObjectName(name);
}


void SoundHandler::setSoundsList(QListWidget *soundsList) {
	this->soundsList = soundsList;
}


void SoundHandler::addSound() {
	QString file = QFileDialog::
		getOpenFileName(0, tr("Choose sound file"), QString(homeDir), tr("Sounds (*.ogg)") );
	if ( !file.isNull() ) {
		DomainFacade *facade = DomainFacade::getFacade();
		int activeScene = frameBar->getActiveScene();
		int activeFrame = frameBar->getActiveFrame();
		facade->addSound( activeScene, activeFrame,
				file.toLocal8Bit().constData() );
	}
}


void SoundHandler::removeSound() {
	int index = soundsList->currentRow();
	if (index >= 0) {
		int scene = frameBar->getActiveScene();
		int frame = frameBar->getActiveFrame();
		DomainFacade::getFacade()->removeSound(scene, frame, index);
	}
}


void SoundHandler::setSoundName()
{
	int index = soundsList->currentRow();
	if (index >= 0) {
		bool ok = false;
		QString text = QInputDialog::getText(0, tr("Sound name"),
				tr("Enter the name of the sound:"),
				QLineEdit::Normal,QString(), &ok);
		if ( ok && !text.isEmpty() ) {
			int scene = frameBar->getActiveScene();
			int frame = frameBar->getActiveFrame();
			DomainFacade::getFacade()->setSoundName(scene, frame,
					index, text.toLocal8Bit().data() );
		}
	}
}
