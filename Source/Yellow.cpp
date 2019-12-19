// YellowLISP (c) 2019 Stuart Riffle

#include "Yellow.h"
#include "Interpreter.h"

#define COLOR_TITLE  "\x1b[30m\x1b[43m" // Black on yellow
#define COLOR_PROMPT "\x1b[33m"         // Yellow
#define COLOR_ERROR  "\x1b[31m"         // Red
#define COLOR_RESET  "\x1b[0m"

void PrintErrorMessage(const string& message)
{
    std::cout << COLOR_ERROR << message << COLOR_RESET << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << 
        COLOR_TITLE << " YellowLISP " << 
        VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << " " <<
        COLOR_RESET << std::endl;

    std::cout << "(c) 2019 Stuart Riffle" << std::endl;
    std::cout << std::endl;

    Interpreter lisp;

    // TODO: handle running files from the 
    lisp.REPL();

    return RETURN_SUCCESS;
}
