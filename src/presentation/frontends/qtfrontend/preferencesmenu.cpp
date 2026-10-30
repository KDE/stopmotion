/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
	importVideoTab = nullptr;
	exportVideoTab = nullptr;
	videoDeviceTab = nullptr;
	tabWidget = new QTabWidget;

	applyButton = new QPushButton(tr("Apply"), this);
	applyButton->setDefault(true);
	connect(applyButton, &QAbstractButton::clicked, this, &PreferencesMenu::apply);

	closeButton = new QPushButton(tr("Close"), this);
	closeButton->setDefault(true);
	connect(closeButton, &QAbstractButton::clicked, this, &QWidget::close);

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

