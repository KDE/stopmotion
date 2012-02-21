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
#include "preferencesmenu.h"

#include "src/foundation/preferencestool.h"
#include "flexiblelineedit.h"

#include <QVBoxLayout>


PreferencesMenu::PreferencesMenu(QWidget *parent) 
		: QDialog(parent, Qt::Dialog)
{
	importVideoTab = 0;
	exportVideoTab = 0;
	videoDeviceTab = 0;
	tabWidget = new QTabWidget;
	
	applyButton = new QPushButton(tr("Apply"), this);
	applyButton->setDefault(true);
	connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));

	closeButton = new QPushButton(tr("Close"), this);
	closeButton->setDefault(true);
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(applyButton);
	buttonLayout->addWidget(closeButton);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addLayout(buttonLayout);
	
	setLayout(mainLayout);
	setWindowTitle(tr("Preferences Menu"));
	setMaximumWidth(600);
	setMinimumWidth(600);
	setModal(false);
	
	makeVideoDeviceTab();
	makeVideoImportTab();
	makeVideoExportTab();
}


void PreferencesMenu::makeVideoImportTab()
{
	importVideoTab = new ImportTab;
	importVideoTab->initializeImportValues();	
	importVideoTab->setMinimumHeight(300);
	tabWidget->addTab(importVideoTab, tr("Video &Import"));
}


void PreferencesMenu::makeVideoExportTab()
{
	exportVideoTab = new ExportTab;
	exportVideoTab->initialize();
	exportVideoTab->setMinimumHeight(300);
	tabWidget->addTab(exportVideoTab, tr("Video &Export"));
}


void PreferencesMenu::makeVideoDeviceTab()
{
	videoDeviceTab = new DeviceTab;
	videoDeviceTab->initialize();
	videoDeviceTab->setMinimumHeight(300);
	tabWidget->addTab(videoDeviceTab, tr("Video &Device"));
}


void PreferencesMenu::apply()
{
	setFocus();
	videoDeviceTab->apply();
	importVideoTab->apply();
	exportVideoTab->apply();
	PreferencesTool::get()->flushPreferences();
}


void PreferencesMenu::display()
{
	videoDeviceTab->initialize();
	this->show();
}
	

void PreferencesMenu::retranslateStrings()
{
	applyButton->setText(tr("Apply"));
	closeButton->setText(tr("Close"));
	setWindowTitle(tr("Preferences Menu"));
	
	tabWidget->setTabText(0, tr("Video &Device"));
	tabWidget->setTabText(1, tr("Video &Import"));
	tabWidget->setTabText(2, tr("Video &Export"));
	
	importVideoTab->retranslateStrings();
	exportVideoTab->retranslateStrings();
	videoDeviceTab->retranslateStrings();
}

