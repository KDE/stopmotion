/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "cameragrabber.h"

CameraGrabber::CameraGrabber(QObject* parent) : QObject(parent) {
}

CameraGrabber::~CameraGrabber() {
}

const QString& CameraGrabber::errorDetail() const {
	return detail;
}
