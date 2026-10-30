/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "modelhandler.h"

#include "logger.h"
#include "src/presentation/frontends/qtfrontend/mainwindowgui.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"
#include "src/domain/domainfacade.h"
#include "src/technical/stringiterator.h"
#include "src/technical/util.h"

#include <cstddef>
#include <string>
#include <QChar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QStringList>

ModelHandler::ModelHandler ( QObject *parent, QStatusBar *sb, FrameBar *frameBar,
		QString* lastVisitedDir, const char *name )
		: QObject(parent), frameBar(frameBar), statusBar(sb),
		lastVisitedDir(lastVisitedDir) {
	fileDialog = nullptr;
	removeFramesButton = nullptr;
	setObjectName(name);
}


ModelHandler::~ModelHandler() {
}


void ModelHandler::setRemoveFramesButton(QPushButton* removeFramesButton) {
	this->removeFramesButton = removeFramesButton;
}


void ModelHandler::chooseFrame() {
	delete fileDialog;
	fileDialog = new QFileDialog((MainWindowGUI*)parent(),
			tr("Choose frames to add"), *lastVisitedDir);
	QStringList filters;
	filters << "Images (*.png *.jpg *.jpeg  *.gif *.PNG *.JPG *.JPEG *.GIF)"
			<< "Joint Photographic Ex. Gr. (*.jpg *.jpeg *.JPG *.JPEG)"
			<< "Portable Network Graphics (*.png *.PNG)"
			<< "GIMP native (*.xcf *.XCF)"
			<< "Tagged Image File Format (*.tif *.TIF)"
			<< "Windows Bitmap (*.bmp *.BMP)"
			<< "TrueVision Targa (*.tga *.TGA)"
			<< "Portable Anymap (*.pnm *.PNM)"
			<< "X11 Pixmap (*.xpm *.XPM)"
			<< "ZSoft IBM PC Paintbrush (*.pcx *.PCX)"
			<< "CompuServe Graph. Interch. Format (*.gif *.GIF)"
			<< "Interleaved Bitmap (*.lbm *.iff *.LBM *.IFF)"
			<< "All files (*)";
	fileDialog->setNameFilters(filters);
	fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog->setFileMode(QFileDialog::ExistingFiles);

	//PicturePreview* p = new PicturePreview(fileDialog);

	QObject::connect( fileDialog, &QFileDialog::filesSelected,
			this, &ModelHandler::addFrames );

	fileDialog->show();
}


class StringListIterator : public StringIterator {
	QStringList::ConstIterator b;
	QStringList::ConstIterator e;
	std::string buffer;

	void set() {
		if (!atEnd()) {
			buffer = b->toStdString();
		}
	}

public:
	StringListIterator(QStringList::ConstIterator begin,
			QStringList::ConstIterator end) : b(begin), e(end) {
		set();
	}
	~StringListIterator() override {
	}
	int count() override {
		int c = 0;
		for (QStringList::ConstIterator i(b); i != e; ++i) {
			++c;
		}
		return c;
	}
	bool atEnd() const override {
		return b == e;
	}
	const char* get() const override {
		return buffer.c_str();
	}
	void next() override {
		++b;
		set();
	}
};


void ModelHandler::addFrames(const QStringList & fileNames) {
	Logger::get().logDebug("addFrames in modelhandler");

	// the fileDialog pointer is NULL when adding of frames is
	// done by drag 'n drop
	if ( fileDialog != nullptr ) {
		fileDialog->hide();
		*lastVisitedDir = fileDialog->directory().path();
	}

	if ( !fileNames.isEmpty() ) {
		StringListIterator fNames(fileNames.begin(), fileNames.end());
		int scene = frameBar->getActiveScene();
		int frame = frameBar->getActiveFrame() + 1;
		if (scene < 0) {
			scene = 0;
		}
		DomainFacade* facade = DomainFacade::getFacade();
		int frameCount = facade->getSceneSize(scene);
		if (frameCount < frame)
			frame = frameCount;
		if (frame < 0)
			frame = 0;
		facade->addFrames(scene, frame, fNames);
		emit modelChanged();
	}
}


void ModelHandler::addFrame( const QString &fileName ) {
	if (fileDialog != nullptr) {
		fileDialog->hide();
		*lastVisitedDir = fileDialog->directory().path();
	}

	QStringList fileNames;
	fileNames.push_back(fileName);
	this->addFrames(fileNames);
}


void ModelHandler::removeFrames() {
	if (removeFramesButton->isEnabled()) {
		int selectionFrame = frameBar->getSelectionAnchor();
		int activeScene = frameBar->getActiveScene();
		int activeFrame = frameBar->getActiveFrame();
		int lowend = (selectionFrame < activeFrame ) ? selectionFrame : activeFrame;
		int highend = (selectionFrame > activeFrame ) ? selectionFrame : activeFrame;
		if (activeScene < 0 || lowend < 0) {
			return;
		}

		DomainFacade::getFacade()->removeFrames(activeScene, lowend,
				highend - lowend + 1);
		statusBar->showMessage( tr("Removed the selected frame"), 2000 );
	}
}


void ModelHandler::newScene() {
	int activeScene = frameBar->getActiveScene();

	if (activeScene >= 0) {
		DomainFacade::getFacade()->newScene(activeScene+1);
	}
	else {
		int numScenes = DomainFacade::getFacade()->getNumberOfScenes();
		if(numScenes > 0) {
			DomainFacade::getFacade()->newScene(numScenes);
		}
		else {
			DomainFacade::getFacade()->newScene(activeScene+1);
		}
	}
	emit modelChanged();
}


void ModelHandler::removeScene() {
	int activeScene = frameBar->getActiveScene();
	if (0 <= activeScene)
		DomainFacade::getFacade()->removeScene(activeScene);
}


/*!
    \fn ModelHandler::editCurrentFrame()
 */
int ModelHandler::editCurrentFrame() {
	std::string gimpCommand;
	if (!Util::checkCommand(&gimpCommand, "gimp")) {
		QMessageBox::warning(static_cast<MainWindowGUI *>(parent()), tr("Warning"),
			tr("You do not have Gimp installed on your system"));
		return 1;
	}

	// Determine the active scene and active frame.
	int activeScene = frameBar->getActiveScene();
	int activeFrame = frameBar->getActiveFrame();

	if (activeScene < 0 || activeFrame < 0) {
		QMessageBox::warning(static_cast<MainWindowGUI *>(parent()), tr("Warning"),
			tr("There is no active frame to open"));
		return 1;
	}
	DomainFacade* facade = DomainFacade::getFacade();
	facade->duplicateImage(activeScene, activeFrame);
	const char *path = facade->getImagePath(activeScene, activeFrame);

	QStringList argList;
	// arg0 are the options, and arg1 is the path of the frame.
	// Start Gimp without splash screen.
	argList.append(QStringLiteral(u"--no-splash"));
	argList.append(QString::fromLocal8Bit(path));

	QProcess process;
	if (!process.startDetached(QLatin1String(gimpCommand.c_str()), argList)) {
		QMessageBox::warning(static_cast<MainWindowGUI *>(parent()), tr("Warning"),
			tr("Failed to start Gimp!"));
		return 1;
	}

	return 0;
}
