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
#ifndef EXPORTTAB_H
#define EXPORTTAB_H

#include <QWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QTextEdit>
#include <QToolBox>
#include <QRadioButton>

#include <vector>


/**
 * The export tab in the preferences menu
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ExportTab : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Sets up the tab.
	 * @param parent the parent of the this widget
	 */
	ExportTab(QWidget *parent = 0);
	
	/**
	 * Applies the settings in the import tab.
	 */
	void apply();
	
	/**
	 * Initializes the table and fills it with starting values.
	 */
	void initialize();
	
protected:
	void resizeEvent(QResizeEvent *event);
	
private slots:
	void addEncoder();
	void removeEncoder();
	void contentsChanged(int row, int column);
	void activeCellChanged(int row, int column);
	void editSettings();
	void closeSettings();
	void updateStartString(const QString &txt);
	void updateStopString(const QString &txt);
	void setDefaultOutput(const QString &txt);
	void setYesButtonOn();
	void setNoButtonOn();
	void browseFiles();
	
private:
	std::vector<QString>startEncoderStrings;
	std::vector<QString>stopEncoderStrings;
	std::vector<QString>outputFiles;
	
	QPushButton *addButton;
	QPushButton *removeButton;
	QPushButton *editButton;
	QPushButton *browseButton;
	QPushButton *closeButton;
	QRadioButton *yesButton;
	QRadioButton *noButton;
	QGroupBox *encoderPrefs;
	QTableWidget *encoderTable;
	QLineEdit *startEncoder;
	QLineEdit *stopEncoder;
	QLabel *startEncoderLabel;
	QLabel *stopEncoderLabel;
	QLineEdit *defaultOutput;
	QLabel *defaultOutputLabel;
	QLabel *askForOutput;
	QTextEdit *infoText;
	
	void makeGUI();
	void freeProperty(const char *prop, const char *tag = "");
};

#endif
