/***************************************************************************
 *   Copyright (C) 2017 by Linuxstopmotion contributors;                   *
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

#include "oomtestutil.h"
#include "testhome.h"
#include "fakefiles.h"
#include "src/domain/animation/workspacefile.h"
#include "src/foundation/uiexception.h"

#include <QTest>

#include "tworkspace.h"

TestWorkspace::TestWorkspace() : testEnvFs(0), mfs(0) {
	testEnvFs = new TestHome();
	mfs = new RealOggEmptyJpg();
}

TestWorkspace::~TestWorkspace() {
	delete mfs;
	delete testEnvFs;
}

void TestWorkspace::jpgsCanBeCopiedIntoWorkspace() {
	wrapFileSystem(0);
	wrapFileSystem(testEnvFs);
	wrapFileSystem(mfs);
	try {
		TemporaryWorkspaceFile wf("/tmp/image.jpg",
				WorkspaceFileType::image());
	} catch (...) {
		QFAIL("Copying .jpg image to workspace should be OK");
	}
	try {
		TemporaryWorkspaceFile wf("/tmp/image.jpeg",
				WorkspaceFileType::image());
	} catch (...) {
		QFAIL("Copying .jpeg image to workspace should be OK");
	}
	try {
		TemporaryWorkspaceFile wf("/tmp/image.JPG",
				WorkspaceFileType::image());
	} catch (...) {
		QFAIL("Copying .JPG image to workspace should be OK");
	}
	wrapFileSystem(0);
}

void TestWorkspace::nonJpgImagesCannotBeCopied() {
	wrapFileSystem(0);
	wrapFileSystem(testEnvFs);
	wrapFileSystem(mfs);
	try {
		TemporaryWorkspaceFile wf("/tmp/image.png",
				WorkspaceFileType::image());
	} catch (UiException& e) {
		QCOMPARE(e.warning(), UiException::unsupportedImageType);
		wrapFileSystem(0);
		return;
	} catch (...) {
	}
	QFAIL("Copying .PNG image to workspace should not be OK");
	wrapFileSystem(0);
}
