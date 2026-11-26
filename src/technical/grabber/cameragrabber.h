/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CAMERAGRABBER_H
#define CAMERAGRABBER_H

#include <QObject>
#include <QString>

class QByteArray;
class QImage;

/** Values of the preference "grabberbackend". */
enum GrabberBackend {
	qtMultimediaBackend = 0,
	commandLineBackend = 1,
	gphoto2Backend = 2
};

/**
 * A camera that delivers its images itself, as opposed to the external
 * programs.
 */
class CameraGrabber : public QObject {
	Q_OBJECT
public:
	explicit CameraGrabber(QObject* parent = nullptr);
	~CameraGrabber() override;
	/**
	 * Starts the camera.
	 * @param deviceId The camera to use, if the backend supports choosing.
	 * @return false if there is no camera; @ref errorDetail may say why.
	 */
	virtual bool start(const QByteArray& deviceId) = 0;
	/**
	 * Writes the image to capture to a JPEG file.
	 * @return true once the file holds the image. false if it is not ready
	 * yet; call again later with the same path.
	 */
	virtual bool storeImage(const QString& path) = 0;
	/** Why @ref start failed, if known. */
	const QString& errorDetail() const;

Q_SIGNALS:
	void imageChanged(const QImage& image);
	/**
	 * The camera reported an error or was disconnected.
	 * @param detail A description of the error, if there is one.
	 */
	void failed(const QString& detail);

protected:
	QString detail;
};

#endif
