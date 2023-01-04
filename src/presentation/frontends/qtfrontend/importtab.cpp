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
#include "importtab.h"

#include "flexiblelineedit.h"
#include "preferencestool.h"
#include "graphics/icons/close.xpm"

#include <algorithm>
#include <memory>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QHeaderView>

class QResizeEvent;

ImportTab::ImportTab( QWidget *parent ) : QWidget(parent)
{
	deviceSelectionTable = 0;
	addButton            = 0;
	removeButton         = 0;
	changeButton         = 0;
	closeChangeBoxButton = 0;
	prePollEdit          = 0;
	startDaemonEdit      = 0;
	stopDaemonEdit       = 0;
	grabberPreferences   = 0;
	prePollLabel         = 0;
	startDaemonLabel     = 0;
	stopDaemonLabel      = 0;
	checkTableItem       = 0;
	informationText      = 0;

	makeGUI();
}


void ImportTab::makeGUI()
{
	this->setFocusPolicy(Qt::ClickFocus);

	informationText = new QTextEdit;
	informationText->setReadOnly(true);
	informationText->setHtml(
		"<p>" + tr("Below you can set which program/process Stopmotion should use "
		"for grabbing images from the selected device.") + "</p><p>" +
		tr("You should always use <b>$VIDEODEVICE</b> and <b>$IMAGEFILE</b> to represent "
		"the video device and the image file, respectively.") + "</p>");
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

	connect(deviceSelectionTable, SIGNAL(cellClicked(int, int)),
			this, SLOT(activeCellChanged(int, int)));
	connect(deviceSelectionTable, SIGNAL(cellChanged(int, int)),
			this, SLOT(contentsChanged(int, int)));

	addButton = new QPushButton(tr("&Add"));
	addButton->setFocusPolicy( Qt::NoFocus );
	QObject::connect(addButton, SIGNAL(clicked()), this, SLOT(addImportProgram()));

	removeButton = new QPushButton(tr("&Remove"));
	QObject::connect( removeButton, SIGNAL(clicked()), this, SLOT(removeImportProgram()));

	changeButton = new QPushButton(tr("&Edit"));
	QObject::connect( changeButton, SIGNAL(clicked()), this, SLOT(changeSettings()));

	grabberPreferences = new QGroupBox;
	grabberPreferences->setTitle(tr("Import device settings"));
	grabberPreferences->hide();

	closeChangeBoxButton = new QPushButton;
	closeChangeBoxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	closeChangeBoxButton->setIcon(QPixmap(closeicon));
	closeChangeBoxButton->setFlat(true);
	QObject::connect( closeChangeBoxButton, SIGNAL(clicked()),this, SLOT(closeChangeBox()));

	prePollLabel = new QLabel( tr("Pre-poll command") );
	prePollEdit = new FlexibleLineEdit;
	prePollLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	prePollEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QObject::connect( prePollEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updatePrePollString(const QString &)));

	startDaemonLabel = new QLabel( tr("Start daemon") );
	startDaemonEdit = new FlexibleLineEdit;
	startDaemonLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	startDaemonEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QObject::connect( startDaemonEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updateStartDaemonString(const QString &)));

	stopDaemonLabel = new QLabel( tr("Stop daemon") );
	stopDaemonEdit = new FlexibleLineEdit;
	stopDaemonLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	stopDaemonEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QObject::connect( stopDaemonEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updatestopDaemonString(const QString &)) );

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
		Preference name(QString("importname%1").arg(i).toLatin1().constData(), "");
		deviceSelectionTable->setItem( i, 0, new QTableWidgetItem(name.get()) );
		Preference desc(QString("importdescription%1").arg(i).toLatin1().constData(), "");
		deviceSelectionTable->setItem( i, 1, new QTableWidgetItem(desc.get()) );

		Preference prepoll(QString("importprepoll%1").arg(i).toLatin1().constData(), "");
		prePollStrings.push_back(QString(prepoll.get()));

		Preference start(QString("importstartdaemon%1").arg(i).toLatin1().constData(), "");
		startDaemonStrings.push_back(QString(start.get()));

		Preference stop(QString("importstopdaemon%1").arg(i).toLatin1().constData(), "");
		stopDaemonStrings.push_back(QString(stop.get()));
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
			prefs->removePreference(QString("importname%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importdescription%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importprepoll%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importstartdaemon%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importstopdaemon%1").arg(i).toLatin1().constData());
		}
	}

	numImports = deviceSelectionTable->rowCount();
	if (numImports > 0) {
		prefs->setPreference("numberofimports", numImports);
		prefs->setPreference("activedevice", deviceSelectionTable->currentRow());
		for (int i = 0; i < numImports; ++i) {
			prefs->setPreference(QString("importname%1").arg(i).toLatin1().constData(),
					deviceSelectionTable->item(i, 0)->text().toLatin1().constData());
			prefs->setPreference(QString("importdescription%1").arg(i).toLatin1().constData(),
					deviceSelectionTable->item(i, 1)->text().toLatin1().constData());
			prefs->setPreference(QString("importprepoll%1").arg(i).toLatin1().constData(),
					prePollStrings[i].toLatin1().constData());
			prefs->setPreference(QString("importstartdaemon%1").arg(i).toLatin1().constData(),
					startDaemonStrings[i].toLatin1().constData());
			prefs->setPreference(QString("importstopdaemon%1").arg(i).toLatin1().constData(),
					stopDaemonStrings[i].toLatin1().constData());
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
	deviceSelectionTable->setItem( newRow, 0, new QTableWidgetItem(QString("")) );
	deviceSelectionTable->setItem( newRow, 1, new QTableWidgetItem(QString("")) );
	deviceSelectionTable->setCurrentCell(newRow, 0);

	prePollStrings.push_back("");
	startDaemonStrings.push_back("");
	stopDaemonStrings.push_back("");
}


void ImportTab::removeImportProgram()
{
	int selectedRow = deviceSelectionTable->currentRow();
	if (selectedRow >= 0) {
		prePollStrings.erase(prePollStrings.begin() + selectedRow);
		startDaemonStrings.erase(startDaemonStrings.begin() + selectedRow);
		stopDaemonStrings.erase(stopDaemonStrings.begin() + selectedRow);
		deviceSelectionTable->removeRow(selectedRow);
		contentsChanged(0, 0);
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
	if (selected >= 0) {
		prePollEdit->setText(prePollStrings[selected]);
		startDaemonEdit->setText(startDaemonStrings[selected]);
		stopDaemonEdit->setText(stopDaemonStrings[selected]);
		grabberPreferences->show();
	}
}


void ImportTab::updatePrePollString(const QString &txt)
{
	prePollStrings[deviceSelectionTable->currentRow()] = txt;
}


void ImportTab::updateStartDaemonString(const QString &txt)
{
	startDaemonStrings[deviceSelectionTable->currentRow()] = txt;
}


void ImportTab::updatestopDaemonString(const QString &txt)
{
	stopDaemonStrings[deviceSelectionTable->currentRow()] = txt;
}


void ImportTab::closeChangeBox()
{
	grabberPreferences->hide();
	this->resize(minimumSize());
}


void ImportTab::retranslateStrings()
{
	informationText->setHtml(
		"<p>" + tr("Below you can set which program/process Stopmotion should use "
		"for grabbing images from the selected device.") + "</p><p>" +
		tr("You should always use <b>$VIDEODEVICE</b> and <b>$IMAGEFILE</b> to represent "
		"the video device and the image file, respectively.") + "</p>");

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
