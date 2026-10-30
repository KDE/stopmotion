/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "src/application/soundhandler.h"

#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"

#include <QFileDialog>
#include <QInputDialog>


SoundHandler::SoundHandler(QObject *parent, QStatusBar *sb,
		const FrameBar* fb, const QString& homeDir, const char *name)
	: QObject(parent), statusBar(sb), frameBar(fb), homeDir(homeDir) {
	soundsList = nullptr;
	setObjectName(name);
}


void SoundHandler::setSoundsList(QListWidget *soundsList) {
	this->soundsList = soundsList;
}


void SoundHandler::addSound() {
	QString file = QFileDialog::
		getOpenFileName(nullptr, tr("Choose sound file"), homeDir, tr("Sounds (*.ogg)") );
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
		QString text = QInputDialog::getText(nullptr, tr("Sound name"),
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
