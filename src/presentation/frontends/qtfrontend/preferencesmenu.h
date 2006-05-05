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
#ifndef PREFERENCESMENU_H
#define PREFERENCESMENU_H

#include "importtab.h"
#include "exporttab.h"

#include <QDialog>
#include <QTabWidget>


/**
 * The import and export preferences menu.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class PreferencesMenu : public QDialog 
{
	Q_OBJECT
public:
	/**
	 * Sets up the filepreview-label.
	 * @param parent the parent of the widget.
	 */
	PreferencesMenu(QWidget *parent = 0);
	
private:
	ImportTab *importVideoTab;
	ExportTab *exportVideoTab;
	QTabWidget *tabWidget;

	void makeVideoImportTab();
	void makeVideoExportTab();
	
private slots:
	void apply();
};

#endif
