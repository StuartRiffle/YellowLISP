#include "Yellow.h"
#include "Interpreter.h"
#include "Console.h"

Interpreter::Interpreter() :
    _interactive(false)
{
}

Interpreter::~Interpreter()
{
}

void Interpreter::PrintErrorMessage(const string& desc, const string& message)
{
    std::cout << desc << ": " << message << std::endl;
}

void Interpreter::EvaluateExpressions(const list<NodeRef>& exps)
{
    try
    {
        for (auto& node : exps)
        {
            try
            {
                CELL_INDEX exprCell  = _runtime.EncodeSyntaxTree(node);
                CELL_INDEX valueCell = _runtime.EvaluateCell(exprCell);

                if (_interactive)
                {
                    string output = _runtime.GetPrintedValue(valueCell);
                    std::cout << output << std::endl;
                }
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


void Interpreter::RunSourceCode(const string& source)
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

        PrintErrorMessage(desc.str(), error._message);
        std::cout << error._extraInfo << std::endl;

        if (!_interactive)
            exit(RETURN_PARSING_ERROR);
    }
}

void Interpreter::REPL()
{
    _interactive = true;

    for (;;)
    {
        string source;

        SetTextColor(ANSI_YELLOW);
        std::cout << "> ";
        ResetTextColor();

        std::getline(std::cin, source);

        RunSourceCode(source);
    }
}
