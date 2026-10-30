/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "editmenuhandler.h"

#include "src/domain/domainfacade.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"

#include <QApplication>
#include <QClipboard>
#include <QDrag> 
#include <QList>
#include <QMimeData>
#include <QStatusBar>
#include <QStringList>
#include <QUrl>


EditMenuHandler::EditMenuHandler ( QObject *parent, QStatusBar *sb, 
		FrameBar *frameBar, const char *name ) 
	: QObject(parent), statusBar(sb), frameBar(frameBar)
{
	gotoMenu = nullptr;
	setObjectName(name);
}


void EditMenuHandler::setGotoMenu( QWidget * gotoMenu )
{
	this->gotoMenu = gotoMenu;
}


void EditMenuHandler::gotoFrame(int frameNumber)
{
	frameBar->updateNewActiveFrame(frameBar->getActiveScene(), frameNumber);
	closeGotoMenu();
}


void EditMenuHandler::closeGotoMenu()
{
	gotoMenu->hide();
}


void EditMenuHandler::undo()
{
	DomainFacade::getFacade()->undo();
	emit undoOrRedo();
}


void EditMenuHandler::redo()
{
	DomainFacade::getFacade()->redo();
	emit undoOrRedo();
}


void EditMenuHandler::copy()
{
	QList<QUrl> urls;

	int selectionFrame = frameBar->getSelectionAnchor();
	int activeScene = frameBar->getActiveScene();
	int activeFrame = frameBar->getActiveFrame();
	int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
	int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;

	DomainFacade* facade = DomainFacade::getFacade();
	for (int i = lowend; i <= highend; ++i) {
		const char* imagePath = facade->getImagePath(activeScene, i);
		if (imagePath)
			urls.append(QUrl::fromLocalFile(imagePath));
	}

	//QDrag *drag = new QDrag((MainWindowGUI*)this->parent());
	QMimeData *mimeData = new QMimeData;

	mimeData->setUrls(urls);
	//drag->setMimeData(mimeData);

	//drag->start(Qt::MoveAction);
	QApplication::clipboard()->setMimeData(mimeData);
}

void EditMenuHandler::cut() {
	copy();
	emit removeFrames();
}

void EditMenuHandler::paste()
{
	const QMimeData *mimeData = QApplication::clipboard()->mimeData();
	if ( mimeData && mimeData->hasUrls() ) {
		QStringList fileNames;
		QList<QUrl> urls = mimeData->urls();
		int numFrames = urls.size();
		for (int i = 0; i < numFrames; ++i) {
			fileNames.append(urls[i].toLocalFile());
		}
		emit addFrames(fileNames);
	}
}
