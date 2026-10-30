/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef EXPORTTAB_H
#define EXPORTTAB_H

#include <QWidget>

#include <vector>

class QPushButton;
class QGroupBox;
class QTableWidget;
class QLineEdit;
class QLabel;
class QGridLayout;
class QTextEdit;
class QToolBox;
class QRadioButton;


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
	ExportTab(QWidget *parent = nullptr);

	/**
	 * Applies the settings in the import tab.
	 */
	void apply();

	/**
	 * Initializes the table and fills it with starting values.
	 */
	void initialize();

	void retranslateStrings();

protected:
	void resizeEvent(QResizeEvent *event) override;

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
};

#endif
