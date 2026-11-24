/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QTCAMERAGRABBER_H
#define QTCAMERAGRABBER_H

#include <QCameraDevice>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QObject>
#include <QVideoSink>

#include <memory>

class QByteArray;
class QCamera;
class QString;
class QVideoFrame;

/** Values of the preference "grabberbackend". */
enum GrabberBackend {
	qtMultimediaBackend = 0,
	commandLineBackend = 1
};

/**
 * Shows a camera through Qt Multimedia.
 */
class QtCameraGrabber : public QObject {
	Q_OBJECT
public:
	explicit QtCameraGrabber(QObject* parent = nullptr);
	~QtCameraGrabber() override;
	/**
	 * Starts the camera with the ID @a deviceId, or the default camera if no
	 * camera has this ID. On Linux, the ID is the path of the video device.
	 * @return false if there is no camera.
	 */
	bool start(const QByteArray& deviceId);
	/**
	 * @return The newest image from the camera, or a null image if none has
	 * arrived yet.
	 */
	const QImage& latestImage() const;

Q_SIGNALS:
	void imageChanged(const QImage& image);
	/**
	 * The camera reported an error or was disconnected.
	 * @param detail A description of the error, if there is one.
	 */
	void failed(const QString& detail);

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
