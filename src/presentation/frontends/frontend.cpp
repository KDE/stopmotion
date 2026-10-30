/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "frontend.h"

Frontend::~Frontend() {
}

Frontend::CriticalError::CriticalError() {
}

Frontend::CriticalError::~CriticalError() noexcept {
}

const char* Frontend::CriticalError::what() const noexcept {
	return "Critical error. Program stopped.";
}
