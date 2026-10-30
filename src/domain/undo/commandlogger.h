/*
 * SPDX-FileCopyrightText: 2013-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef COMMANDLOGGER_H_
#define COMMANDLOGGER_H_

/**
 * For receiving the command's string representation during calls to Make
 * methods.
 */
class CommandLogger {
protected:
	virtual ~CommandLogger() = 0;
public:
	/**
	 * Records text representing the command about to be executed.
	 * If any command, undo or redo has previously been recorded but not
	 * committed, it is discarded.
	 * @par
	 * This function is allowed to throw an exception. The written command is
	 * not to be committed to the log until the @ref commit method is called.
	 * @par
	 * @ref commit needs to be called later, when the command has been
	 * successfully executed.
	 * @param text The text of the command to be written to the log. Should
	 * not contain any nulls or line delimiters.
	 */
	virtual void writePendingCommand(const char* text) = 0;
	/**
	 * Records that an undo is about to happen. Any previous uncommitted
	 * command or undo or redo is discarded. The undo is not committed until
	 * @ref commit is called later.
	 */
	virtual void writePendingUndo() = 0;
	/**
	 * Records that a redo is about to happen. Any previous uncommitted
	 * command or undo or redo is discarded. The redo is not committed until
	 * @ref commit is called later.
	 */
	virtual void writePendingRedo() = 0;
	/**
	 * Indicates that the command, undo or redo has been successfully executed.
	 */
	virtual void commit() = 0;
	/**
	 * Writes committed commands to the file. This will only have an effect if
	 * a previous call to commit() threw an exception.
	 */
	virtual void flush() = 0;
};

#endif /* COMMANDLOGGER_H_ */
