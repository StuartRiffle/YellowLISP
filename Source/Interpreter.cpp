#include "Yellow.h"
#include "Interpreter.h"
#include "Console.h"

Interpreter::Interpreter(const InterpreterSettings* settings)
{
    if (settings)
        _settings = *settings;
}

void Interpreter::PrintErrorMessage(int code, const string& desc, const string& message)
{
    std::cout << desc << " " << code << ": " << message << std::endl;
}

vector<CELL_INDEX> Interpreter::EvaluateExpressions(const list<NodeRef>& exps)
{
    vector<CELL_INDEX> outputs;
    outputs.reserve(exps.size());

    for (auto& node : exps)
    {
        CELL_INDEX exprCell  = _runtime.EncodeSyntaxTree(node);
        CELL_INDEX valueCell = _runtime.EvaluateCell(exprCell);

        outputs.push_back(valueCell);
    }

    return outputs;
}


CELL_INDEX Interpreter::RunSourceCode(const string& source)
{
    list<NodeRef> exps = _parser.ParseExpressions(source);
    vector<CELL_INDEX> values = EvaluateExpressions(exps);

    if (!values.empty())
        return values.back();

    return 0;
}

string Interpreter::Evaluate(const string& source)
{
#if YELLOW_THREAD_SAFE
    std::lock_guard<std::recursive_mutex> lock(_mutex);
#endif

    string output;

    try
    {
        CELL_INDEX valueCell = RunSourceCode(source);
        output = _runtime.GetPrintedValue(valueCell);
    }
    catch (YellowError error)
    {
    #if !YELLOW_CATCH_EXCEPTIONS
        throw error;
    #endif

        if (!_settings._catchExceptions)
            throw error;

        output = error.what();

        SetTextColor(ANSI_RED);
        std::cout << error.what() << std::endl;
        ResetTextColor();
    }
    catch (std::exception error)
    {
    #if !YELLOW_CATCH_EXCEPTIONS
        throw error;
    #endif

        if (!_settings._catchExceptions)
            throw error;

        output = error.what();

        SetTextColor(ANSI_RED);
        std::cout << "CRITICAL ERROR: unhandled exception" << std::endl;
        ResetTextColor();
    }

    return output;
}

void Interpreter::RunREPL()
{
    _settings._repl = true;

    for (;;)
    {
        string source;

        SetTextColor(ANSI_YELLOW);
        std::cout << "-> ";
        ResetTextColor();

        std::getline(std::cin, source);

        string output = Evaluate(source);
        std::cout << output << std::endl;
    }
}


