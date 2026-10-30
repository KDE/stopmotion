/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef LANGUAGEHANDLER_H
#define LANGUAGEHANDLER_H

#include "src/config.h"

#include <QObject>
#include <QApplication>
#include <QMenu>
#include <QTranslator>
#include <QHash>
#include <QAction>


/**
 * This class handles the loading of the different languages which are supported
 * by the program.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class LanguageHandler : public QObject
{
	Q_OBJECT
public:
	/**
	 * Creates and initializes the LanguageHandler.
	 * @param parent the parent of the LanguageHandler.
	 * @param stApp the main window
	 * @param name the name of the LanguageHandler
	 */
	LanguageHandler ( QObject *parent = nullptr, QApplication *stApp = nullptr, const char *name = nullptr );
	
	/**
	 * Creates the languagesMenu.
	 * @param parent the parent menu.
	 * @return a populated language menu.
	 */
	QMenu* createLanguagesMenu(QMenu *parent);
	
private:
	QTranslator *translator;
	QMenu *languagesMenu;
	QString qmPath;
	QAction *activeAction;
	QHash<QAction*, QString> locales;
	
private slots:
	/**
	 * Switch to the language in location menuID in the languages menu.
	 * @param menuID the ide of the menu options with the language to
	 * switch to.
	 */
	void changeLanguage(QAction *activeAction);
	
signals:
	/**
	 * Called when the user has changed the language.
	 */
	void languageChanged();
};

#endif
