/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "oomtestutil.h"
#include "testhome.h"
#include "fakefiles.h"
#include "src/domain/animation/workspacefile.h"
#include "src/foundation/uiexception.h"

#include <QTest>

#include "tworkspace.h"

TestWorkspace::TestWorkspace() : testEnvFs(nullptr), mfs(nullptr) {
	testEnvFs = new TestHome();
	mfs = new RealOggEmptyJpg();
}

TestWorkspace::~TestWorkspace() {
	delete mfs;
	delete testEnvFs;
}

void TestWorkspace::jpgsCanBeCopiedIntoWorkspace() {
	wrapFileSystem(nullptr);
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
	wrapFileSystem(nullptr);
}

void TestWorkspace::nonJpgImagesCannotBeCopied() {
	wrapFileSystem(nullptr);
	wrapFileSystem(testEnvFs);
	wrapFileSystem(mfs);
	try {
		TemporaryWorkspaceFile wf("/tmp/image.png",
				WorkspaceFileType::image());
	} catch (UiException& e) {
		QCOMPARE(e.warning(), UiException::unsupportedImageType);
		wrapFileSystem(nullptr);
		return;
	} catch (...) {
	}
	QFAIL("Copying .PNG image to workspace should not be OK");
	wrapFileSystem(nullptr);
}
