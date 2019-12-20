#pragma once
#include "Yellow.h"
#include "Parser.h"
#include "Runtime.h"

class Interpreter
{
    Parser _parser;
    Runtime _runtime;
    bool _interactive;

    void PrintErrorMessage(const string& desc, const string& message);
    void EvaluateExpressions(const list<NodeRef>& exps);

public:
    Interpreter();
    ~Interpreter();

    void RunCode(const string& source);
    void REPL();
};
