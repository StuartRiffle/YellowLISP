Do you know what this world could *really* use right now?

That's right. Another fucking LISP. One that will never quite be finished, and which only barely works in the first place.

Well, I got you fam.

# YellowLISP

This is a LISP interpreter written in C++14. It is intended to be used as an embedded interpreter for scripting.

Of all the pet LISP implementations out there, it is easily the most yellow.

## Status

Working so far are:
- Basic CL primitives and semantics
- First class functions
- Lexical scoping
- Macros, nested backquotes
- Constant expression folding
- Mark-and-sweep garbage collection
- REPL with pretty error messages

Planned or in progress:
- Tail call recursion 
- Foreign function interface
- Generational GC
- Debugging from the REPL

## Building

On **Linux** (or anything POSIX), build using [CMake](https://cmake.org/) by going into the `Build` folder and typing this:
```
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
On **Windows**, you can also use CMake, but it's probably easier to build from the [Visual Studio](https://visualstudio.microsoft.com/vs/community/) solution in the root of the repository.

Either way, you will find the executable in the `Build` folder.

## Embedding

You can embed YellowLISP into a larger program by including one header file. There is nothing to link.

```
#include "YellowLISP/Embedded.h"

void HelloYellow()
{
    YellowLISP::Interpreter lisp;
    std::string result = lisp.Evaluate("(* 123 456)");
    printf("%s\n", result.c_str());
}
```

## Running

Launching the executable with no parameters will give you an interactive prompt (the REPL).

Files with the extension `.lisp` will be loaded and evaluated in the order given, after which the executable will exit.

Run `YellowLISP --help` for a complete list of options.

