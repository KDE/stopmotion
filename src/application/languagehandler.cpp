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
#include "languagehandler.h"

#include "logger.h"
#include "src/foundation/preferencestool.h"
#include "src/config.h"

#include <assert.h>
#include <QDebug>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QLatin1String>
#include <QString>


LanguageHandler::LanguageHandler(QObject *parent, QApplication *stApp, const char *name) 
		: QObject(parent), languagesMenu(0) {
	qmPath = QString(stopmotion::translationsDirectory);
	activeAction = 0;
	translator = new QTranslator(this);

	// Get system locale.
	QString locale = QLocale::system().name().toLower();
	if (locale == QLatin1String("nb_no"))
	    locale = QStringLiteral(u"no_nb");
	else if (locale == QLatin1String("nn_no"))
	    locale = QStringLiteral(u"no_nn");
	else if (locale == QLatin1String("se_no"))
	    locale = QStringLiteral(u"no_se");
	else
	    locale.truncate(2);

	// Put together a translation file based on the qmPath or keep
	// it empty if the locale is english.
	const bool englishLocale = (locale == QLatin1String("en"));
	const QString prefix = qmPath + u'/' + QStringLiteral(u"stopmotion_");
	QString translationFile = englishLocale ? QString() : prefix + locale;

	if (!englishLocale && !QFile::exists(translationFile + QStringLiteral(u".qm"))) {
		// Was not able to find a translation file for the locale, so use the
		// language saved in the preferences file, or use English as fall-back.
		const QByteArray localeArray = locale.toLatin1();
		const char *localePtr = localeArray.constData();
		Preference languagePref("language", localePtr);
		if (languagePref.get()) {
			translationFile = prefix + QLatin1String(languagePref.get());
			if (!QFile::exists(translationFile + QStringLiteral(u".qm")))
				translationFile = QString();
	    } else {
	    	translationFile = QString();
	    }
	} else {
	    PreferencesTool::get()->setPreference("language", locale.toLatin1().constData());
	}

	if (!translationFile.isEmpty()) {
	    Logger::get().logDebug("Loading translator: ");
	    Logger::get().logDebug(translationFile.toLatin1().constData());
	    translator->load(translationFile);
	}

	stApp->installTranslator(translator);
	setObjectName(name);
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

		// Checks that the minimum requirement for accepting a string is covered.
		// The minimum requirement is that the menu option string (English) is translated.
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

	Preference languagePref("language", "en");
	activeAction = locales.key(QString(languagePref.get()));
	if (activeAction != 0) {
		activeAction->setChecked(true);
	} else {
		Logger::get().logWarning("Something wrong with the locale!");
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

