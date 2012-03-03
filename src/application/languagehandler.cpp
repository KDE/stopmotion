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
#include "languagehandler.h"

#include "src/foundation/preferencestool.h"

#include <qdir.h>


LanguageHandler::LanguageHandler ( QObject *parent, QStatusBar *sb, QApplication *stApp,
		const char *name ) 
		: QObject(parent, name), statusBar(sb), stApp(stApp)
{
	qmPath = stApp->applicationDirPath() + "/translations";
	stTranslator = new QTranslator(this);
	stApp->installTranslator(stTranslator);
}


QPopupMenu* LanguageHandler::createLanguagesMenu(QPopupMenu *parent)
{
	languagesMenu = new QPopupMenu(parent);
	
	QDir dir(qmPath);
	QStringList fileNames = dir.entryList("stopmotion_*.qm");
	
	//English is a special case (base language)
	int id = languagesMenu->insertItem( "&1 English", 
			this, SLOT(switchToLanguage(int)), 0, 0 );
	if(!strcmp(PreferencesTool::get()->getPreference("language", "en"), "en")) {
		languagesMenu->setItemChecked(id, true);
	}
	
	
	int nr = 2;
	for(int i=0; i < (int)fileNames.size(); ++i) {
		QTranslator translator;
		translator.load(fileNames[i], qmPath);
		
		QTranslatorMessage message =
				translator.findMessage("LanguageHandler", "English", 
								"This should be translated to the name of the "
								"language you are translating to, in that language. "
								"Example: English = Deutsch (Deutsch is \"German\" "
								"in German)");
		QString language = message.translation();
		
		//Checks that the mimimum requirement for accepting a string is covered.
		//The mimimum requirement is that the menu option string (English) is translated.
		if(language != "") {
			id = languagesMenu->insertItem( QString("&%1 %2").arg(nr).arg(language),
				this, SLOT(switchToLanguage(int)), 0, nr-1 );
			++nr;
			
			QString locale = fileNames[i];
			locale = locale.mid(locale.find('_') + 1);
			locale.truncate(locale.find('.'));
			locales.push_back(locale);
			
			//Checks the menu option if this is the starting language.
			if(!strcmp(PreferencesTool::get()->getPreference("language", "en"), locale)) {
				languagesMenu->setItemChecked(id, true);
			}
		}
	}
	
	return languagesMenu;
	
	
	/*QString infoText =
		tr("<h4>English</h4> "
		"<p>Select this menu options to get the <em>English</em> translation of "
		"the application.</p>");
	languagesMenu->setWhatsThis(id, infoText);*/
	/*id = languagesMenu->insertItem( tr("&Norwegian"), 
			this, SLOT(switchToLanguage(int)) );
	infoText =
		tr("<h4>Norwegian</h4> "
		"<p>Select this menu options to get the <em>Norwegian</em> translation of "
		"the application.</p>");
	languagesMenu->setWhatsThis(id, infoText);*/
	

	
}


void LanguageHandler::switchToLanguage(int menuID)
{
	//English(0) is a special case (base language)
	if(menuID == 0) {
		stTranslator->clear();
		PreferencesTool::get()->setPreference("language", "en");
	}
	else {
		stTranslator->load("stopmotion_" + locales[menuID-1], qmPath);
		PreferencesTool::get()->setPreference("language", locales[menuID-1].ascii());
	}
	
	for(int i=0; i < (int)languagesMenu->count(); ++i) {
		languagesMenu->setItemChecked(i, i == menuID);
	}
	
	emit languageChanged();
}

