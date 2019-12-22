#include "Yellow.h"
#include "Interpreter.h"

bool SanityCheck()
{
    Interpreter lisp;

    // Baby steps, Ellie

    assert(lisp.Evaluate("")                     == "");
    assert(lisp.Evaluate("\n\n\n")               == "");
    assert(lisp.Evaluate("t")                    == "t");
    assert(lisp.Evaluate("nil")                  == "nil");
    assert(lisp.Evaluate("()")                   == "nil");
    assert(lisp.Evaluate("1")                    == "1");
    assert(lisp.Evaluate("2.3")                  == "2.3");
    assert(lisp.Evaluate("-4.5")                 == "-4.5");
    assert(lisp.Evaluate("67e-3")                == "0.067");
    assert(lisp.Evaluate("foo")                  == "foo");
    assert(lisp.Evaluate("FOO")                  == "FOO");
    assert(lisp.Evaluate("(atom 3)")             == "t");
    assert(lisp.Evaluate("(atom (atom 3))")      == "t");
    assert(lisp.Evaluate("(atom '(atom 3))")     == "nil");

//    assert(lisp.Evaluate("\"foo\"") == "\"foo\"");
//    assert(lisp.Evaluate("\"FOO\"") == "\"FOO\"");
//    assert(lisp.Evaluate("(atom 'atom)") == "t");

    return true;
}

