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
#ifndef LANGUAGEHANDLER_H
#define LANGUAGEHANDLER_H

#include "src/config.h"

#include <qobject.h>
#include <qstatusbar.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qtranslator.h>


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
	 * @param sb a statusbar for displaying status information to the user.
	 * @param stApp the main window
	 * @param name the name of the LanguageHandler
	 */
	LanguageHandler ( QObject *parent = 0, QStatusBar *sb = 0, QApplication *stApp = 0,
			 const char *name = 0 );
	
	/**
	 * Creates the languagesMenu.
	 *
	 * @param parent the parent menu.
	 * @return a populated language menu.
	 */
	QPopupMenu* createLanguagesMenu(QPopupMenu *parent);
	
	
private:
	QStatusBar *statusBar;
	QApplication *stApp;
	
	QTranslator *stTranslator;
	QPopupMenu *languagesMenu;
	QStringList locales;
	QString qmPath;
	
private slots:
	/**
	 * Switch to the language in location menuID in the languages menu.
	 * @param menuID the ide of the menu options with the language to
	 * switch to.
	 */
	void switchToLanguage(int menuID);
	
signals:
	/**
	 * Called when the user has changed the language.
	 */
	void languageChanged();
};

#endif
