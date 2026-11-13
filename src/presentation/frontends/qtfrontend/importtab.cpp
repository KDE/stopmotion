/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "importtab.h"

#include "flexiblelineedit.h"
#include "preferencestool.h"
#include "graphics/icons/close.xpm"

#include <algorithm>
#include <memory>
#include <QSignalBlocker>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QHeaderView>

class QResizeEvent;

ImportTab::ImportTab( QWidget *parent ) : QWidget(parent)
{
	deviceSelectionTable = nullptr;
	addButton            = nullptr;
	removeButton         = nullptr;
	changeButton         = nullptr;
	closeChangeBoxButton = nullptr;
	prePollEdit          = nullptr;
	startDaemonEdit      = nullptr;
	stopDaemonEdit       = nullptr;
	grabberPreferences   = nullptr;
	prePollLabel         = nullptr;
	startDaemonLabel     = nullptr;
	stopDaemonLabel      = nullptr;
	checkTableItem       = nullptr;
	informationText      = nullptr;

	makeGUI();
}


void ImportTab::makeGUI()
{
	this->setFocusPolicy(Qt::ClickFocus);

	informationText = new QTextEdit;
	informationText->setReadOnly(true);
	informationText->setHtml(
		QStringLiteral("<p>") + tr("Below you can set which program/process Stopmotion should use "
		"for grabbing images from the selected device.") + QStringLiteral("</p><p>") +
		tr("You should always use <b>$VIDEODEVICE</b> and <b>$IMAGEFILE</b> to represent "
		"the video device and the image file, respectively.") + QStringLiteral("</p>"));
	informationText->setMinimumWidth(440);
	informationText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QStringList lst;
	lst << tr("Name") << tr("Description");

	deviceSelectionTable = new QTableWidget;
	deviceSelectionTable->setColumnCount(2);
	deviceSelectionTable->setRowCount(0);
	deviceSelectionTable->setSelectionMode(QAbstractItemView::SingleSelection);
	deviceSelectionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	deviceSelectionTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	deviceSelectionTable->setHorizontalHeaderLabels(lst);
	deviceSelectionTable->verticalHeader()->setVisible(false);

	connect(deviceSelectionTable, &QTableWidget::currentCellChanged,
			this, &ImportTab::activeCellChanged);
	connect(deviceSelectionTable, &QTableWidget::cellChanged,
			this, &ImportTab::contentsChanged);

	addButton = new QPushButton(tr("&Add"));
	addButton->setFocusPolicy( Qt::NoFocus );
	QObject::connect(addButton, &QAbstractButton::clicked, this, &ImportTab::addImportProgram);

	removeButton = new QPushButton(tr("&Remove"));
	QObject::connect( removeButton, &QAbstractButton::clicked, this, &ImportTab::removeImportProgram);

	changeButton = new QPushButton(tr("&Edit"));
	QObject::connect( changeButton, &QAbstractButton::clicked, this, &ImportTab::changeSettings);

	grabberPreferences = new QGroupBox;
	grabberPreferences->setTitle(tr("Import device settings"));
	grabberPreferences->hide();

	closeChangeBoxButton = new QPushButton;
	closeChangeBoxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	closeChangeBoxButton->setIcon(QPixmap(closeicon));
	closeChangeBoxButton->setFlat(true);
	QObject::connect( closeChangeBoxButton, &QAbstractButton::clicked,this, &ImportTab::closeChangeBox);

	prePollLabel = new QLabel( tr("Pre-poll command") );
	prePollEdit = new FlexibleLineEdit;
	prePollLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	prePollEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QObject::connect( prePollEdit, &QLineEdit::textChanged,
			this, &ImportTab::updatePrePollString);

	startDaemonLabel = new QLabel( tr("Start daemon") );
	startDaemonEdit = new FlexibleLineEdit;
	startDaemonLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	startDaemonEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QObject::connect( startDaemonEdit, &QLineEdit::textChanged,
			this, &ImportTab::updateStartDaemonString);

	stopDaemonLabel = new QLabel( tr("Stop daemon") );
	stopDaemonEdit = new FlexibleLineEdit;
	stopDaemonLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	stopDaemonEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QObject::connect( stopDaemonEdit, &QLineEdit::textChanged,
			this, &ImportTab::updatestopDaemonString );

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(informationText);
	QVBoxLayout *buttonLayout = new QVBoxLayout;
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	buttonLayout->setSpacing(2);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(addButton);
	buttonLayout->addWidget(removeButton);
	buttonLayout->addWidget(changeButton);
	QHBoxLayout *deviceLayout = new QHBoxLayout;
	deviceLayout->addWidget(deviceSelectionTable);
	deviceLayout->addLayout(buttonLayout);
	mainLayout->addLayout(deviceLayout);
	mainLayout->addWidget(grabberPreferences);
	setLayout(mainLayout);

	QGridLayout *grabberPrefsLayout = new QGridLayout;
	grabberPrefsLayout->addWidget(closeChangeBoxButton, 0, 2, Qt::AlignLeading);
	grabberPrefsLayout->addWidget(prePollLabel, 0, 0, Qt::AlignTrailing);
	grabberPrefsLayout->addWidget(prePollEdit, 0, 1);
	grabberPrefsLayout->addWidget(startDaemonLabel, 1, 0, Qt::AlignLeading);
	grabberPrefsLayout->addWidget(startDaemonEdit, 1, 1, 1, 2);
	grabberPrefsLayout->addWidget(stopDaemonLabel, 2, 0, Qt::AlignLeading);
	grabberPrefsLayout->addWidget(stopDaemonEdit, 2, 1, 1, 2);
	grabberPrefsLayout->setColumnStretch(1, 1);
	grabberPreferences->setLayout(grabberPrefsLayout);
}


