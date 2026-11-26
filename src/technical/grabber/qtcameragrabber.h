/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QTCAMERAGRABBER_H
#define QTCAMERAGRABBER_H

#include "cameragrabber.h"

#include <QCameraDevice>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QVideoSink>

#include <memory>

class QByteArray;
class QCamera;
class QString;
class QVideoFrame;

/**
 * Shows a camera through Qt Multimedia.
 */
class QtCameraGrabber final : public CameraGrabber {
	Q_OBJECT
public:
	explicit QtCameraGrabber(QObject* parent = nullptr);
	~QtCameraGrabber() override;
	/**
	 * Starts the camera with the ID @a deviceId, or the default camera if no
	 * camera has this ID. On Linux, the ID is the path of the video device.
	 */
	bool start(const QByteArray& deviceId) override;
	/** Stores the newest image from the camera. */
	bool storeImage(const QString& path) override;

private:
	void frameChanged(const QVideoFrame& frame);
	void videoInputsChanged();

	QMediaDevices mediaDevices;
	QVideoSink sink;
	QMediaCaptureSession session;
	// destroyed before the session it is part of
	std::unique_ptr<QCamera> camera;
	QCameraDevice device;
	QImage image;
};

#endif
