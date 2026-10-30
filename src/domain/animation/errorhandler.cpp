/*
 * SPDX-FileCopyrightText: 2019 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "errorhandler.h"

#include "src/foundation/uiexception.h"

namespace {
class ThrowingErrorHandler : public ErrorHandler {
public:
	ThrowingErrorHandler() {
	}
	~ThrowingErrorHandler() override {
	}
	void error(UiException e) override {
		throw e;
	}
};
ThrowingErrorHandler singleton;
}

ErrorHandler* ErrorHandler::getThrower() {
  return &singleton;
}

ErrorHandler::~ErrorHandler() {
}
