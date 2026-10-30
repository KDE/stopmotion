/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "src/presentation/frontends/qtfrontend/framepreferencesmenu.h"

#include "src/domain/domainfacade.h"
#include "graphics/icons/close.xpm"
#include "src/application/soundhandler.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"

#include <QToolTip>
#include <QLabel>
#include <QGridLayout>


FramePreferencesMenu::FramePreferencesMenu( QWidget * parent, 
		SoundHandler *soundHandler, const FrameBar* fb, const char * name )
	: MenuFrame(parent, name), soundHandler(soundHandler),
	  frameBar(fb) {
	soundsList = nullptr;
	soundsLabel = nullptr;
	closeButton = nullptr;
	addSoundButton = nullptr;
	removeSoundsButton = nullptr;
	changeNameButton = nullptr;
	grid = nullptr;
	
	this->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	grid = new QGridLayout;
	grid->setContentsMargins(1, 1, 1, 1);
	grid->setSpacing(1);
	
	addSoundButton = new QPushButton;
	addSoundButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	addSoundButton->setFocusPolicy( Qt::NoFocus );
	connect( addSoundButton, &QAbstractButton::clicked, soundHandler, &SoundHandler::addSound );
	
	removeSoundsButton = new QPushButton;
	removeSoundsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	removeSoundsButton->setFocusPolicy( Qt::NoFocus );
	connect( removeSoundsButton, &QAbstractButton::clicked, soundHandler, &SoundHandler::removeSound );
			
	changeNameButton = new QPushButton;
	changeNameButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	changeNameButton->setFocusPolicy( Qt::NoFocus );
	connect( changeNameButton, &QAbstractButton::clicked, soundHandler, &SoundHandler::setSoundName );
	
	closeButton = new QPushButton;
	closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	closeButton->setIcon(QPixmap(closeicon));
	closeButton->setFlat(true);
	connect(closeButton, &QAbstractButton::clicked, this, &FramePreferencesMenu::close );
	
	soundsLabel = new QLabel;
	soundsList = new QListWidget;
	
	grid->addWidget( soundsLabel, 0, 1 );
	grid->addWidget( addSoundButton, 1, 0 );
	grid->addWidget( removeSoundsButton, 2, 0 );
	grid->addWidget( changeNameButton, 3, 0 );
 	grid->addWidget( closeButton, 0, 2 );
	
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(soundsList);
	grid->addLayout( layout, 1, 1, 3, 2);
	
	grid->setColumnStretch(1, 1);
	setLayout(grid);
	soundHandler->setSoundsList(soundsList);
}


void FramePreferencesMenu::open() {
	soundsList->clear();
	int activeFrame = frameBar->getActiveFrame();
	int activeScene = frameBar->getActiveScene();
	DomainFacade* facade = DomainFacade::getFacade();
	int numSounds = facade->getNumberOfSounds(activeScene,
					activeFrame);
	for (int i = 0; i < numSounds; ++i) {
		const char* soundName = facade->getSoundName(
				activeScene, activeFrame, i);
		if (soundName)
			soundsList->addItem(new QListWidgetItem(soundName));
	}
	show();
}


void FramePreferencesMenu::close()
{
	MenuFrame::close((QWidget*)parent());
}


void FramePreferencesMenu::retranslateStrings()
{
	addSoundButton->setText(    tr("Add &sound"));
	removeSoundsButton->setText(tr("&Remove Sound"));
	changeNameButton->setText(  tr("Change name"));
	soundsLabel->setText(       tr("Sounds:"));
	
	QString infoText = 
			tr("<h4>Add sound</h4> "
			"<p>With this button you can <em>add sounds</em> to the selected frame.</p> "
			"<p>The sound will begin playing when this frame is shown and play "
			"until it is done.</p>");
	addSoundButton->setToolTip(infoText);
	addSoundButton->setWhatsThis(infoText);
	
	infoText = 
			tr("<h4>Remove sound</h4> "
			"<p>With this button you can <em>remove</em> the selected sound from "
			"this frame.</p>");
	removeSoundsButton->setToolTip(infoText);
	removeSoundsButton->setWhatsThis(infoText);
	
	infoText = 
			tr("<h4>Change name</h4> "
			"<p>With this button you can change the name of the selected sound. "
			"<BR>The name of the sound has no other effect than making it easier "
			"work with the animation.</p>");
	changeNameButton->setToolTip(infoText);
	changeNameButton->setWhatsThis(infoText);
	
	infoText = 
			tr("<h4>Sounds</h4> "
			"<p>This list shows all the sounds connected to this frame.</p>"
			"<p>The sounds will begin playing when this frame is shown and play "
			"until they are done.</p>");
	soundsLabel->setToolTip(infoText);
	soundsList->setWhatsThis(infoText);
}

FramePreferencesMenu::~FramePreferencesMenu() {
}
