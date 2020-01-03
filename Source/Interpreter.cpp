// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Bootstrap.h"
#include "Interpreter.h"
#include "Console.h"
#include "Testing.h"

Interpreter::Interpreter(Console* console, const InterpreterSettings* settings) :
    _parser(console),
    _runtime(console)
{
    static DummyConsole dummyConsole;

    _console = console? console : &dummyConsole;

    if (settings)
        _settings = *settings;

    //RunSourceCode(gBootstrapCode);
}

vector<CELLID> Interpreter::EvaluateExpressions(const list<NodeRef>& exps)
{
    vector<CELLID> outputs;
    outputs.reserve(exps.size());

    for (auto& node : exps)
    {
        CELLID exprCell  = _runtime.EncodeSyntaxTree(node);
        //_parser.DumpSyntaxTree(node);

        CELLID valueCell = _runtime.EvaluateCell(exprCell);
        outputs.push_back(valueCell);    
        TEST_COVERAGE;
    }

    return outputs;
}

CELLID Interpreter::RunSourceCode(const string& source)
{
    CELLID result;

    list<NodeRef> exps = _parser.ParseExpressionList(source);
    vector<CELLID> values = EvaluateExpressions(exps);

    if (!values.empty())
    {
        result = values.back();
        TEST_COVERAGE;
    }

    return result;
}

string Interpreter::Evaluate(const string& source)
{
#if YELLOW_THREAD_SAFE
    std::lock_guard<std::recursive_mutex> lock(_mutex);
#endif

    string output;

    try
    {
        CELLID valueCell = RunSourceCode(source);
        if (valueCell.IsValid())
            output = _runtime.GetPrintedValue(valueCell);
    }
    catch (YellowError error)
    {
        TEST_COVERAGE;

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
        TEST_COVERAGE;

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
        TEST_COVERAGE;

        _console->PrintErrorPrefix("INTERNAL ERROR");
        _console->Print("unhandled exception\n");
    }

    // This is a safe place to perform garbage collection. We can't do
    // it during evaluation, because there will be temporary cells in
    // use that aren't reachable yet. 

    //_runtime.HandleGarbage();

    RETURN_WITH_COVERAGE(output);
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

        TEST_COVERAGE;
    }
}


