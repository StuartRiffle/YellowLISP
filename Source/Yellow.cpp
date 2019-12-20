// YellowLISP (c) 2019 Stuart Riffle

#include "Yellow.h"
#include "Interpreter.h"

int main(int argc, char** argv)
{
    std::cout << 
        COLOR_TITLE << " YellowLISP " << 
        VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << " " <<
        COLOR_RESET << std::endl;

    std::cout << "(c) 2019 Stuart Riffle" << std::endl;
    std::cout << std::endl;

    Interpreter lisp;
    lisp.REPL();

    return RETURN_SUCCESS;
}
