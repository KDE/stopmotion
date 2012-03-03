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
#include "importtab.h"

#include "src/foundation/preferencestool.h"
#include "flexiblelineedit.h"

#include "graphics/icons/close.xpm"

#include <qlayout.h>
#include <qlabel.h>
#include <iostream>


ImportTab::ImportTab( QWidget *parent ) : QFrame(parent)
{
	selectedDevice = -1;
	
	deviceSelectionTable = NULL;
	addButton            = NULL;
	removeButton         = NULL;
	changeButton         = NULL;
	prePollEdit          = NULL;
	startDeamonEdit      = NULL;
	stopDeamonEdit       = NULL;
	grabberPreferences   = NULL;
	
	makeGUI();
}


void ImportTab::makeGUI()
{
	this->setFocusPolicy(QWidget::ClickFocus);
	QGridLayout *grid = new QGridLayout( this, 1, 1, 3 );
	
	QLabel *informationText = 
			new QLabel(tr(
				"<p>Below you can set which program/process stopmotion should use "
				"for grabbing images from the webcam, and displaying video.<br> "
				"<br> </p>"), this);
	informationText->setMinimumWidth(440);
	informationText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	deviceSelectionTable = new QTable(this);
	deviceSelectionTable->setMinimumHeight(120);
	deviceSelectionTable->setNumCols(3);
	deviceSelectionTable->setSelectionMode(QTable::SingleRow);
	deviceSelectionTable->horizontalHeader()->setLabel( 0, tr( "Active" ) );
	deviceSelectionTable->horizontalHeader()->setLabel( 1, tr( "Name" ) );
	deviceSelectionTable->horizontalHeader()->setLabel( 2, tr( "Description" ) );
	deviceSelectionTable->adjustColumn(0);
	deviceSelectionTable->adjustColumn(1);
	deviceSelectionTable->adjustColumn(2);
	QObject::connect( deviceSelectionTable, SIGNAL(currentChanged(int, int)),
			this, SLOT(activeRowChanged(int)) );
	QObject::connect( deviceSelectionTable, SIGNAL(valueChanged(int, int)),
			this, SLOT(valueChanged(int, int)) );
	
	addButton = new QPushButton(tr("&Add") ,this);
	addButton->setFocusPolicy( QWidget::NoFocus );
	QObject::connect(addButton, SIGNAL(clicked()), 
			this, SLOT(addImportProgram()) );
	
	removeButton = new QPushButton(tr("&Remove"), this);
	QObject::connect( removeButton, SIGNAL(clicked()), 
			this, SLOT(removeImportProgram()) );
	
	changeButton = new QPushButton(tr("&Edit"), this);
	QObject::connect( changeButton, SIGNAL(clicked()), 
			this, SLOT(changeSettings()) );
	
	QSpacerItem *space3 = new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
	QSpacerItem *rightSpace = new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	QSpacerItem *leftSpace = new QSpacerItem(5, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	grabberPreferences = new QGroupBox(this);
	grabberPreferences->setTitle(tr("Import device settings"));
	grabberPreferences->hide();
	
	QGridLayout *grabberGrid = new QGridLayout(grabberPreferences, 1, 1, 3);
	
	QPushButton *closeChangeBoxButton = new QPushButton(grabberPreferences);
	closeChangeBoxButton->setPixmap(closeicon);
	closeChangeBoxButton->setFlat(true);
	QObject::connect( closeChangeBoxButton, SIGNAL(clicked()),
			this, SLOT(closeChangeBox()) );
	
	QLabel *prePollLabel = new QLabel( tr("Pre-poll command"), grabberPreferences );
	prePollEdit = new FlexibleLineEdit(grabberPreferences);
	QObject::connect( prePollEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updatePrePollString(const QString &)) );
	
	QLabel *startDeamonLabel = new QLabel( tr("Start deamon"), grabberPreferences );
	startDeamonEdit = new FlexibleLineEdit(grabberPreferences);
	QObject::connect( startDeamonEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updateStartDeamonString(const QString &)) );
	
	QLabel *stopDeamonLabel = new QLabel( tr("Stop deamon"), grabberPreferences );
	stopDeamonEdit = new FlexibleLineEdit(grabberPreferences);
	QObject::connect( stopDeamonEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(updatestopDeamonString(const QString &)) );
	
	grid->addWidget( informationText, 0, 1 );
	grid->addMultiCellWidget( deviceSelectionTable, 1, 4, 1, 1 );
	grid->addWidget( addButton, 2, 2 );
	grid->addWidget( removeButton, 3, 2 );
	grid->addWidget( changeButton, 4, 2 );
	
	grabberGrid->addWidget(closeChangeBoxButton, 0, 1);
	grabberGrid->addWidget(prePollLabel, 1, 0);
	grabberGrid->addWidget(prePollEdit, 2, 0);
	grabberGrid->addWidget(startDeamonLabel, 3, 0);
	grabberGrid->addWidget(startDeamonEdit, 4, 0);
	grabberGrid->addWidget(stopDeamonLabel, 5, 0);
	grabberGrid->addWidget(stopDeamonEdit, 6, 0);
	grid->addMultiCellWidget( grabberPreferences, 5, 5, 1, 2 );
	
	grid->addItem( space3, 6, 1 );
	grid->addItem( rightSpace, 3, 3 );
	grid->addItem( leftSpace, 3, 0);
}


