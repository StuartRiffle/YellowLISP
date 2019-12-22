// YellowLISP (c) 2019 Stuart Riffle

#include "Yellow.h"
#include "Interpreter.h"
#include "Console.h"

int main(int argc, char** argv)
{
    //SetTextColor(ANSI_YELLOW);
    printf("YellowLISP %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    ResetTextColor();

    Interpreter lisp;
    lisp.REPL();

    return RETURN_SUCCESS;
}
