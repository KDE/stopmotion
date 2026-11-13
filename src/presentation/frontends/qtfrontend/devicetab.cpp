/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "devicetab.h"

#include <algorithm>
#include <memory>
#include <string>

#include "flexiblelineedit.h"
#include "graphics/icons/close.xpm"
#include "src/domain/domainfacade.h"
#include "src/foundation/preferencestool.h"
#include "stopmotion_debug.h"
#include "src/technical/util.h"

#include <QSignalBlocker>
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
	deviceTable          = nullptr;
	addButton            = nullptr;
	removeButton         = nullptr;
	editButton           = nullptr;
	closeChangeBoxButton = nullptr;
	deviceEdit           = nullptr;
	devicePreferences    = nullptr;
	deviceLabel          = nullptr;
	checkTableItem       = nullptr;
	informationText      = nullptr;

	numAutoDetectedDevices = -1;
	makeGUI();
}


void DeviceTab::makeGUI()
{
	this->setFocusPolicy(Qt::ClickFocus);

	informationText = new QTextEdit;
	informationText->setReadOnly(true);
	informationText->setHtml(
		QStringLiteral("<p>") + tr("Below you can set which device Stopmotion should use for grabbing images "
		"and displaying video.") + QStringLiteral("<br><br>") +
		tr("You can select from the auto-detected devices below or add devices yourself. "
		"It is not recommended to use devices which is not auto-detected, but feel free to do "
		"it if you are an advanced user.") + QStringLiteral("<br><br>") +
		tr("The selected device is recognized as <b>$VIDEODEVICE</b> under Video Import.") +
		QStringLiteral("</p>"));
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

	connect(deviceTable, &QTableWidget::currentCellChanged, this, &DeviceTab::activeCellChanged);
	connect(deviceTable, &QTableWidget::cellChanged, this, &DeviceTab::contentsChanged);

	addButton = new QPushButton(tr("&Add"));
	addButton->setFocusPolicy( Qt::NoFocus );
	connect(addButton, &QAbstractButton::clicked, this, &DeviceTab::addDevice);

	removeButton = new QPushButton(tr("&Remove"));
	connect( removeButton, &QAbstractButton::clicked, this, &DeviceTab::removeDevice);

	editButton = new QPushButton(tr("&Edit"));
	QObject::connect( editButton, &QAbstractButton::clicked, this, &DeviceTab::editDevice);

	devicePreferences = new QGroupBox;
	devicePreferences->setTitle(tr("Video device settings"));
	devicePreferences->hide();

	closeChangeBoxButton = new QPushButton;
	closeChangeBoxButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	closeChangeBoxButton->setIcon(QPixmap(closeicon));
	closeChangeBoxButton->setFlat(true);
	connect(closeChangeBoxButton, &QAbstractButton::clicked,this, &DeviceTab::closeChangeBox);

	deviceLabel = new QLabel( tr("Video Device ($VIDEODEVICE): ") );
	deviceEdit = new FlexibleLineEdit;
	connect(deviceEdit, &QLineEdit::textChanged,
			this, &DeviceTab::updateDeviceString);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(informationText);
	QVBoxLayout *buttonLayout = new QVBoxLayout;
	buttonLayout->setContentsMargins(0, 0, 0, 0);
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
	hbLayout->setContentsMargins(0, 0, 0, 0);
	hbLayout->setSpacing(0);
	hbLayout->addStretch(1);
	hbLayout->addWidget(closeChangeBoxButton);
	devicePrefsLayout->addLayout(hbLayout);
	devicePrefsLayout->addWidget(deviceLabel);
	devicePrefsLayout->addWidget(deviceEdit);
	devicePreferences->setLayout(devicePrefsLayout);
}


