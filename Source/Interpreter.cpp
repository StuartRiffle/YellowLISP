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

vector<CELL_INDEX> Interpreter::EvaluateExpressions(const list<NodeRef>& exps)
{
    std::unique_lock<std::mutex> lock(_mutex);

    vector<CELL_INDEX> outputs;
    outputs.reserve(exps.size());

    try
    {
        for (auto& node : exps)
        {
            try
            {
                CELL_INDEX exprCell  = _runtime.EncodeSyntaxTree(node);
                CELL_INDEX valueCell = _runtime.EvaluateCell(exprCell);

                outputs.push_back(valueCell);
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

    return outputs;
}


CELL_INDEX Interpreter::RunSourceCode(const string& source)
{
    std::unique_lock<std::mutex> lock(_mutex);

    try
    {
        list<NodeRef> exps = _parser.ParseExpressions(source);
        vector<CELL_INDEX> values = EvaluateExpressions(exps);

        if (!values.empty())
            return values.back();
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

    return 0;
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

        CELL_INDEX valueCell = RunSourceCode(source);
        string output = _runtime.GetPrintedValue(valueCell);

        std::cout << output << std::endl;
    }
}


