/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GPHOTOGRABBER_H
#define GPHOTOGRABBER_H

#include "cameragrabber.h"

#include <QString>

#include <atomic>
#include <mutex>
#include <thread>

struct _Camera;
struct _GPContext;

/**
 * Shows the live view of a camera through libgphoto2, typically a DSLR or
 * system camera on USB, and captures photos at full resolution. The camera
 * is used from a thread of its own, as its functions block.
 */
class GPhotoGrabber final : public CameraGrabber {
	Q_OBJECT
public:
	explicit GPhotoGrabber(QObject* parent = nullptr);
	~GPhotoGrabber() override;
	/** Starts the first camera found; @a deviceId is not used. */
	bool start(const QByteArray& deviceId) override;
	/**
	 * Takes a photo with the camera. The first call requests it, later calls
	 * return true once it is stored.
	 */
	bool storeImage(const QString& path) override;

private:
	enum CaptureState { idle, requested, capturing, stored };
	void run();
	bool preview();
	bool capture(const QString& path);
	QString errorText(int result) const;

	_Camera* camera = nullptr;
	_GPContext* context = nullptr;
	std::thread worker;
	std::atomic<bool> stopping{false};
	std::mutex mutex;
	CaptureState captureState = idle;
	QString capturePath;
};

#endif
