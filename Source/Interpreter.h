// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"
#include "Parser.h"
#include "Runtime.h"
#include "Console.h"

struct InterpreterSettings
{
    bool _debugMode         = false;
    bool _repl              = false;
    bool _catchExceptions   = true;
};

class Interpreter
{
    InterpreterSettings _settings;

    Console* _console;
    Parser   _parser;
    Runtime  _runtime;

    std::recursive_mutex _mutex;

    void PrintErrorMessage(int code, const string& desc, const string& message);

    vector<CELLID> EvaluateExpressions(const list<NodeRef>& exps);
    CELLID RunSourceCode(const string& source);

public:
    Interpreter(Console* console = nullptr, const InterpreterSettings* settings = nullptr);

    string Evaluate(const string& source);
    void RunREPL();
};
