/*
 * SPDX-FileCopyrightText: 2019 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

class UiException;

class ErrorHandler {
protected:
  virtual ~ErrorHandler() = 0;
public:
  virtual void error(UiException) = 0;
  /**
   * Returns an ErrorHandler that just throws any exception it gets
   * immediately.
   * @return The error handler; ownership is not returned.
   */
	static ErrorHandler* getThrower();
};

#endif
