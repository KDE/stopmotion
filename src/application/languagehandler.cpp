/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "languagehandler.h"

#include "stopmotion_debug.h"
#include "src/foundation/preferencestool.h"
#include "src/config.h"

#include <cassert>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QLatin1String>
#include <QString>


namespace {
// The name of the language in that language, like "Deutsch" for de
QString languageName(const QString& code) {
	const QString language = code.section(u'@', 0, 0);
	const QString variant = code.section(u'@', 1);
	const QLocale locale(language);
	QString name = locale.nativeLanguageName();
	if (name.isEmpty())
		name = code;
	else if (language.contains(u'_')
			&& name == QLocale(locale.language()).nativeLanguageName())
		name += QStringLiteral(" (%1)").arg(locale.nativeTerritoryName());
	if (!variant.isEmpty())
		name += QStringLiteral(" (%1)").arg(variant);
	return name;
}
}

LanguageHandler::LanguageHandler(QObject *parent, QApplication *stApp, const char *name) 
		: QObject(parent), languagesMenu(nullptr) {
	activeAction = nullptr;
	translator = new QTranslator(this);
	findTranslations();

	// Use the language of the system if there is a translation for it, or
	// the one saved in the preferences file, or English as fall-back.
	QString language = systemLanguage();
	if (language.isEmpty()) {
		Preference languagePref("language", "en");
		language = QString::fromLatin1(languagePref.get());
		if (!translations.contains(language))
			language = QStringLiteral("en");
	} else {
		PreferencesTool::get()->setPreference("language", language.toLatin1().constData());
	}

	if (language != QLatin1String("en")) {
	    qCDebug(STOPMOTION_LOG, "Loading translator: ");
	    qCDebug(STOPMOTION_LOG, translations.value(language).toLocal8Bit().constData());
	    load(language);
	}

	stApp->installTranslator(translator);
	setObjectName(name);
}

void LanguageHandler::findTranslations() {
	// In a build directory, the executable is in bin/ and the translations
	// are next to it. They take precedence over installed ones.
	const QString buildDir = QCoreApplication::applicationDirPath()
			+ QStringLiteral(u"/..");
	// Stopmotion's own translations, named stopmotion_<language>.qm
	const QString prefix = QStringLiteral("stopmotion_");
	const QStringList dirs = { buildDir,
			QString::fromLocal8Bit(stopmotion::translationsDirectory) };
	for (const QString& d : dirs) {
		const QDir dir(d);
		const QStringList fileNames = dir.entryList(QStringList(prefix + u"*.qm"), QDir::Files);
		for (const QString& fileName : fileNames) {
			const QString code = fileName.mid(prefix.size()).chopped(3);
			if (!translations.contains(code))
				translations.insert(code, dir.filePath(fileName));
		}
	}
	// KDE's translations, for the other languages, in <language>/<subdir>
	const QList<std::pair<QString, QString>> localeDirs = {
			{ buildDir + QStringLiteral(u"/ECMPoQm"), QString() },
			{ QString::fromLocal8Bit(stopmotion::localeDirectory),
					QStringLiteral(u"/LC_MESSAGES") } };
	for (const auto& [d, subdir] : localeDirs) {
		const QDir localeDir(d);
		const QStringList languages = localeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (const QString& language : languages) {
			const QString file = localeDir.filePath(language + subdir
					+ QStringLiteral(u"/stopmotion_qt.qm"));
			if (!translations.contains(language) && QFile::exists(file))
				translations.insert(language, file);
		}
	}
}

/**
 * Returns the first language of the system with a translation, "en" if
 * English comes first, or an empty string.
 */
QString LanguageHandler::systemLanguage() const {
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString& uiLanguage : uiLanguages) {
		// e.g. pt-Latn-BR becomes pt_BR
		const QString language = QLocale(uiLanguage).name();
		if (translations.contains(language))
			return language;
		const QString base = language.section(u'_', 0, 0);
		if (translations.contains(base))
			return base;
		if (base == QLatin1String("en"))
			return base;
	}
	return QString();
}

bool LanguageHandler::load(const QString& language) {
	const QString file = translations.value(language);
	if (file.isEmpty() || !translator->load(file)) {
		qCWarning(STOPMOTION_LOG, "Failed to load translation for %s",
				language.toLocal8Bit().constData());
		return false;
	}
	return true;
}


QMenu* LanguageHandler::createLanguagesMenu(QMenu *parent)
{
	assert(parent);
	languagesMenu = parent->addMenu(tr("&Translation"));
	connect(languagesMenu, &QMenu::triggered, this, &LanguageHandler::changeLanguage);

	//English is a special case (base language)
	QAction *langAct = languagesMenu->addAction(QStringLiteral("&1 English"));
	langAct->setCheckable(true);
	locales.insert(langAct, QStringLiteral("en"));

	int num = 2;
	for (auto it = translations.cbegin(); it != translations.cend(); ++it) {
		langAct = languagesMenu->addAction(QStringLiteral("&%1 %2").arg(num++).arg(languageName(it.key())));
		langAct->setCheckable(true);
		langAct->setChecked(false);
		locales.insert(langAct, it.key());
	}

	Preference languagePref("language", "en");
	activeAction = locales.key(QString::fromLatin1(languagePref.get()));
	if (activeAction != nullptr) {
		activeAction->setChecked(true);
	} else {
		qCWarning(STOPMOTION_LOG, "Something wrong with the locale!");
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
	if (locale != QLatin1String("en")) {
		load(locale);
	}
	else {
		// Loading an empty file name unloads the current translation, which
		// is all we want for English. It always reports failure.
		static_cast<void>(translator->load(QString()));
	}
	
	PreferencesTool::get()->setPreference("language", locale.toLatin1().constData());
	Q_EMIT languageChanged();
}