void ImportTab::initializeImportValues()
{
	PreferencesTool* pref = PreferencesTool::get();
	selectedDevice = PreferencesTool::get()->getPreference("activeCommand", 0);

	
	int numImports = pref->getPreference("numberofimports", 1);
	int newRow = 0;
	for(int i=0; i<numImports; i++) {
		newRow = deviceSelectionTable->numRows();
		deviceSelectionTable->insertRows(newRow);
		deviceSelectionTable->setItem( newRow, 0, 
				new QCheckTableItem( deviceSelectionTable, "") );
		deviceSelectionTable->setText( newRow, 1, QString(pref->getPreference(
				QString("importname%1").arg(i).ascii(), "")) );
		deviceSelectionTable->setText( newRow, 2, QString( pref->getPreference(
				QString("importdescription%1").arg(i).ascii(), "")) );
		prePollStrings.push_back(QString( pref->getPreference(
				QString("importprepoll%1").arg(i).ascii(), "")));
		startDeamonStrings.push_back(QString( pref->getPreference(
				QString("importstartdeamon%1").arg(i).ascii(), "")));
		stopDeamonStrings.push_back(QString( pref->getPreference(
				QString("importstopdeamon%1").arg(i).ascii(), "")));
		
	}
	deviceSelectionTable->adjustColumn(1);
	deviceSelectionTable->adjustColumn(2);
	
	int width = deviceSelectionTable->columnWidth(2);
	int prefWidth = deviceSelectionTable->width() - 
			(deviceSelectionTable->columnWidth(0) + deviceSelectionTable->columnWidth(1));
	if (width < prefWidth) {
		deviceSelectionTable->setColumnWidth(2, prefWidth + 4);
	}
	
	int activeCommand = pref->getPreference("activedevice", 0);
	if(activeCommand > -1) {
		((QCheckTableItem*)deviceSelectionTable->item(activeCommand, 0))->setChecked(true);
	}
}


