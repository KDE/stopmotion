/*
 * command.h
 */

#ifndef COMMAND_H_
#define COMMAND_H_

class Command;
class CommandHistoryAdder;

/**
 * Adds c to the history via a. For use in Command::Do.
 * Might throw an exception. If so, c is deleted.
 */
void AddToCommandHistory(CommandHistoryAdder& a, Command& c);

/**
 * Internal exception; command attempted to add two inverses to the history
 */
class CommandDoubleAddToHistory {
};

/**
 * Internal exception; attempt to remove an item from an empty command list
 */
class CommandListPopEmpty {
};

/**
 * Interface for visitors that want to find out which files are referenced by a
 * command or history.
 */
class FileNameVisitor {
protected:
	virtual ~FileNameVisitor();
public:
	virtual void AddFile(const char*) = 0;
};

/**
 * Base class of all command classes, objects of which are manipulated by the
 * undo system. Most commands should derive from CommandAtomic.
 */
class Command {
	Command(const Command&);
	Command& operator=(const Command&);
public:
	Command();
	virtual ~Command() = 0;
	/**
	 * Performs its action, calling 'adder' once only to add its inverse
	 * to the history. For exception safety the following conditions must
	 * be met: At each point where an exception might be thrown (1) the
	 * 'this' object must represent only the remaining parts of the action
	 * yet to be performed and (2) the object (already) added to 'adder'
	 * must represent the inverse of all the actions that have so far been
	 * performed successfully.
	 */
	virtual void Do(CommandHistoryAdder& adder) = 0;
	/**
	 * calls v.Add(f) for each file f referenced by the command
	 */
	virtual void Accept(FileNameVisitor& v) const;
};

/**
 * Base class of all 'atomic' command classes, that is, ones that will not
 * throw exceptions in their operation (except perhaps in a preparation method)
 */
class CommandAtomic : public Command {
public:
	// do not override further
	void Do(CommandHistoryAdder& adder);
	/**
	 * Perform the action itself, relinquishing ownership of any owned
	 * objects that have been passed to others (for example nulling their
	 * pointers), and returning an inverse command. Any exception thrown
	 * must be thrown before any part of the action has taken place.
	 */
	virtual Command& DoAtomic() = 0;
};

class CommandList;

/**
 * Command made up of other commands. If execution of the composite is
 * interrupted by an exception being thrown then (provided that all the
 * commands making it up are well-behaved in this regard) the command history
 * will be in the correct state: with all parts not yet performed still on the
 * redo stack (or undo if undoing) and the inverses of all the parts so far
 * performed on the undo stack (or redo if undoing).
 */
class CommandComposite : public Command {
	CommandList* cs;
public:
	CommandComposite();
	~CommandComposite();
	/**
	 * Add another command to the list (to be executed after all the
	 * previously added commands).
	 */
	void Add(Command&);
	void Do(CommandHistoryAdder& adder);
	void Accept(FileNameVisitor& v) const;
};

/**
 * Command history for undo and redo.
 */
class CommandHistory {
	CommandList* past;
	CommandList* future;
public:
	CommandHistory();
	~CommandHistory();
	/**
	 * Returns 'true' if and only if Undo will perform an action, i.e. if
	 * there are any actions in the history to undo.
	 */
	bool CanUndo();
	/**
	 * Returns 'true' if and only if Redo will perform an action, i.e. if
	 * there are any actions in the history to redo.
	 */
	bool CanRedo();
	/**
	 * Undoes the last action in the history, if any.
	 */
	void Undo();
	/**
	 * Redoes the next action in the history, if any.
	 */
	void Redo();
	/**
	 * Executes the command c, placing its inverse into the undo history,
	 * deleting the Redo history. Any partial composite function remaining
	 * after a thrown exception will be on the Redo stack.
	 */
	void Do(Command&);
	/**
	 * Calls v.Add(f) for each filename f referenced by the commands in the
	 * history.
	 */
	void Accept(FileNameVisitor& v) const;
};

#endif /* COMMAND_H_ */
