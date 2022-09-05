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
#include "devicetab.h"

#include <algorithm>
#include <memory>
#include <string>

#include "flexiblelineedit.h"
#include "graphics/icons/close.xpm"
#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/logger.h"
#include "src/technical/util.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QTextEdit>


DeviceTab::DeviceTab(QWidget *parent)
	: QWidget(parent)
{
	deviceTable          = 0;
	addButton            = 0;
	removeButton         = 0;
	editButton           = 0;
	closeChangeBoxButton = 0;
	deviceEdit           = 0;
	devicePreferences    = 0;
	deviceLabel          = 0;
	checkTableItem       = 0;
	informationText      = 0;

	numAutoDetectedDevices = -1;
	makeGUI();
}


void DeviceTab::makeGUI()
{
	this->setFocusPolicy(Qt::ClickFocus);

	informationText = new QTextEdit;
	informationText->setReadOnly(true);
	informationText->setHtml(
		"<p>" + tr("Below you can set which device Stopmotion should use for grabbing images "
		"and displaying video.") + "<br><br>" +
		tr("You can select from the auto-detected devices below or add devices yourself. "
		"It is not recommended to use devices which is not auto-detected, but feel free to do "
		"it if you are an advanced user.") + "<br><br>" +
		tr("The selected device is recognized as <b>$VIDEODEVICE</b> under Video Import.") +
		"</p>");
	informationText->setMinimumWidth(440);
	informationText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QStringList lst;
	lst << tr("Name") << tr("Description");

	deviceTable = new QTableWidget;
	deviceTable->setColumnCount(2);
	deviceTable->setRowCount(0);
	deviceTable->setSelectionMode(QAbstractItemView::SingleSelection);
	deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	deviceTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	deviceTable->setHorizontalHeaderLabels(lst);
	deviceTable->verticalHeader()->setVisible(false);

	connect(deviceTable, SIGNAL(cellClicked(int, int)), this, SLOT(activeCellChanged(int, int)));
	connect(deviceTable, SIGNAL(cellChanged(int, int)), this, SLOT(contentsChanged(int, int)));

	addButton = new QPushButton(tr("&Add"));
	addButton->setFocusPolicy( Qt::NoFocus );
	connect(addButton, SIGNAL(clicked()), this, SLOT(addDevice()));

	removeButton = new QPushButton(tr("&Remove"));
	connect( removeButton, SIGNAL(clicked()), this, SLOT(removeDevice()));

	editButton = new QPushButton(tr("&Edit"));
	QObject::connect( editButton, SIGNAL(clicked()), this, SLOT(editDevice()));

	devicePreferences = new QGroupBox;
	devicePreferences->setTitle(tr("Video device settings"));
	devicePreferences->hide();

	closeChangeBoxButton = new QPushButton;
	closeChangeBoxButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	closeChangeBoxButton->setIcon(QPixmap(closeicon));
	closeChangeBoxButton->setFlat(true);
	connect(closeChangeBoxButton, SIGNAL(clicked()),this, SLOT(closeChangeBox()));

	deviceLabel = new QLabel( tr("Video Device ($VIDEODEVICE): ") );
	deviceEdit = new FlexibleLineEdit;
	connect(deviceEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updateDeviceString(const QString &)));

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(informationText);
	QVBoxLayout *buttonLayout = new QVBoxLayout;
	buttonLayout->setMargin(0);
	buttonLayout->setSpacing(2);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(addButton);
	buttonLayout->addWidget(removeButton);
	buttonLayout->addWidget(editButton);
	QHBoxLayout *deviceLayout = new QHBoxLayout;
	deviceLayout->addWidget(deviceTable);
	deviceLayout->addLayout(buttonLayout);
	mainLayout->addLayout(deviceLayout);
	mainLayout->addWidget(devicePreferences);
	setLayout(mainLayout);

	QVBoxLayout *devicePrefsLayout = new QVBoxLayout;
	QHBoxLayout *hbLayout = new QHBoxLayout;
	hbLayout->setMargin(0);
	hbLayout->setSpacing(0);
	hbLayout->addStretch(1);
	hbLayout->addWidget(closeChangeBoxButton);
	devicePrefsLayout->addLayout(hbLayout);
	devicePrefsLayout->addWidget(deviceLabel);
	devicePrefsLayout->addWidget(deviceEdit);
	devicePreferences->setLayout(devicePrefsLayout);
}