void ImportTab::initializeImportValues()
{
	PreferencesTool* pref = PreferencesTool::get();

	int numImports = pref->getPreference("numberofimports", 0);
	deviceSelectionTable->setRowCount(numImports);
	for (int i = 0; i < numImports; ++i) {
		Preference name(QStringLiteral("importname%1").arg(i).toUtf8().constData(), "");
		deviceSelectionTable->setItem( i, 0, new QTableWidgetItem(QString::fromUtf8(name.get())) );
		Preference desc(QStringLiteral("importdescription%1").arg(i).toUtf8().constData(), "");
		deviceSelectionTable->setItem( i, 1, new QTableWidgetItem(QString::fromUtf8(desc.get())) );

		Preference prepoll(QStringLiteral("importprepoll%1").arg(i).toUtf8().constData(), "");
		prePollStrings.push_back(QString::fromUtf8(prepoll.get()));

		Preference start(QStringLiteral("importstartdaemon%1").arg(i).toUtf8().constData(), "");
		startDaemonStrings.push_back(QString::fromUtf8(start.get()));

		Preference stop(QStringLiteral("importstopdaemon%1").arg(i).toUtf8().constData(), "");
		stopDaemonStrings.push_back(QString::fromUtf8(stop.get()));
	}

	int activeCommand = pref->getPreference("activedevice", -1);
	if (activeCommand > -1) {
		deviceSelectionTable->setCurrentCell(activeCommand, 0);
	}
}


void ImportTab::apply() {
	PreferencesTool *prefs = PreferencesTool::get();

	// Remove old preferences
	int numImports = prefs->getPreference("numberofimports", -1);
 	if (numImports > 0) {
		for (int i = 0; i < numImports; ++i) {
			prefs->removePreference(QStringLiteral("importname%1").arg(i).toUtf8().constData());
			prefs->removePreference(QStringLiteral("importdescription%1").arg(i).toUtf8().constData());
			prefs->removePreference(QStringLiteral("importprepoll%1").arg(i).toUtf8().constData());
			prefs->removePreference(QStringLiteral("importstartdaemon%1").arg(i).toUtf8().constData());
			prefs->removePreference(QStringLiteral("importstopdaemon%1").arg(i).toUtf8().constData());
		}
	}

	numImports = deviceSelectionTable->rowCount();
	if (numImports > 0) {
		prefs->setPreference("numberofimports", numImports);
		prefs->setPreference("activedevice", deviceSelectionTable->currentRow());
		for (int i = 0; i < numImports; ++i) {
			prefs->setPreference(QStringLiteral("importname%1").arg(i).toUtf8().constData(),
					deviceSelectionTable->item(i, 0)->text().toUtf8().constData());
			prefs->setPreference(QStringLiteral("importdescription%1").arg(i).toUtf8().constData(),
					deviceSelectionTable->item(i, 1)->text().toUtf8().constData());
			prefs->setPreference(QStringLiteral("importprepoll%1").arg(i).toUtf8().constData(),
					prePollStrings[i].toUtf8().constData());
			prefs->setPreference(QStringLiteral("importstartdaemon%1").arg(i).toUtf8().constData(),
					startDaemonStrings[i].toUtf8().constData());
			prefs->setPreference(QStringLiteral("importstopdaemon%1").arg(i).toUtf8().constData(),
					stopDaemonStrings[i].toUtf8().constData());
		}
	} else {
		prefs->setPreference("numberofimports", -1);
		prefs->setPreference("activedevice", -1);
	}
}


