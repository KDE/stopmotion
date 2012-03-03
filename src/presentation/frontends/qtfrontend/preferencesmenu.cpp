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
#include "preferencesmenu.h"

#include "src/foundation/preferencestool.h"
#include "flexiblelineedit.h"


PreferencesMenu::PreferencesMenu( QWidget *parent ) 
		: QTabDialog(parent, "preferencesmenu", false, WDestructiveClose)
{
	importVideoTab = NULL;
	exportVideoTab = NULL;
	
	connectButtons();
	makeVideoImportTab();
	makeVideoExportTab();
	setCancelButton();
}


void PreferencesMenu::connectButtons()
{
	QObject::connect( this, SIGNAL(applyButtonPressed()), this, SLOT(apply()) );
}


void PreferencesMenu::makeVideoImportTab()
{
	importVideoTab = new ImportTab(this);
	importVideoTab->initializeImportValues();	
	importVideoTab->setMinimumHeight(300);
	addTab(importVideoTab, tr("Video &Import"));
}


void PreferencesMenu::makeVideoExportTab()
{
	exportVideoTab = new ExportTab(this);
	exportVideoTab->initialize();
	exportVideoTab->setMinimumHeight(300);
	addTab(exportVideoTab, tr("Video &Export"));
}


void PreferencesMenu::apply()
{
	setFocus();
	importVideoTab->apply();
	exportVideoTab->apply();
	PreferencesTool::get()->flushPreferences();
}

