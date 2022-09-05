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
#ifndef IMPORTTAB_H
#define IMPORTTAB_H

#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QWidget>
#include <QTextEdit>

#include <vector>

using namespace std;


/**
 * The import tab in the preferences menu.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ImportTab : public QWidget
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

	void retranslateStrings();

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void addImportProgram();
	void removeImportProgram();
	void contentsChanged(int row, int column);
	void activeCellChanged(int row, int column);
	void changeSettings();
	void closeChangeBox();
	void updatePrePollString(const QString &txt);
	void updateStartDaemonString(const QString &txt);
	void updatestopDaemonString(const QString &txt);

private:
	vector<QString>prePollStrings;
	vector<QString>startDaemonStrings;
	vector<QString>stopDaemonStrings;

	QTableWidget *deviceSelectionTable;
	QPushButton *addButton;
	QPushButton *removeButton;
	QPushButton *changeButton;
	QPushButton *closeChangeBoxButton;
	QLineEdit *prePollEdit;
	QLineEdit *startDaemonEdit;
	QLineEdit *stopDaemonEdit;
	QGroupBox *grabberPreferences;
	QLabel *prePollLabel;
	QLabel *startDaemonLabel;
	QLabel *stopDaemonLabel;
	QTableWidgetItem *checkTableItem;
	QTextEdit *informationText;
};

#endif
