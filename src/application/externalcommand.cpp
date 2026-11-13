/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "externalcommand.h"

#include <QByteArray>
#include <QChar>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTextCursor>
#include <QVBoxLayout>


ExternalCommand::ExternalCommand(QWidget *parent)
	: QWidget(parent)
{
    vboxLayout = new QVBoxLayout(this);
    vboxLayout->setSpacing(6);
    vboxLayout->setContentsMargins(9, 9, 9, 9);
    
	textBrowser = new QTextBrowser(this);
    vboxLayout->addWidget(textBrowser);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setContentsMargins(0, 0, 0, 0);
    
	label = new QLabel(this);
    label->setText(tr("Input to program:"));
    hboxLayout->addWidget(label);

    lineEdit = new QLineEdit(this);
	connect(lineEdit, &QLineEdit::returnPressed, this, &ExternalCommand::submitInputToProgram);
    lineEdit->setEchoMode(QLineEdit::Normal);
    hboxLayout->addWidget(lineEdit);

    submitButton = new QPushButton(this);
	connect(submitButton, &QAbstractButton::clicked, this, &ExternalCommand::submitInputToProgram);
    submitButton->setText(tr("Submit"));
    hboxLayout->addWidget(submitButton);

    closeButton = new QPushButton(this);
    connect(closeButton, &QAbstractButton::clicked, this, &QWidget::close);
    closeButton->setEnabled(false);
    closeButton->setText(tr("Close"));
    hboxLayout->addWidget(closeButton);

    vboxLayout->addLayout(hboxLayout);
	
	process = new QProcess(this);
	connect(process, &QProcess::readyReadStandardOutput, this, &ExternalCommand::readFromStandardOutput);
	connect(process, &QProcess::readyReadStandardError, this, &ExternalCommand::readFromStandardError);
	connect(process, &QProcess::finished, this, &ExternalCommand::displayExitStatus);
	connect(process, &QProcess::errorOccurred, this, &ExternalCommand::displayError);
    
	resize(QSize(593, 363).expandedTo(minimumSizeHint()));
	
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle( tr("Output from external command") );
}


void ExternalCommand::run(const QString &command)
{
	QStringList arguments = QProcess::splitCommand(command);
	if (arguments.isEmpty()) {
		QMessageBox::warning(this, tr("Result"), tr("Failed!"));
		finish();
		return;
	}
	const QString executable = arguments.takeFirst();
	process->start(executable, arguments);
}


void ExternalCommand::readFromStandardOutput()
{
	textBrowser->ensureCursorVisible();
	QTextCursor curs = textBrowser->textCursor();
	curs.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	curs.insertText(QString::fromLocal8Bit(process->readAllStandardOutput()));
}


void ExternalCommand::readFromStandardError()
{
	textBrowser->ensureCursorVisible();
	QTextCursor curs = textBrowser->textCursor();
	curs.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	curs.insertText(QString::fromLocal8Bit(process->readAllStandardError()));
}


void ExternalCommand::submitInputToProgram()
{
	if ( lineEdit->isModified() ) {
		QString input = lineEdit->text();
		input.append(u'\n');
		process->write(input.toLocal8Bit());
		lineEdit->clear();
	}
}


void ExternalCommand::displayError(QProcess::ProcessError error)
{
	// otherwise finished() follows
	if (error != QProcess::FailedToStart)
		return;
	textBrowser->append(process->errorString());
	QMessageBox::warning(this, tr("Result"), tr("Failed!"));
	finish();
}


void ExternalCommand::displayExitStatus(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
		QMessageBox::warning(this, tr("Result"), tr("Failed!"));
	}
	else {                                                     
		QMessageBox::information(this, tr("Result"), tr("Successful!"));
	}
	finish();
}


void ExternalCommand::finish()
{
	lineEdit->setEnabled(false);
	submitButton->setEnabled(false);
	closeButton->setEnabled(true);
}

