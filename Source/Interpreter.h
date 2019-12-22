#pragma once
#include "Yellow.h"
#include "Parser.h"
#include "Runtime.h"

class Interpreter
{
    Parser  _parser;
    Runtime _runtime;
    bool    _interactive;

    std::recursive_mutex _mutex;

    void PrintErrorMessage(const string& desc, const string& message);
    vector<CELL_INDEX> EvaluateExpressions(const list<NodeRef>& exps);

public:
    Interpreter();
    ~Interpreter();

    CELL_INDEX RunSourceCode(const string& source);
    void REPL();
};
