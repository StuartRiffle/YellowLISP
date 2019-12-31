// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Bootstrap.h"
#include "Interpreter.h"
#include "Console.h"

Interpreter::Interpreter(Console* console, const InterpreterSettings* settings) :
    _parser(console),
    _runtime(console)
{
    _console = console;

    if (settings)
        _settings = *settings;

    RunSourceCode(gBootstrapCode);
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

        _console->PrintErrorPrefix("ERROR", error._code);
        _console->Print("%s\n", error.what());
    }
    catch (std::exception error)
    {
    #if !YELLOW_CATCH_EXCEPTIONS
        throw;
    #endif

        if (!_settings._catchExceptions)
            throw;

        _console->PrintErrorPrefix("INTERNAL ERROR");
        _console->Print("%s\n", error.what());
    }
    catch (...)
    {
        _console->PrintErrorPrefix("INTERNAL ERROR");
        _console->Print("unhandled exception\n");
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
        _console->PrintColor(COLOR_YELLOW, 0, "-> ");

        string source = _console->ReadLine();
        string output = Evaluate(source);

        if (output.length() > 0)
            _console->Print("%s\n", output.c_str());
    }
}


