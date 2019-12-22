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
- multi-threaded within a single instance
- suitable for doing actual work yet; maybe check out [Clojure](https://clojure.org/) or [Scheme](https://www.call-cc.org/)?

## Implementation details

A CONS cell fits in one 64-bit machine word. 32 bits are used for data (cdr), and 26 bits to link to the next cell (car). The rest contain flags and type information.

Cell references are stored as indices into a big table, not as pointers. This allows for up to 2^26 cells, which is about 64 million of them, having a maximum footprint of 512MB. It would probably be better to use 29 bits for each field, and just accept losing 3 bits of precision for numeric literals, to maximize the address space (2^29 is four billion cells). But I wanted to keep things simple to start.

The interpreter works in the normal way: it parses source into an AST of variant nodes, encodes that AST into CONS cells, and then evaluates them. Lexical scoping is implemented by keeping a map of symbol overrides on the stack as EVAL calls itself.

Square brackets and parentheses are interchangeable. I like brackets better because you don't have to press shift to type them, and I can read code more easily for some reason.

Garbage collection is mark-and-sweep, and considers everything that's in the global scope, and in the function scopes of the current callstack, to be reachable. If GC fails to free at least 10% of the cell table capacity, the table is expanded by 1.5x, to avoid situations where an almost-full cell table triggers GC over and over again.

Tiny strings (4 characters or less) are stored directly in the CONS cell. Longer ones are stored in a string table, and just indexed by the CONS cell. Duplicate strings are pooled and reference counted, and released by the GC when no longer in use.

Runtime errors are handled as exceptions, and are caught by the REPL. Asserts and unexpected C++ exceptions are surfaced as "internal" errors. Those cases are bugs and need fixing.

## Building

On **Windows**, build and run the Visual Studio solution in the root of the repository.

On **Linux** (or anything else), build using CMake by going into the `Build` folder and typing this:
```
	cmake -DCMAKE_BUILD_TYPE=Release ..
	make
```

## Embedding

You can embed YellowLISP into a larger program by including one header file, like this:

```
#include "YellowLISP/Source/Embedded.h"

void HelloYellow()
{
	YellowLISP::Interpreter lisp;
	printf("%s\n", lisp.Evaluate("(print (+ 1 2 3))"));
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

I started this project thinking that LISP-type languages were a kind of ivory tower of ideological purity. But "special forms" seems like a nice way to say "dirty hacks", and plenty of them are required. I feel like I'm missing something.

LISP beginners (like me) are also annoyed by all of the parentheses and the anachronistic identifiers. Experts think that those concerns are silly, and indicate a weak spirit. But non-trivial LISP is unreadable, even once you learn to sight-read "cddadr" (which I have not). I think this is a real liability, and limits its use for quick-and-dirty scripting.

## Status

YellowLISP is very much a work in progress, and is **not** production-ready, or good. So I appreciate you reading this far!

