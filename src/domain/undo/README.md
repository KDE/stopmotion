The Undo System
===============

This document is to help developers understand the architecture
of Stopmotion and write their own commands.

Stopmotion's undo system is designed to allow not only Undo and Redo
of any and all operations, but to write those operations to a file
so that any crash can be recovered from. The idea is that the system
should be so reliable that Stopmotion does not need a "do you want
to save before exiting?" dialog; the app can simply close and
recover to exactly the same point by reading the undo history log
next time it opens. The user therefore need not bother to name her
project until she wants to change to a different project later, and
the crash recovery system gets a constant workout as it is used for
every app startup.

Stopmotion can do this because each command is invoked by
specifying its name and parameters (rather than by calling a
method or passing an object, for example). In normal operation this
specification is written to a file (the 'undo history log'). An undo
or redo is also written to this log. When recovering, the
specifications are read from this file in order and so the same
sequence of operations is reconstructed and applied to the project.
This results in a project in exactly the same state as when the app
was closed (or crashed!), even down to having the same commands on
its undo and redo stacks.

In order to achieve this, the app has some tricky code in
`executor.cpp`, but the most important factor in making it work
is that every possible change to the project is performed through a
Command object that adheres to strict rules.

No changes to the project except through commands
-------------------------------------------------

There must be no direct change to the project. All changes are
mediated through the `Animation` class, which always calls the
`ConcreteExecutor` object in order to achieve real work. The
`ConcreteExecutor` knows which command factory to call based on the
name of the command it needs to produce. This allows it to work the
same way whether the instruction came from the `Animation` class or
from the undo history log.

There can be no command that does not allow undo; there can be no
"Are you sure you want to do this? It cannot be undone" dialog.

All commands have a factory that produces them
----------------------------------------------

`addallcommands.cpp` contains a function
`makeAnimationCommandExecutor` that adds each factory (with its
command's name) to the `ConcreteExecutor`. This enables a line
of text from the undo history log to construct the correct command.
Each command factory derives from `CommandFactory`. Factories must
produce commands from parameters read from a `Parameters` object.
This parameters object either came from the `Animation` class or from
the recovery system. Normally this is a standard parameters object,
but some derive their own to help the `Animation` class avoid memory
leaks-- do not worry about this, it is almost never a good idea.

All commands produce their own inverse
--------------------------------------

Commands derive from the `Command` class. `Command` only has two
methods. The first, `accept`, is not very interesting. It is used to
tell a `FileNameVisitor` which files are owned by this command. It is
used for testing.

The second, `execute`, is the heart and soul of the undo system. This
method performs three actions:
* Perform the command (altering the project)
* Construct an inverse of the command
* Delete itself
And it must do these operations carefully. There must be no memory
allocation or other possibility of exceptions being thrown from the
moment the project starts to be altered until the method ends. This
means constructing the inverse and reserving any memory that will be
required before changing the project in any way. It also means that
if a command cannot be completed in one step without the possibility
of leaving the project in a half-way state, the developer should
re-think the design, possibly it should be broken up into smaller
commands that can alter the project without risking leaving it in a
half-way state.

Many commands come in pairs. A 'remove' command takes something out
of the project and stores it in an 'add' command (its inverse). The
'add' command adds it to the project and stores its position in a
'remove' command.

Everything is fiercely tested
-----------------------------

Any command that fulfils the above criteria is automatically
thoroughly tested. The function `testUndo` in `testundo.cpp`
creates random commands with random parameters and tests that these
strings of commands obey all the rules required to make the undo
system bulletproof. `testUndo` is called twice: First it is called
with a simple set of commands for manipulating a string. This tests
that the undo system itself works. Secondly it is called with the
commands Stopmotion uses for its own projects. This tests that all
the commands work properly.

Run the tests by typing, at the project root:

    make test

`testUndo` works by constructing a tree of projects; each leaf of
the tree is reached by performing some set of actions. A set of
actions might be running a string of commands, or running a string
of commands where one memory access fails, or re-running commands
from the undo history log, or many other things. Certain pairs of
leaves of this tree should result in the same project; for example
running commands A, then clearing the undo history log, then
running commands B should give the same result as running commands
A then replaying the history log. Similarly, running commands C then
undoing them all then redoing them all should give the same result
as just running commands C.

I hope this helps others understand what is going on here! -Tim
