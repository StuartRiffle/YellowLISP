// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Bootstrap.h"
#include "Interpreter.h"
#include "Console.h"

Interpreter::Interpreter(const InterpreterSettings* settings)
{
    if (settings)
        _settings = *settings;

    RunSourceCode(gBootstrapCode);
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
        //_parser.DumpSyntaxTree(node);

        CELL_INDEX valueCell = _runtime.EvaluateCell(exprCell);
        outputs.push_back(valueCell);    
    }

    return outputs;
}

CELL_INDEX Interpreter::RunSourceCode(const string& source)
{
    list<NodeRef> exps = _parser.ParseExpressionList(source);

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
        throw;
    #endif

        if (!_settings._catchExceptions)
            throw;

        SetTextColor(ANSI_RED);
        std::cout << "ERROR " << error._code << ": ";
        ResetTextColor();

        std::cout << error.what() << std::endl;
    }
    catch (std::exception error)
    {
    #if !YELLOW_CATCH_EXCEPTIONS
        throw;
    #endif

        if (!_settings._catchExceptions)
            throw;

        SetTextColor(ANSI_RED);
        std::cout << "CRITICAL ERROR: "; 
        ResetTextColor();

        std::cout << "unhandled error (this is not your fault)" << std::endl;
        std::cout << error.what() << std::endl;
    }
    catch (...)
    {
        SetTextColor(ANSI_RED);
        std::cout << "UNHANDLED EXCEPTION" << std::endl; 
        ResetTextColor();
    }

    // This is a safe place to perform garbage collection. We can't do
    // it during evaluation, because there will be temporary cells in
    // use that aren't reachable yet. 

    _runtime.HandleGarbage();

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
        std::getline(std::cin, source);
        ResetTextColor();

        string output = Evaluate(source);

        if (output.length() > 0)
            std::cout << output << std::endl;
    }
}


