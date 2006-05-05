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
#include "importtab.h"

#include "flexiblelineedit.h"
#include "graphics/icons/close.xpm"

#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QTextEdit>
#include <QHeaderView>


ImportTab::ImportTab( QWidget *parent ) : QWidget(parent)
{
	deviceSelectionTable = 0;
	addButton            = 0;
	removeButton         = 0;
	changeButton         = 0;
	closeChangeBoxButton = 0;
	prePollEdit          = 0;
	startDeamonEdit      = 0;
	stopDeamonEdit       = 0;
	grabberPreferences   = 0;
	prePollLabel         = 0;
	startDeamonLabel     = 0;
	stopDeamonLabel      = 0;
	checkTableItem       = 0;
	
	makeGUI();
}


void ImportTab::makeGUI()
{
	this->setFocusPolicy(Qt::ClickFocus);
	
	QTextEdit *informationText = new QTextEdit;
	informationText->setReadOnly(true);
	informationText->setHtml(tr(
				"<p>Below you can set which program/process stopmotion should use "
				"for grabbing images from the webcam, and displaying video.<br> "
				"<br> </p>"));
	informationText->setMinimumWidth(440);
	informationText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	QStringList lst;
	lst << tr("Name") << tr("Description");
	
	deviceSelectionTable = new QTableWidget;
	deviceSelectionTable->setColumnCount(2);
	deviceSelectionTable->setRowCount(0);
	deviceSelectionTable->setSelectionMode(QAbstractItemView::SingleSelection);
	deviceSelectionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	deviceSelectionTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
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
	closeChangeBoxButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	closeChangeBoxButton->setIcon(QPixmap(closeicon));
	closeChangeBoxButton->setFlat(true);
	QObject::connect( closeChangeBoxButton, SIGNAL(clicked()),this, SLOT(closeChangeBox()));
	
	prePollLabel = new QLabel( tr("Pre-poll command") );
	prePollEdit = new FlexibleLineEdit;
	QObject::connect( prePollEdit, SIGNAL(textChanged(const QString &)), 
			this, SLOT(updatePrePollString(const QString &)));
	
	startDeamonLabel = new QLabel( tr("Start deamon") );
	startDeamonEdit = new FlexibleLineEdit;
	QObject::connect( startDeamonEdit, SIGNAL(textChanged(const QString &)), 
			this, SLOT(updateStartDeamonString(const QString &)));
	
	stopDeamonLabel = new QLabel( tr("Stop deamon") );
	stopDeamonEdit = new FlexibleLineEdit;
	QObject::connect( stopDeamonEdit, SIGNAL(textChanged(const QString &)), 
			this, SLOT(updatestopDeamonString(const QString &)) );
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(informationText);
	QVBoxLayout *buttonLayout = new QVBoxLayout;
	buttonLayout->setMargin(0);
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

	QVBoxLayout *grabberPrefsLayout = new QVBoxLayout;
	QHBoxLayout *hbLayout = new QHBoxLayout;
	hbLayout->setMargin(0);
	hbLayout->setSpacing(0);
	hbLayout->addStretch(1);
	hbLayout->addWidget(closeChangeBoxButton);
	grabberPrefsLayout->addLayout(hbLayout);
	grabberPrefsLayout->addWidget(prePollLabel);
	grabberPrefsLayout->addWidget(prePollEdit);
	grabberPrefsLayout->addWidget(startDeamonLabel);
	grabberPrefsLayout->addWidget(startDeamonEdit);
	grabberPrefsLayout->addWidget(stopDeamonLabel);
	grabberPrefsLayout->addWidget(stopDeamonEdit);
	grabberPreferences->setLayout(grabberPrefsLayout);
}


