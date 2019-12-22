#include "Yellow.h"

void TestInterpreter()
{
    const char* expectedOutputs[] =
    {
        "",                     "",
        "t",                    "t",
        "nil",                  "nil",
        "()",                   "nil",
        "1",                    "1",
        "2.34",                 "2.34",
        "56E-3",                "0.056",
        "foo",                  "foo",
        "FOO",                  "FOO",
        "\"foo\"",              "\"foo\"",
        "\"FOO\"",              "\"FOO\"",
        "(atom 3)",             "t",
        "(atom (atom 3))",      "t",
        "(atom '(atom 3))",     "nil",
        "(atom 'atom)",         "t",

        "(progn (setq a 7) a)", "7",

        // TODO: test error cases too
    };

#if 0
    // Run the tests twice to make sure we're not leaking cells

    TestInterpreter(interpreter);
    interpreter.CollectGarbage();

    size_t cellsInUse = interpreter.GetCellsInUse();

    TestInterpreter(interpreter);
    interpreter.CollectGarbage();

    assert(cellsInUse == interpreter.GetCellsInUse());
#endif
}

