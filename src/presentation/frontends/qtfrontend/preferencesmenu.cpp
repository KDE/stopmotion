/***************************************************************************
 *   Copyright (C) 2005-2017 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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

#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "devicetab.h"
#include "exporttab.h"
#include "importtab.h"
#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/uiexception.h"
#include "src/presentation/frontends/frontend.h"


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
	setMaximumWidth(800);
	setMinimumWidth(620);
	setModal(false);

	makeVideoDeviceTab();
	makeVideoImportTab();
	makeVideoExportTab();
}


void PreferencesMenu::makeVideoImportTab()
{
	importVideoTab = new ImportTab;
	importVideoTab->initializeImportValues();	
	importVideoTab->setMinimumHeight(400);
	tabWidget->addTab(importVideoTab, tr("Video &Import"));
}


void PreferencesMenu::makeVideoExportTab()
{
	exportVideoTab = new ExportTab;
	exportVideoTab->initialize();
	exportVideoTab->setMinimumHeight(460);
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
	try {
		PreferencesTool::get()->flush();
	} catch (UiException& ex) {
		DomainFacade::getFacade()->getFrontend()->handleException(ex);
	}
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

