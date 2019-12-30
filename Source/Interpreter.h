// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"
#include "Parser.h"
#include "Runtime.h"

struct InterpreterSettings
{
    bool _debugMode;
    bool _repl;
    bool _catchExceptions;

    InterpreterSettings() :
        _debugMode(false),
        _repl(false),
        _catchExceptions(true) {}
};

class Interpreter
{
    InterpreterSettings _settings;

    Parser  _parser;
    Runtime _runtime;

    std::recursive_mutex _mutex;

    void PrintErrorMessage(int code, const string& desc, const string& message);

    vector<CELL_INDEX> EvaluateExpressions(const list<NodeRef>& exps);
    CELL_INDEX RunSourceCode(const string& source);

public:
    Interpreter(const InterpreterSettings* settings = NULL);

    string Evaluate(const string& source);
    void RunREPL();
};
