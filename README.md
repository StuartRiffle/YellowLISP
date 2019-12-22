Do you know what this world could *really* use right now?

That's right. Another fucking LISP. One that will never quite be finished, and which only barely works in the first place.

Well, I got you fam.

# YellowLISP

This is a LISP interpreter, written in C++11 for 64-bit machines. Of all the toy LISP implementations out there, it is easily the most yellow, but beyond that I don't know.

YellowLISP **is**:
- small, portable, dependency free, namespaced, and available as a single header file, making it suitable for embedding or use as a configuration language
- thread safe (in that it uses no global state, and access to the interpreter is serialized)
- able to run simple LISP programs (I am aware that this is a low bar)

It is **not**:
- an exercise in minimalism, because down that road is madness
- multi-threaded within a single instance of the interpreter
- suitable for doing actual work yet; maybe check out [Clojure](https://clojure.org/) or [Scheme](https://www.call-cc.org/)?

## Implementation details

A CONS cell fits in one 64-bit machine word. 32 bits are used for data (CDR), and 26 bits to link to the next cell (CAR). The rest contain flags and type information.

Cell references are stored as indices into a big table, not as pointers. This allows for up to 2^26 cells, which is about 64 million of them, having a maximum footprint of 512MB. It would probably be better to use 29 bits for each field, and just accept losing 3 bits of precision for numeric literals, to maximize the address space (2^29 is a half billion cells). But I wanted to keep things simple to start.

The interpreter works in the normal way: it parses source into an AST of variant nodes, encodes that AST into CONS cells, and then evaluates them. Lexical scoping is implemented by keeping a stack of symbol overrides as EVAL calls itself.

Square brackets and parentheses are interchangeable. I like brackets better because you don't have to press shift to type them, and I can read code more easily for some reason.

Garbage collection is mark-and-sweep, and considers everything that's in the global scope, or referenced by the function scopes of the current callstack, to be reachable. If GC fails to free at least 10% of the cell table capacity, the table is expanded by 1.5x, to avoid situations where an almost-full cell table triggers GC over and over again.

Tiny string values (4 characters or less) are stored directly in the CONS cell. Longer ones are stored in a string table and indexed. Identical strings are reference counted, and released by the GC when no longer needed.

Runtime errors are handled as exceptions, and are caught by the REPL. Asserts and unexpected C++ exceptions are surfaced as "internal" errors. Those cases are bugs and need fixing.

## Building

On **Linux** (or anything POSIX), build using [CMake](https://cmake.org/) by going into the `Build` folder and typing this:
```
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
On **Windows**, you can also use CMake, but it's probably easier to build from the [Visual Studio](https://visualstudio.microsoft.com/vs/community/) solution in the root of the repository.

Either way, you will find the executable in the `Build` folder.

## Running

Launching the executable with no parameters will give you an interactive prompt (the REPL).

If any parameters are LISP files, with the extension `.lisp`, they will be loaded and evaluated in the order given. The executable will exit after that.

## Embedding

You can embed YellowLISP into a larger program by including one header file:

```
#include "YellowLISP/Source/Embedded.h"

void HelloYellow()
{
	YellowLISP::Interpreter lisp;
	printf("%s\n", lisp.Evaluate("(+ 1 2 3)"));
}
```

## Feature support

- [x] Runs and appears to work so far
- [x] Lexical scoping
- [x] Garbage collection
- [ ] Backquotes
- [ ] Macros
- [ ] Tail call recursion
- [ ] Debugging

### Primitives

- [x] Fundamental (atom car cdr cons eq eval quote)
- [ ] Arithmetic (add sub mul div ...)
- [ ] Comparison (ne gt ge lt le ...)

### Native types

- [x] Integer
- [x] Floating point
- [ ] Character
- [x] String
- [x] List
- [ ] Vector

## Unsolicited opinions

I started this project thinking that LISP-type languages were a kind of ivory tower of ideological purity. But "special forms" seems like a nice way to say "dirty hacks", and a few of them are required. I feel like I'm missing something.

LISP beginners (like me) are also annoyed by all of the parentheses and the anachronistic identifiers. Experts think that those concerns are silly, and indicate a weak spirit. But non-trivial LISP is basically unreadable, even once you learn to sight-read "cddadr", which I have not. I think this is a real liability, and limits its use for quick-and-dirty scripting.

## Status

YellowLISP is still experimental, and is **not** production-ready, or good. So I appreciate you reading this far!

