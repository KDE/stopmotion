/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "languagehandler.h"

#include "logger.h"
#include "src/foundation/preferencestool.h"
#include "src/config.h"

#include <cassert>
#include <QDebug>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QLatin1String>
#include <QString>


LanguageHandler::LanguageHandler(QObject *parent, QApplication *stApp, const char *name) 
		: QObject(parent), languagesMenu(nullptr) {
	qmPath = QString(stopmotion::translationsDirectory);
	activeAction = nullptr;
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
	    if (!translator->load(translationFile)) {
		    Logger::get().logWarning("Failed to load translation %s",
				    translationFile.toLocal8Bit().constData());
	    }
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
	connect(languagesMenu, &QMenu::triggered, this, &LanguageHandler::changeLanguage);

	QDir dir(qmPath);
	QStringList fileNames = dir.entryList(QStringList("stopmotion_*.qm"));

	//English is a special case (base language)
	QAction *langAct = languagesMenu->addAction("&1 English");
	langAct->setCheckable(true);
	locales.insert(langAct, "en");

	int num = 2;
	for (int i = 0; i < fileNames.size(); ++i) {
		QTranslator translator;
		if (!translator.load(fileNames[i], qmPath)) {
			Logger::get().logWarning("Failed to load translation %s",
					fileNames[i].toLocal8Bit().constData());
			continue;
		}

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
	if (activeAction != nullptr) {
		activeAction->setChecked(true);
	} else {
		Logger::get().logWarning("Something wrong with the locale!");
	}
	return languagesMenu;
}


void LanguageHandler::changeLanguage(QAction *action)
{
	if (activeAction != nullptr) {
		activeAction->setChecked(false);
	}
	action->setChecked(true);
	activeAction = action;
	
	QString locale = locales[action];
	if (locale != "en") {
		if (!translator->load("stopmotion_" + locale, qmPath)) {
			Logger::get().logWarning("Failed to load translation for %s",
					locale.toLocal8Bit().constData());
		}
	}
	else {
		// Loading an empty file name unloads the current translation, which
		// is all we want for English. It always reports failure.
		static_cast<void>(translator->load(QString()));
	}
	
	PreferencesTool::get()->setPreference("language", locale.toLatin1().constData());
	emit languageChanged();
}

