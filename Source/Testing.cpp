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

        printf("\"%s\" evaluates to \"%s\" instead of \"%s\"\n", source, output.c_str(), expectedOutput);
    }
}

bool SanityCheck()
{
    try
    {
        Interpreter lisp;

        CheckOutput(lisp, "",                   "");
        CheckOutput(lisp, "\n",                 "");
        CheckOutput(lisp, "1",                  "1");
        CheckOutput(lisp, " 2 ",                "2");
        CheckOutput(lisp, "3.4",                "3.4");
        CheckOutput(lisp, "-5.6",               "-5.6");
        CheckOutput(lisp, "78e-3",              "0.078");

        CheckOutput(lisp, "t",                  "t");
        CheckOutput(lisp, "nil",                "nil");
        CheckOutput(lisp, "'nil",               "nil");
        CheckOutput(lisp, "()",                 "nil");
        CheckOutput(lisp, "'()",                "nil");

        CheckOutput(lisp, "'foo",               "foo");
        CheckOutput(lisp, "'FOO",               "FOO");
        CheckOutput(lisp, "\"foo\"",            "\"foo\"");
        CheckOutput(lisp, "\"FOO\"",            "\"FOO\"");

        CheckOutput(lisp, "(atom ())",          "t");
        CheckOutput(lisp, "(atom '())",         "t");
        CheckOutput(lisp, "(atom 3)",           "t");
        CheckOutput(lisp, "(atom 'atom)",       "t");
        CheckOutput(lisp, "(atom (atom 3))",    "t");
        CheckOutput(lisp, "(atom '(atom 3))",   "nil");

        return true;
    }
    catch (std::exception e)
    {
        SetTextColor(ANSI_RED);
        printf("SANITY CHECK EXCEPTION: ");
        ResetTextColor();

        printf("%s\n", e.what());
    }

    return false;
}