void DeviceTab::initialize() {
	Logger::get().logDebug("Initializing video device settings");
	PreferencesTool *pref = PreferencesTool::get();

	std::vector<GrabberDevice> devices = DomainFacade::getFacade()->getGrabberDevices();
	numAutoDetectedDevices = devices.size();
	deviceTable->setRowCount(numAutoDetectedDevices);

	// Auto-detected devices
	for (int i = 0; i < numAutoDetectedDevices; ++i) {
		QString name( devices[i].name.c_str() );
		QString device( devices[i].device.c_str() );
		QString desc(QString("*Autodetected* ") + devices[i].type.c_str() + "-" +
			tr("device") + " (" + device + ")");

		QTableWidgetItem *item = new QTableWidgetItem(name);
		item->setFlags( item->flags() & (~Qt::ItemIsEditable) );
		deviceTable->setItem(i, 0,  item);

		item = new QTableWidgetItem(desc);
		item->setFlags( item->flags() & (~Qt::ItemIsEditable) );
		deviceTable->setItem(i, 1, item);

		deviceNameStrings.push_back(name);
		deviceStrings.push_back(device);
		deviceDescriptionStrings.push_back(desc);
	}

	// Userdefined devices
	int active = pref->getPreference("activeVideoDevice", -1);
	int numDevices = pref->getPreference("numDevices", 0);
	int numUserDevices = 0;
	int newActive = -1;
	std::string oldDevice;
	std::string oldName;
	bool oldWasAutoDetected = false;

	for (int i = 0; i < numDevices; ++i) {
		Preference descP(QString("deviceDescription%1").arg(i).toUtf8().constData(),"");
		QString desc(descP.get());
		Preference nameP(QString("deviceName%1").arg(i).toUtf8().constData(),"");
		QString name(nameP.get());
		Preference deviceP(QString("device%1").arg(i).toUtf8().constData(),"");
		QString device(deviceP.get());
		if (active == i) {
			oldDevice = device.toStdString();
			oldName = name.toStdString();
		}

		if ( !desc.startsWith("*Autodetected*") ) {
			if (active == i)
				newActive = numUserDevices + numAutoDetectedDevices;
			deviceTable->setRowCount( numUserDevices + numAutoDetectedDevices + 1 );
			deviceTable->setItem(numUserDevices + numAutoDetectedDevices,
					0, new QTableWidgetItem(name));
			deviceTable->setItem(numUserDevices + numAutoDetectedDevices,
					1, new QTableWidgetItem(desc));
			++numUserDevices;

			deviceNameStrings.push_back(name);
			deviceStrings.push_back(device);
			deviceDescriptionStrings.push_back(desc);
		} else if (active == i) {
			oldWasAutoDetected = true;
		}
	}

	if (oldWasAutoDetected && 0 < numAutoDetectedDevices) {
		// try to find a reasonable match for the last chosen auto detected
		// device
		int bestScore = 0;
		newActive = numAutoDetectedDevices - 1;
		for (int i = 0; i != numAutoDetectedDevices; ++i) {
			int score = 0;
			if (oldName == devices[i].name)
				score = 2;
			if (oldDevice == devices[i].device)
				score += 1;
			if (bestScore < score) {
				newActive = i;
			}
		}
	}
	if (0 <= newActive)
		deviceTable->setCurrentCell(newActive, 0);
}