void ImportTab::apply()
{
	PreferencesTool *prefs = PreferencesTool::get();
	int size = deviceSelectionTable->numRows();
	int activeCommand = -1;
	
	//Deletes removed imports from the preferencestool.
	int prefSize = prefs->getPreference("numberofimports", -1);
 	if(prefSize > size) {
		cout << "hm" << endl;
		for(int i=size; i<prefSize; ++i) {
			prefs->removePreference(QString("importname%1").arg(i).ascii());
			prefs->removePreference(QString("importdescription%1").arg(i).ascii());
			prefs->removePreference(QString("importprepoll%1").arg(i).ascii());
			prefs->removePreference(QString("importstartdeamon%1").arg(i).ascii());
			prefs->removePreference(QString("importstopdeamon%1").arg(i).ascii());
		}
	}
	prefs->setPreference("numberofimports", size, true);
	
	for(int i=0; i<size; i++) {
		prefs->setPreference(QString("importname%1").arg(i), 
				deviceSelectionTable->text(i, 1).ascii(), true);
		prefs->setPreference(QString("importdescription%1").arg(i),
				deviceSelectionTable->text(i, 2).ascii(), true);
		prefs->setPreference(QString("importprepoll%1").arg(i),
				prePollStrings[i].ascii(), true);
		prefs->setPreference(QString("importstartdeamon%1").arg(i),
				startDeamonStrings[i].ascii(), true);
		prefs->setPreference(QString("importstopdeamon%1").arg(i),
				stopDeamonStrings[i].ascii(), true);
		if( ((QCheckTableItem*)deviceSelectionTable->item(i, 0))->isChecked() ) {
			activeCommand = i;
		}
	}
	
	prefs->setPreference("activedevice", activeCommand, true);
}


void ImportTab::addImportProgram()
{
	int newRow = deviceSelectionTable->numRows();
	deviceSelectionTable->insertRows(newRow);
	deviceSelectionTable->setItem( newRow, 0, 
			new QCheckTableItem( deviceSelectionTable, "") );
	prePollStrings.push_back("");
	startDeamonStrings.push_back("");
	stopDeamonStrings.push_back("");
}


void ImportTab::removeImportProgram()
{
	prePollStrings.erase(prePollStrings.begin() + selectedDevice);
	startDeamonStrings.erase(startDeamonStrings.begin() + selectedDevice);
	stopDeamonStrings.erase(stopDeamonStrings.begin() + selectedDevice);
	deviceSelectionTable->removeRow(selectedDevice);
}


void ImportTab::valueChanged(int row, int column)
{
	switch (column)
	{
		case 0:
		{
			int size = deviceSelectionTable->numRows();
			for(int i = 0; i<size; i++) {
				((QCheckTableItem*)deviceSelectionTable->item(i, 0))->
						setChecked(i == row);
			}
			break;
		}
		case 1: case 2:
		{
			deviceSelectionTable->adjustColumn(1);
			deviceSelectionTable->adjustColumn(2);
			
			int width = deviceSelectionTable->columnWidth(2);
			int prefWidth = deviceSelectionTable->width() - 
					(deviceSelectionTable->columnWidth(0) + 
					deviceSelectionTable->columnWidth(1));
			if (width < prefWidth) {
				deviceSelectionTable->setColumnWidth(2, prefWidth - 34);
			}
			
			activeRowChanged(row);
			break;
		}
	}
}


void ImportTab::activeRowChanged(int row)
{
	selectedDevice = row;
	if(grabberPreferences->isShown()) {
		changeSettings();
	}
}


void ImportTab::changeSettings()
{
	if(selectedDevice >= 0) {
		prePollEdit->setText(prePollStrings[selectedDevice]);
		startDeamonEdit->setText(startDeamonStrings[selectedDevice]);
		stopDeamonEdit->setText(stopDeamonStrings[selectedDevice]);
		grabberPreferences->show();
	}
}


void ImportTab::updatePrePollString(const QString &txt)
{
	prePollStrings[selectedDevice] = txt;
}


void ImportTab::updateStartDeamonString(const QString &txt)
{
	startDeamonStrings[selectedDevice] = txt;
}


void ImportTab::updatestopDeamonString(const QString &txt)
{
	stopDeamonStrings[selectedDevice] = txt;
}


void ImportTab::closeChangeBox()
{
	grabberPreferences->hide();
}