void DeviceTab::initialize() {
	qCDebug(STOPMOTION_LOG, "Initializing video device settings");
	PreferencesTool *pref = PreferencesTool::get();

	std::vector<GrabberDevice> devices = DomainFacade::getFacade()->getGrabberDevices();
	numAutoDetectedDevices = devices.size();
	deviceTable->setRowCount(numAutoDetectedDevices);

	// Auto-detected devices
	for (int i = 0; i < numAutoDetectedDevices; ++i) {
		QString name = QString::fromStdString(devices[i].name);
		QString device = QString::fromStdString(devices[i].device);
		QString desc(QStringLiteral("*Autodetected* ") + QString::fromStdString(devices[i].type) + QStringLiteral("-") +
			tr("device") + QStringLiteral(" (") + device + QStringLiteral(")"));

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
		Preference descP(QStringLiteral("deviceDescription%1").arg(i).toUtf8().constData(),"");
		QString desc = QString::fromUtf8(descP.get());
		Preference nameP(QStringLiteral("deviceName%1").arg(i).toUtf8().constData(),"");
		QString name = QString::fromUtf8(nameP.get());
		Preference deviceP(QStringLiteral("device%1").arg(i).toUtf8().constData(),"");
		QString device = QString::fromUtf8(deviceP.get());
		if (active == i) {
			oldDevice = device.toStdString();
			oldName = name.toStdString();
		}

		if ( !desc.startsWith(QLatin1String("*Autodetected*")) ) {
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
			prefs->removePreference(QStringLiteral("deviceName%1").arg(i).toUtf8().constData());
			prefs->removePreference(QStringLiteral("deviceDescription%1").arg(i).toUtf8().constData());
			prefs->removePreference(QStringLiteral("device%1").arg(i).toUtf8().constData());
		}
	}

	// Set new preferences
	numDevices = deviceTable->rowCount();
	if (numDevices > 0) {
		prefs->setPreference("numDevices", numDevices);
		prefs->setPreference("activeVideoDevice", deviceTable->currentRow());
		for (int i = 0; i < numDevices; ++i) {
			prefs->setPreference(QStringLiteral("deviceName%1").arg(i).toUtf8().constData(),
					deviceTable->item(i, 0)->text().toUtf8().constData());
			prefs->setPreference(QStringLiteral("deviceDescription%1").arg(i).toUtf8().constData(),
					deviceTable->item(i, 1)->text().toUtf8().constData());
			prefs->setPreference(QStringLiteral("device%1").arg(i).toUtf8().constData(),
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
	if (selected < 0) {
		closeChangeBox();
		return;
	}
	if (static_cast<int>(deviceStrings.size()) <= selected)
		return;
	editedRow = selected;
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


void DeviceTab::closeChangeBox()
{
	editedRow = -1;
	devicePreferences->hide();
	this->resize(minimumSize());
}


void DeviceTab::updateDeviceString(const QString &txt)
{
	if (0 <= editedRow && editedRow < static_cast<int>(deviceStrings.size()))
		deviceStrings[editedRow] = txt;
}


void DeviceTab::addDevice()
{
	int newRow = deviceTable->rowCount();
	deviceTable->setRowCount(newRow + 1);
	deviceTable->setItem( newRow, 0, new QTableWidgetItem(QString()) );
	deviceTable->setItem( newRow, 1, new QTableWidgetItem(QString()) );
	deviceTable->setCurrentCell(newRow, 0);

	deviceStrings.push_back(QString());
	deviceNameStrings.push_back(QString());
	deviceDescriptionStrings.push_back(QString());
}


void DeviceTab::removeDevice()
{
	int selectedRow = deviceTable->currentRow();
	if (selectedRow >= 0 && selectedRow >= numAutoDetectedDevices) {
		deviceStrings.erase(deviceStrings.begin() + selectedRow);
		deviceNameStrings.erase(deviceNameStrings.begin() + selectedRow);
		deviceDescriptionStrings.erase(deviceDescriptionStrings.begin() + selectedRow);
		{
			// the vectors are shorter already
			QSignalBlocker blocker(deviceTable);
			deviceTable->removeRow(selectedRow);
		}
		contentsChanged(0, 0);
		activeCellChanged(0, 0);
	}
}


void DeviceTab::retranslateStrings()
{
	informationText->setHtml(
		QStringLiteral("<p>") + tr("Below you can set which device Stopmotion should use for grabbing images "
		"and displaying video.") + QStringLiteral("<br><br>") +
		tr("You can select from the auto-detected devices below or add devices yourself. "
		"It is not recommended to use devices which is not auto-detected, but feel free to do "
		"it if you are an advanced user.") + QStringLiteral("<br><br>") +
		tr("The selected device is recognized as <b>$VIDEODEVICE</b> under Video Import.") +
		QStringLiteral("</p>"));

	QStringList lst;
	lst << tr("Name") << tr("Description");
	deviceTable->setHorizontalHeaderLabels(lst);

	addButton->setText( tr("&Add") );
	removeButton->setText( tr("&Remove") );
	editButton->setText( tr("&Edit") );


	devicePreferences->setTitle(tr("Video device settings"));
	deviceLabel->setText( tr("Video Device ($VIDEODEVICE): ") );
}
