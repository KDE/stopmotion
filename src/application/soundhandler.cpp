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
#include "soundhandler.h"

#include "src/domain/domainfacade.h"

#include <qfiledialog.h>
#include <qinputdialog.h>

SoundHandler::SoundHandler ( QObject *parent, QStatusBar *sb, const char* homeDir,
		const char *name ) 
		: QObject(parent, name), statusBar(sb), homeDir(homeDir)
{
	soundsList = NULL;
}


void SoundHandler::setSoundsList(QListBox *soundsList)
{
	this->soundsList = soundsList;
}


void SoundHandler::addSound()
{
	QString file = QFileDialog::getOpenFileName(homeDir, tr("Sounds (*.ogg)") );
	if ( !file.isNull() ) {
		int ret = DomainFacade::getFacade()->addSound( DomainFacade::getFacade()->
				getActiveFrameNumber(), (char*)file.ascii() );
		if (ret == 0) {
			soundsList->insertItem( DomainFacade::getFacade()->
					getFrame(DomainFacade::getFacade()->
					getActiveFrameNumber())->getSoundName(soundsList->numRows()) );
			emit soundsChanged();
		}
	}
	else {
		Logger::get().logWarning("openDialog returned a NULL pointer");
	}
}


void SoundHandler::removeSound()
{
	int index = soundsList->index(soundsList->selectedItem());
	
	if(index >= 0) {
		DomainFacade::getFacade()->removeSound( 
			DomainFacade::getFacade()->getActiveFrameNumber(), index );
		
		soundsList->removeItem(index);
		emit soundsChanged();
	}
}


void SoundHandler::setSoundName()
{
	int index = soundsList->index(soundsList->selectedItem());
	
	if(index >= 0) {
		bool ok;
		QString text = QInputDialog::getText(
			tr("Sound name"), tr("Enter the name of the sound:"), QLineEdit::Normal,
			QString::null, &ok, (QWidget*)this->parent() );
		
		if ( ok && !text.isEmpty() ) {
			DomainFacade::getFacade()->setSoundName(DomainFacade::getFacade()->
					getActiveFrameNumber(), index, (char*)text.ascii() );
	
			soundsList->changeItem( text, index );
		}
	}
}
