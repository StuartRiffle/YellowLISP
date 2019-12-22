#include "Yellow.h"
#include "Console.h"
#include "Interpreter.h"

void CheckOutput(Interpreter& lisp, const char* source, const char* expectedOutput)
{
    string output = lisp.Evaluate(source);
    if (output != expectedOutput)
    {
        SetTextColor(ANSI_RED);
        printf("SANITY CHECK FAILED: ");
        ResetTextColor();

        printf("\"%s\" evaluated to \"%s\" instead of \"%s\"\n", source, output.c_str(), expectedOutput);
    }
}

bool SanityCheck()
{
    try
    {
        Interpreter lisp;

        CheckOutput(lisp, "",                   "");
        CheckOutput(lisp, "\n\n\n",             "");
        CheckOutput(lisp, "t",                  "t");
        CheckOutput(lisp, "nil",                "nil");
        CheckOutput(lisp, "()",                 "nil");
        CheckOutput(lisp, "1",                  "1");
        CheckOutput(lisp, "2.3",                "2.3");
        CheckOutput(lisp, "-4.5",               "-4.5");
        CheckOutput(lisp, "67e-3",              "0.067");
        CheckOutput(lisp, "foo",                "foo");
        CheckOutput(lisp, "FOO",                "FOO");
        CheckOutput(lisp, "\"foo\"",            "\"foo\"");
        CheckOutput(lisp, "\"FOO\"",            "\"FOO\"");
        CheckOutput(lisp, "(atom 3)",           "t");
        CheckOutput(lisp, "(atom 'atom)",       "t");
        CheckOutput(lisp, "(atom (atom 3))",    "t");
        CheckOutput(lisp, "(atom '(atom 3))",   "nil");

        return true;
    }
    catch (std::exception e)
    {
        SetTextColor(ANSI_RED);
        printf("SANITY CHECK FAILED: ");
        ResetTextColor();

        printf("%s\n", e.what());
    }

    return false;
}

