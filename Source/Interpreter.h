#pragma once
#include "Yellow.h"
#include "Parser.h"
#include "Runtime.h"

struct InterpreterSettings
{
    bool _debugMode;
    bool _repl;

    InterpreterSettings() :
        _debugMode(false),
        _repl(false) {}
};

class Interpreter
{
    InterpreterSettings _settings;

    Parser  _parser;
    Runtime _runtime;

    std::recursive_mutex _mutex;

    void PrintErrorMessage(const string& desc, const string& message);

    vector<CELL_INDEX> EvaluateExpressions(const list<NodeRef>& exps);
    CELL_INDEX RunSourceCode(const string& source);

public:
    Interpreter(const InterpreterSettings* settings = NULL);

    string Evaluate(const string& source);
    void RunREPL();
};
