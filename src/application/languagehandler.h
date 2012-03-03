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
	LanguageHandler ( QObject *parent = 0, QApplication *stApp = 0, const char *name = 0 );
	
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