void DeviceTab::apply() {
	PreferencesTool *prefs = PreferencesTool::get();

	// Remove old preferences
	int numDevices = prefs->getPreference("numDevices", -1);
 	if (numDevices > 0) {
		for (int i = 0; i < numDevices; ++i) {
			prefs->removePreference(QString("deviceName%1").arg(i).toUtf8().constData());
			prefs->removePreference(QString("deviceDescription%1").arg(i).toUtf8().constData());
			prefs->removePreference(QString("device%1").arg(i).toUtf8().constData());
		}
	}

	// Set new preferences
	numDevices = deviceTable->rowCount();
	if (numDevices > 0) {
		prefs->setPreference("numDevices", numDevices);
		prefs->setPreference("activeVideoDevice", deviceTable->currentRow());
		for (int i = 0; i < numDevices; ++i) {
			prefs->setPreference(QString("deviceName%1").arg(i).toUtf8().constData(),
					deviceTable->item(i, 0)->text().toUtf8().constData());
			prefs->setPreference(QString("deviceDescription%1").arg(i).toUtf8().constData(),
					deviceTable->item(i, 1)->text().toUtf8().constData());
			prefs->setPreference(QString("device%1").arg(i).toUtf8().constData(),
					deviceStrings[i].toUtf8().constData());
		}
	} else {
		prefs->setPreference("numDevices", -1);
		prefs->setPreference("activeVideoDevice", -1);
	}
}


void DeviceTab::resizeEvent(QResizeEvent *event)
{
	contentsChanged(0, 0);
	QWidget::resizeEvent(event);
}


void DeviceTab::contentsChanged(int, int)
{
	deviceTable->resizeColumnsToContents();
	int totalWidth = deviceTable->columnWidth(0) + deviceTable->columnWidth(1);
	int tableWidth = deviceTable->width() - 5;
	if ( totalWidth < tableWidth) {
		deviceTable->setColumnWidth( 1, tableWidth - deviceTable->columnWidth(0) );
	}
}


void DeviceTab::activeCellChanged(int, int)
{
	if ( devicePreferences->isVisible() ) {
		editDevice();
	}
}


void DeviceTab::editDevice()
{
	int selected = deviceTable->currentRow();
	if (selected >= 0) {
		deviceEdit->setText(deviceStrings[selected]);
		devicePreferences->show();

		// Disables editing of autodetected devices
		if (selected < numAutoDetectedDevices) {
			deviceEdit->setReadOnly(true);
		}
		else {
			deviceEdit->setReadOnly(false);
		}
	}
}


void DeviceTab::closeChangeBox()
{
	devicePreferences->hide();
	this->resize(minimumSize());
}


void DeviceTab::updateDeviceString(const QString &txt)
{
	deviceStrings[deviceTable->currentRow()] = txt;
}


void DeviceTab::addDevice()
{
	int newRow = deviceTable->rowCount();
	deviceTable->setRowCount(newRow + 1);
	deviceTable->setItem( newRow, 0, new QTableWidgetItem(QString("")) );
	deviceTable->setItem( newRow, 1, new QTableWidgetItem(QString("")) );
	deviceTable->setCurrentCell(newRow, 0);

	deviceStrings.push_back("");
	deviceNameStrings.push_back("");
	deviceDescriptionStrings.push_back("");
}


void DeviceTab::removeDevice()
{
	int selectedRow = deviceTable->currentRow();
	if (selectedRow >= 0 && selectedRow >= numAutoDetectedDevices) {
		deviceStrings.erase(deviceStrings.begin() + selectedRow);
		deviceNameStrings.erase(deviceNameStrings.begin() + selectedRow);
		deviceDescriptionStrings.erase(deviceDescriptionStrings.begin() + selectedRow);
		deviceTable->removeRow(selectedRow);
		contentsChanged(0, 0);
	}
}


void DeviceTab::retranslateStrings()
{
	informationText->setHtml(
		"<p>" + tr("Below you can set which device Stopmotion should use for grabbing images "
		"and displaying video.") + "<br><br>" +
		tr("You can select from the auto-detected devices below or add devices yourself. "
		"It is not recommended to use devices which is not auto-detected, but feel free to do "
		"it if you are an advanced user.") + "<br><br>" +
		tr("The selected device is recognized as <b>$VIDEODEVICE</b> under Video Import.") +
		"</p>");

	QStringList lst;
	lst << tr("Name") << tr("Description");
	deviceTable->setHorizontalHeaderLabels(lst);

	addButton->setText( tr("&Add") );
	removeButton->setText( tr("&Remove") );
	editButton->setText( tr("&Edit") );


	devicePreferences->setTitle(tr("Video device settings"));
	deviceLabel->setText( tr("Video Device ($VIDEODEVICE): ") );
}
