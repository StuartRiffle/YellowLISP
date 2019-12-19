// YellowLISP (c) 2019 Stuart Riffle

#include "Yellow.h"
#include "Bootstrap.h"
#include "Parser.h"
#include "Runtime.h"

#define COLOR_TITLE  "\x1b[30m\x1b[43m" // Black on yellow
#define COLOR_PROMPT "\x1b[33m"         // Yellow
#define COLOR_ERROR  "\x1b[31m"         // Red
#define COLOR_RESET  "\x1b[0m"

void REPL()
{

}

void PrintErrorMessage(const string& message)
{
    std::cout << COLOR_ERROR << message << COLOR_RESET << std::endl;
}

class Interpreter
{
    Parser _parser;
    Runtime _runtime;
    bool _interactive;

    Interpreter() : _interactive(false) {}

    void EvaluateExpressions(const list<NodeRef>& exps)
    {
        try
        {
            for (auto& node : exps)
            {
                try
                {
                    CELL_INDEX valueCell = _runtime.EvaluateSyntaxTree(node);
                    string output = runtime.PrintedValue(valueCell);
                    std::cout << output << std::endl;
                }
                catch (RuntimeError error)
                {
                    PrintErrorMessage("RUNTIME ERROR", error._message);

                    if (!_interactive)
                        exit(RETURN_RUNTIME_ERROR);

                    break;
                }
            }
        }
        catch (std::exception e)
        {
            PrintErrorMessage("INTERNAL ERROR", e.what());
            exit(RETURN_INTERNAL_ERROR);
        }
    }

    void RunCode(const string& source)
    {
        try
        {
            list<NodeRef> exps = _parser.ParseExpressions(source);
            EvaluateExpressions(exps);
        }
        catch (ParsingError error)
        {
            std::stringstream desc;
            desc << "PARSING ERROR (line " << error._line << ")";

            std::stringstream message;
            message << error._message << std::endl << error._extraInfo;

            PrintErrorMessage(desc.str(), message.str());

            if (!_interactive)
                exit(RETURN_PARSING_ERROR);
        }
    }

    void REPL()
    {
        for (;;)
        {
            std::cout << COLOR_PROMPT << "> " << COLOR_RESET;

            string source;
            std::getline(std::cin, source);

            RunCode(source);
        }
    }
};

void main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    std::cout << 
        COLOR_TITLE << " YellowLISP " << 
        VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << " " <<
        COLOR_RESET << std::endl;

    std::cout << "(c) 2019 Stuart Riffle" << std::endl;
    std::cout << std::endl;

    Interpreter lisp;
    lisp.REPL();
}