void ImportTab::resizeEvent(QResizeEvent *event)
{
	contentsChanged(0, 0);
	QWidget::resizeEvent(event);
}


void ImportTab::addImportProgram()
{
	int newRow = deviceSelectionTable->rowCount();
	deviceSelectionTable->setRowCount(newRow + 1);
	deviceSelectionTable->setItem( newRow, 0, new QTableWidgetItem(QString()) );
	deviceSelectionTable->setItem( newRow, 1, new QTableWidgetItem(QString()) );
	deviceSelectionTable->setCurrentCell(newRow, 0);

	prePollStrings.push_back(QString());
	startDaemonStrings.push_back(QString());
	stopDaemonStrings.push_back(QString());
}


void ImportTab::removeImportProgram()
{
	int selectedRow = deviceSelectionTable->currentRow();
	if (selectedRow >= 0) {
		prePollStrings.erase(prePollStrings.begin() + selectedRow);
		startDaemonStrings.erase(startDaemonStrings.begin() + selectedRow);
		stopDaemonStrings.erase(stopDaemonStrings.begin() + selectedRow);
		{
			// the vectors are shorter already
			QSignalBlocker blocker(deviceSelectionTable);
			deviceSelectionTable->removeRow(selectedRow);
		}
		contentsChanged(0, 0);
		activeCellChanged(0, 0);
	}
}


void ImportTab::contentsChanged(int, int)
{
	deviceSelectionTable->resizeColumnsToContents();
	int totalWidth = deviceSelectionTable->columnWidth(0) + deviceSelectionTable->columnWidth(1);
	int tableWidth = deviceSelectionTable->width() - 5;
	if ( totalWidth < tableWidth) {
		deviceSelectionTable->setColumnWidth( 1, tableWidth - deviceSelectionTable->columnWidth(0) );
	}
}


void ImportTab::activeCellChanged(int, int)
{
	if ( grabberPreferences->isVisible() ) {
		changeSettings();
	}
}


void ImportTab::changeSettings()
{
	int selected = deviceSelectionTable->currentRow();
	if (selected < 0) {
		closeChangeBox();
		return;
	}
	if (static_cast<int>(prePollStrings.size()) <= selected)
		return;
	editedRow = selected;
	prePollEdit->setText(prePollStrings[selected]);
	startDaemonEdit->setText(startDaemonStrings[selected]);
	stopDaemonEdit->setText(stopDaemonStrings[selected]);
	grabberPreferences->show();
}


void ImportTab::updatePrePollString(const QString &txt)
{
	if (0 <= editedRow && editedRow < static_cast<int>(prePollStrings.size()))
		prePollStrings[editedRow] = txt;
}


void ImportTab::updateStartDaemonString(const QString &txt)
{
	if (0 <= editedRow && editedRow < static_cast<int>(startDaemonStrings.size()))
		startDaemonStrings[editedRow] = txt;
}


void ImportTab::updatestopDaemonString(const QString &txt)
{
	if (0 <= editedRow && editedRow < static_cast<int>(stopDaemonStrings.size()))
		stopDaemonStrings[editedRow] = txt;
}


void ImportTab::closeChangeBox()
{
	editedRow = -1;
	grabberPreferences->hide();
	this->resize(minimumSize());
}


void ImportTab::retranslateStrings()
{
	informationText->setHtml(
		QStringLiteral("<p>") + tr("Below you can set which program/process Stopmotion should use "
		"for grabbing images from the selected device.") + QStringLiteral("</p><p>") +
		tr("You should always use <b>$VIDEODEVICE</b> and <b>$IMAGEFILE</b> to represent "
		"the video device and the image file, respectively.") + QStringLiteral("</p>"));

	QStringList lst;
	lst << tr("Name") << tr("Description");
	deviceSelectionTable->setHorizontalHeaderLabels(lst);

	addButton->setText( tr("&Add") );
	removeButton->setText( tr("&Remove") );
	changeButton->setText( tr("&Edit") );

	grabberPreferences->setTitle( tr("Import device settings") );
	prePollLabel->setText( tr("Pre-poll command") );
	startDaemonLabel->setText( tr("Start daemon") );
	stopDaemonLabel->setText( tr("Stop daemon") );
}
