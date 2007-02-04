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
#include "src/application/languagehandler.h"

#include "src/foundation/preferencestool.h"
#include "src/config.h"

#include <QDir>
#include <QTranslator>
#include <QLocale>


LanguageHandler::LanguageHandler(QObject *parent, QApplication *stApp, const char *name) 
	: QObject(parent)
{
	qmPath = QString(stopmotion::translationsDirectory);
	activeAction = 0;
	translator = new QTranslator(this);
	
	const char *localePtr = QLocale::system().name().toLatin1().constData();
	const char *languagePref = PreferencesTool::get()->getPreference("language", localePtr);
	QString locale(languagePref);

	if ( !locale.startsWith("en") ) {
		Logger::get().logDebug("Loading translator: ");
		Logger::get().logDebug(locale.toLatin1().constData());
		translator->load( "stopmotion_" + locale, qmPath);
	}
	
	stApp->installTranslator(translator);
	setObjectName(name);
	
	if (strcmp(languagePref, localePtr) != 0) {
		xmlFree((xmlChar*)languagePref);
	}
}


QMenu* LanguageHandler::createLanguagesMenu(QMenu *parent)
{
	assert(parent);
	// For the .po files. findtr isn't as intelligent as luptate
	tr("English");
	
	languagesMenu = parent->addMenu(tr("&Translation"));
	connect(languagesMenu, SIGNAL(triggered(QAction *)), this, SLOT(changeLanguage(QAction *)));
	
	QDir dir(qmPath);
	QStringList fileNames = dir.entryList(QStringList("stopmotion_*.qm"));
	
	//English is a special case (base language)
	QAction *langAct = languagesMenu->addAction("&1 English");
	langAct->setCheckable(true);
	locales.insert(langAct, "en");

	int num = 2;
	for (int i = 0; i < fileNames.size(); ++i) {
		QTranslator translator;
		translator.load(fileNames[i], qmPath);
		
		QString language = translator.translate("LanguageHandler", "English", 
			"This should be translated to the name of the "
			"language you are translating to, in that language. "
			"Example: English = Deutsch (Deutsch is \"German\" "
			"in German)");
		
		// Checks that the mimimum requirement for accepting a string is covered.
		// The mimimum requirement is that the menu option string (English) is translated.
		if (language != "") {
			langAct = languagesMenu->addAction(QString("&%1 %2").arg(num++).arg(language));
			langAct->setCheckable(true);
			langAct->setChecked(false);
			
			QString locale = fileNames[i];
			locale = locale.mid(locale.indexOf('_') + 1);
			locale.truncate(locale.indexOf('.'));
			locales.insert(langAct, locale);
		}
	}
	
	const char *languagePref = PreferencesTool::get()->getPreference("language", "en");
	activeAction = locales.key(QString(languagePref));
	if (activeAction != 0) {
		activeAction->setChecked(true);
	}
	else {
		Logger::get().logWarning("Something wrong with the locale!");
	}
	
	if ( strcmp(languagePref, "en") != 0) {
		xmlFree((xmlChar*)languagePref);
	}
	
	return languagesMenu;
}


void LanguageHandler::changeLanguage(QAction *action)
{
	if (activeAction != 0) {
		activeAction->setChecked(false);
	}
	action->setChecked(true);
	activeAction = action;
	
	QString locale = locales[action];
	if (locale != "en") {
		translator->load("stopmotion_" + locale, qmPath);
	}
	else {
		translator->load("");
	}
	
	PreferencesTool::get()->setPreference("language", locale.toLatin1().constData());
	emit languageChanged();
}

