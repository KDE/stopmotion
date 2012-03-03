/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#include "framepreferencesmenu.h"

#include "src/domain/domainfacade.h"

#include "graphics/icons/close.xpm"

#include <qlayout.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

FramePreferencesMenu::FramePreferencesMenu( QWidget * parent, 
		SoundHandler *soundHandler, const char * name )
		: MenuFrame(parent, name), soundHandler(soundHandler)
{
	this->setFrameStyle(QFrame::PopupPanel | QFrame::Sunken);
	
	QGridLayout *grid = new QGridLayout( this, 1, 1, 3 );
	
	addSoundButton = new QPushButton( this );
	addSoundButton->setFocusPolicy( QWidget::NoFocus );
	QObject::connect( addSoundButton, SIGNAL( clicked() ), 
			soundHandler, SLOT( addSound() ) );
	
	removeSoundsButton = new QPushButton(this);
	removeSoundsButton->setFocusPolicy( QWidget::NoFocus );
	QObject::connect( removeSoundsButton, SIGNAL( clicked() ), 
			soundHandler, SLOT( removeSound() ) );
			
	changeNameButton = new QPushButton(this);
	changeNameButton->setFocusPolicy( QWidget::NoFocus );
	QObject::connect( changeNameButton, SIGNAL( clicked() ), 
			soundHandler, SLOT( setSoundName() ) );
	
	closeButton = new QPushButton(this);
	closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
	closeButton->setPixmap(closeicon);
	closeButton->setFlat(true);
	QObject::connect(closeButton, SIGNAL(clicked()),
			this, SLOT(close()) );
	
	QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, 
			QSizePolicy::Fixed);
	
	soundsLabel = new QLabel(this);
	
	soundsList = new QListBox(this);
	
	QSpacerItem *space = new QSpacerItem(0, 7);
	
	grid->addWidget( addSoundButton, 1, 0 );
	grid->addWidget( removeSoundsButton, 2, 0 );
	grid->addWidget( changeNameButton, 3, 0 );
	grid->addWidget( soundsLabel, 0, 1 );
	grid->addItem( spacer, 0, 2 );
 	grid->addWidget( closeButton, 0, 3 );
	grid->addMultiCellWidget( soundsList, 1, 3, 1, 3, 0 );
	grid->addItem( space, 4, 0 );
	
	soundHandler->setSoundsList(soundsList);
}


void FramePreferencesMenu::open()
{
	soundsList->clear();
	unsigned int numSounds = DomainFacade::getFacade()->getFrame( 
			DomainFacade::getFacade()->getActiveFrameNumber() )->
			getNumberOfSounds();
	
	unsigned int activeFrame = DomainFacade::getFacade()->getActiveFrameNumber();
	for(unsigned int i = 0; i<numSounds; i++) {
		soundsList->insertItem( DomainFacade::getFacade()->getFrame(activeFrame)->
				getSoundName(i) );
	}
	this->show();
}


void FramePreferencesMenu::close()
{
	MenuFrame::close((QWidget*)parent());
}


void FramePreferencesMenu::retranslateStrings()
{
	addSoundButton->setText(	tr("Add &sound"));
	removeSoundsButton->setText(tr("&Remove Sound"));
	changeNameButton->setText(	tr("Change name"));
	soundsLabel->setText(		tr("Sounds:"));
	
	QString infoText = 
			tr("<h4>Add sound</h4> "
			"<p>With this button you can <em>add sounds</em> to the selected frame.</p> "
			"<p>The sound will begin playing when this frame is shown and play "
			"until it is done.</p>");
	QToolTip::add( addSoundButton, infoText );
	QWhatsThis::add( addSoundButton, infoText );
	
	infoText = 
			tr("<h4>Remove sound</h4> "
			"<p>With this button you can <em>remove</em> the selected sound from "
			"this frame.</p>");
	QToolTip::add( removeSoundsButton, infoText );
	QWhatsThis::add( removeSoundsButton, infoText );
	
	infoText = 
			tr("<h4>Change name</h4> "
			"<p>With this button you can change the name of the selected sound. "
			"<BR>The name of the sound has no other effect than making it easier "
			"work with the animation.</p>");
	QToolTip::add( changeNameButton, infoText );
	QWhatsThis::add( changeNameButton, infoText );
	
	infoText = 
			tr("<h4>Sounds</h4> "
			"<p>This lists shows all the sounds connected to this frame.</p>"
			"<p>The sounds will begin playing when this frame is shown and play "
			"until they are done.</p>");
	QWhatsThis::add( soundsLabel, infoText );
	QWhatsThis::add( soundsList, infoText );
}
