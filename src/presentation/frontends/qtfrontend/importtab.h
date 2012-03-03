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
#ifndef IMPORTTAB_H
#define IMPORTTAB_H

#include <qtable.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <vector>
using namespace std;


/**
 * The import tab in the preferences menu.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ImportTab : public QFrame
{
	Q_OBJECT
public:
	/**
	 * Sets up the import tab.
	 * @param parent the parent of the widget.
	 */
	ImportTab( QWidget *parent = 0 );
	
	/**
	 * Sets up the gui for the tab and connects the buttons.
	 */
	void makeGUI();
	
	/**
	 * Initializes the table and fills it with starting values.
	 */
	void initializeImportValues();
	
	/**
	 * Applies the settings in the import tab.
	 */
	void apply();
	
private slots:
	void addImportProgram();
	void removeImportProgram();
	void valueChanged(int row, int column);
	void activeRowChanged(int row);
	void changeSettings();
	void closeChangeBox();
	//These are called so often that I choose to have one for each
	//lineedit instead of 1 with test.
	void updatePrePollString(const QString &txt);
	void updateStartDeamonString(const QString &txt);
	void updatestopDeamonString(const QString &txt);
	
private:
	int selectedDevice;
	int numGrabbers;
	
	//Cache these so that i they aren't flushed to the preftool until the user
	//press ok.
	vector<QString>prePollStrings;
	vector<QString>startDeamonStrings;
	vector<QString>stopDeamonStrings;
	
	QTable *deviceSelectionTable;
	QPushButton *addButton;
	QPushButton *removeButton;
	QPushButton *changeButton;
	QPushButton *closeChangeBoxButton;
	QLineEdit *prePollEdit;
	QLineEdit *startDeamonEdit;
	QLineEdit *stopDeamonEdit;
	QGroupBox *grabberPreferences;
	QGridLayout *grid;
	QGridLayout *grabberGrid;
	QSpacerItem *space3;
	QSpacerItem *rightSpace;
	QSpacerItem *leftSpace;
	QLabel *prePollLabel;
	QLabel *startDeamonLabel;
	QLabel *stopDeamonLabel;
	QCheckTableItem *checkTableItem;
	
	void updateChange(const QString &txt);
};

#endif
