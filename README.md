Do you know what this world could *really* use right now?

That's right. Another fucking LISP. One that will never quite be finished, and which only barely works in the first place.

Well, I got you fam.

# YellowLISP

This is a LISP interpreter, written in C++11 for 64-bit machines. I started the project as a way to learn the language. Of all the toy LISP implementations out there, it is easily the most yellow.

## Status

Working so far are:
- Basic CL primitives and semantics
- First class functions
- Lexical scoping
- Macros, nested backquotes
- Constant expression folding
- Mark-and-sweep garbage collection
- REPL with pretty error messages

In progress:
- Tail call recursion 
- Foreign function interface

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

Run `YellowLISP --help` for a complete list of options.