void ImportTab::initializeImportValues()
{
	PreferencesTool* pref = PreferencesTool::get();

	int numImports = pref->getPreference("numberofimports", 0);
	deviceSelectionTable->setRowCount(numImports);
	const char *prop = 0;
	for (int i = 0; i < numImports; ++i) {
		prop = pref->getPreference(QString("importname%1").arg(i).toLatin1().constData(), "");
		QString name(prop);
		freeProperty(prop);
		
		prop = pref->getPreference(QString("importdescription%1").arg(i).toLatin1().constData(), "");
		QString desc(prop);
		freeProperty(prop);
		
		deviceSelectionTable->setItem( i, 0, new QTableWidgetItem(name) );
		deviceSelectionTable->setItem( i, 1, new QTableWidgetItem(desc) );
		
		prop = pref->getPreference(QString("importprepoll%1").arg(i).toLatin1().constData(), "");
		prePollStrings.push_back(QString(prop));
		freeProperty(prop);
		
		prop = pref->getPreference(QString("importstartdeamon%1").arg(i).toLatin1().constData(), "");
		startDeamonStrings.push_back(QString(prop));
		freeProperty(prop);
		
		prop = pref->getPreference(QString("importstopdeamon%1").arg(i).toLatin1().constData(), "");
		stopDeamonStrings.push_back(QString(prop));
		freeProperty(prop);
	}
	
	int activeCommand = pref->getPreference("activedevice", -1);
	if (activeCommand > -1) {
		deviceSelectionTable->setCurrentCell(activeCommand, 0);
	}
}


void ImportTab::apply()
{
	PreferencesTool *prefs = PreferencesTool::get();
	
	// Remove old preferences
	int numImports = prefs->getPreference("numberofimports", -1);
 	if (numImports > 0) {
		for (int i = 0; i < numImports; ++i) {
			prefs->removePreference(QString("importname%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importdescription%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importprepoll%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importstartdeamon%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("importstopdeamon%1").arg(i).toLatin1().constData());
		}
	}
	
	numImports = deviceSelectionTable->rowCount();
	if (numImports > 0) {
		prefs->setPreference("numberofimports", numImports, true);
		prefs->setPreference("activedevice", deviceSelectionTable->currentRow(), true);
		for (int i = 0; i < numImports; ++i) {
			prefs->setPreference(QString("importname%1").arg(i).toLatin1().constData(), 
					deviceSelectionTable->item(i, 0)->text().toLatin1().constData(), true);
			prefs->setPreference(QString("importdescription%1").arg(i).toLatin1().constData(),
					deviceSelectionTable->item(i, 1)->text().toLatin1().constData(), true);
			prefs->setPreference(QString("importprepoll%1").arg(i).toLatin1().constData(),
					prePollStrings[i].toLatin1().constData(), true);
			prefs->setPreference(QString("importstartdeamon%1").arg(i).toLatin1().constData(),
					startDeamonStrings[i].toLatin1().constData(), true);
			prefs->setPreference(QString("importstopdeamon%1").arg(i).toLatin1().constData(),
					stopDeamonStrings[i].toLatin1().constData(), true);
		}
	}
	else {
		prefs->setPreference("numberofimports", -1, true);
		prefs->setPreference("activedevice", -1, true);
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
	startDeamonStrings.push_back("");
	stopDeamonStrings.push_back("");
}


void ImportTab::removeImportProgram()
{
	int selectedRow = deviceSelectionTable->currentRow();
	if (selectedRow >= 0) {
		prePollStrings.erase(prePollStrings.begin() + selectedRow);
		startDeamonStrings.erase(startDeamonStrings.begin() + selectedRow);
		stopDeamonStrings.erase(stopDeamonStrings.begin() + selectedRow);
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
		startDeamonEdit->setText(startDeamonStrings[selected]);
		stopDeamonEdit->setText(stopDeamonStrings[selected]);
		grabberPreferences->show();
	}
}


void ImportTab::updatePrePollString(const QString &txt)
{
	prePollStrings[deviceSelectionTable->currentRow()] = txt;
}


void ImportTab::updateStartDeamonString(const QString &txt)
{
	startDeamonStrings[deviceSelectionTable->currentRow()] = txt;
}


void ImportTab::updatestopDeamonString(const QString &txt)
{
	stopDeamonStrings[deviceSelectionTable->currentRow()] = txt;
}

	
void ImportTab::freeProperty(const char *prop, const char *tag)
{
	if (strcmp(prop, tag) != 0) {
		xmlFree((xmlChar *)prop);
	}
}


void ImportTab::closeChangeBox()
{
	grabberPreferences->hide();
	this->resize(minimumSize());
}
