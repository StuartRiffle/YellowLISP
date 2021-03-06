// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime(Console* console) : _console(console)
{
    // Cell 0 is unused and invalid. Attempting to access it will assert.
    // Cell 1 is nil, the empty list. Its _next field is 0, to prevent walking through it. 

    _cell.resize(2);

    _null = 1;
    _cell[_null]._type = TYPE_SYMBOL;
    StoreSymbol("nil", _null, SYMBOL_RESERVED);

    _cellFreeList  = 0;
    _cellFreeCount = 0;
    _garbageCollectionNeeded = false;

    _dot = RegisterReserved(".");
    _true = RegisterReserved("#t");
    _false = RegisterReserved("#f");
    _quote = RegisterPrimitive("quote", nullptr, SYMBOLFLAG_DONT_EVAL_ARGS);
    _unquote = RegisterPrimitive("unquote", nullptr, SYMBOLFLAG_DONT_EVAL_ARGS);
    _quasiquote = RegisterPrimitive("quasiquote", nullptr, SYMBOLFLAG_DONT_EVAL_ARGS);

    // R5RS fundamental forms: define, lambda, quote, if, define-syntax, let-syntax, letrec-syntax, syntax-rules, set!
    // R5RS derived forms:     do, let, let*, letrec, cond, case, and, or, begin, named let, delay, unquote, unquote-splicing, quasiquote


    // Language primitives

    RegisterPrimitive("apply",      &Runtime::APPLY);
    RegisterPrimitive("atom",       &Runtime::ATOM);
    RegisterPrimitive("begin",      &Runtime::BEGIN,   SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("car",        &Runtime::CAR);
    RegisterPrimitive("cdr",        &Runtime::CDR);
    RegisterPrimitive("cond",       &Runtime::COND,    SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("cons",       &Runtime::CONS,    SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("define",     &Runtime::DEFINE,  SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("eval",       &Runtime::EVAL);
    RegisterPrimitive("lambda",     &Runtime::LAMBDA,  SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("let",        &Runtime::LET,     SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("letrec",     &Runtime::LETREC,  SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("let*",       &Runtime::LETSTAR, SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("list",       &Runtime::LIST,    SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("set!",       &Runtime::SETQ,    SYMBOLFLAG_DONT_EVAL_ARGS);

    RegisterPrimitive("define-syntax",     &Runtime::DEFINE,  SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("let-syntax",        &Runtime::LET,     SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("letrec-syntax",     &Runtime::LETREC,  SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("syntax-rules",       &Runtime::SETQ,    SYMBOLFLAG_DONT_EVAL_ARGS);

    RegisterPrimitive("eq?",        &Runtime::EQP);
    RegisterPrimitive("eqv?",       &Runtime::EQVP);
    RegisterPrimitive("equal?",     &Runtime::EQUALP);

    RegisterPrimitive("pair?",      &Runtime::PAIRP);
    RegisterPrimitive("symbol?",    &Runtime::SYMBOLP);
    RegisterPrimitive("procedure?", &Runtime::PROCEDUREP);
    RegisterPrimitive("boolean?",    &Runtime::NUMBERP);
    RegisterPrimitive("char?",    &Runtime::NUMBERP);
    RegisterPrimitive("string?",    &Runtime::NUMBERP);
    RegisterPrimitive("port?",    &Runtime::NUMBERP);
    RegisterPrimitive("procedure?",    &Runtime::NUMBERP);

    RegisterPrimitive("number?",    &Runtime::NUMBERP);
    RegisterPrimitive("complex?",    &Runtime::NUMBERP);
    RegisterPrimitive("real?",    &Runtime::NUMBERP);
    RegisterPrimitive("rational?",    &Runtime::NUMBERP);
    RegisterPrimitive("integer?",    &Runtime::NUMBERP);



    RegisterPrimitive("+",       &Runtime::ADD);
    RegisterPrimitive("-",       &Runtime::SUB);
    RegisterPrimitive("*",       &Runtime::MUL);
    RegisterPrimitive("/",       &Runtime::DIV);
    RegisterPrimitive("%",       &Runtime::MOD);
    RegisterPrimitive("<",       &Runtime::LESS);
    RegisterPrimitive("=",       &Runtime::EQLOP);

    RegisterPrimitive("round",   &Runtime::ROUND);
    RegisterPrimitive("trunc",   &Runtime::TRUNCATE);
    RegisterPrimitive("floor",   &Runtime::FLOOR);
    RegisterPrimitive("ceiling", &Runtime::CEILING);
    RegisterPrimitive("exp",     &Runtime::EXP);
    RegisterPrimitive("log",     &Runtime::LOG);
    RegisterPrimitive("sqrt",    &Runtime::SQRT);
    RegisterPrimitive("abs",     &Runtime::ABS );
    RegisterPrimitive("sin",     &Runtime::SIN);
    RegisterPrimitive("sinh",    &Runtime::SINH);
    RegisterPrimitive("asin",    &Runtime::ASIN);
    RegisterPrimitive("asinh",   &Runtime::ASINH);
    RegisterPrimitive("cos",     &Runtime::COS);
    RegisterPrimitive("cosh",    &Runtime::COSH);
    RegisterPrimitive("acos",    &Runtime::ACOS);
    RegisterPrimitive("acosh",   &Runtime::ACOSH);
    RegisterPrimitive("tan",     &Runtime::TAN);
    RegisterPrimitive("tanh",    &Runtime::TANH);
    RegisterPrimitive("atan",    &Runtime::ATAN);
    RegisterPrimitive("atanh",   &Runtime::ATANH);

    // Interpreter/meta commands

    RegisterPrimitive("help",    &Runtime::Help);
    RegisterPrimitive("exit",    &Runtime::Exit);
    RegisterPrimitive("quit",    &Runtime::Exit);
    RegisterPrimitive("run-gc",  &Runtime::RunGC);
}

Runtime::~Runtime()
{
}

CELLID Runtime::RegisterReserved(const char* ident)
{
    SYMBOLIDX symbolIndex = GetSymbolIndex(ident);
    CELLID cellIndex = _symbol[symbolIndex]._symbolCell;

    _symbol[symbolIndex]._type = SYMBOL_RESERVED;
    return (cellIndex);
}

SYMBOLIDX Runtime::StoreSymbol(const char* ident, CELLID cellIndex, SymbolType symbolType, STRINGHASH hash)
{
    if (!hash)
        hash = HashString(ident);

    SYMBOLIDX symbolIndex = (SYMBOLIDX)_symbol.Alloc();
    _globalScope[hash] = symbolIndex;

    _cell[cellIndex]._data = symbolIndex;

    SymbolInfo& symbol = _symbol[symbolIndex];
    symbol._type = symbolType;
    symbol._ident = ident;
    symbol._symbolCell = cellIndex;
    symbol._valueCell.SetInvalid();

    return (symbolIndex);
}

SYMBOLIDX Runtime::GetSymbolIndex(const char* ident)
{
    STRINGHASH hash = HashString(ident);

    auto iter = _globalScope.find(hash);
    if (iter != _globalScope.end())
        return (iter->second);

    CELLID cellIndex = AllocateCell(TYPE_SYMBOL);
    SYMBOLIDX symbolIndex = StoreSymbol(ident, cellIndex, SYMBOL_INVALID, hash);

    return (symbolIndex);
}

CELLID Runtime::RegisterPrimitive(const char* ident, PrimitiveFunc func, SymbolFlags flags)
{
    SYMBOLIDX symbolIndex = GetSymbolIndex(ident);
    SymbolInfo& symbol = _symbol[symbolIndex];

    symbol._type = SYMBOL_PRIMITIVE;
    symbol._flags = flags;
    symbol._primIndex = (PRIMIDX) _primitive.size();

    _primitive.emplace_back();
    PrimitiveInfo& primInfo = _primitive.back();

    primInfo._name = ident;
    primInfo._func = func;

    return (symbol._symbolCell);
}

bool Runtime::IsTruthy(CELLID index)
{
    if (index == _false)
        return _false;

    return _true;
}

CELLID Runtime::EncodeSyntaxTree(const NodeRef& node)
{
    DebugValidateCells();

    CELLID result = EncodeTreeNode(node);
    //DumpCellGraph(result, true);

    DebugValidateCells();
    return (result);
}

CELLID Runtime::EncodeTreeNode(const NodeRef& node)
{
    switch (node->_type)
    {
        case AST_NODE_INT_LITERAL:
        {
            CELLID intCell = AllocateCell(TYPE_INT);

            StoreIntLiteral(intCell, node->_int);
            return (intCell);
        }
        case AST_NODE_FLOAT_LITERAL:
        {
            CELLID floatCell = AllocateCell(TYPE_FLOAT);

            StoreFloatLiteral(floatCell, node->_float);
            return (floatCell);
        }
        case AST_NODE_STRING_LITERAL:
        {
            CELLID stringCell = AllocateCell(TYPE_STRING);

            StoreStringLiteral(stringCell, node->_string.c_str());
            return (stringCell);
        }
        case AST_NODE_IDENTIFIER:
        {
            SYMBOLIDX symbolIndex = GetSymbolIndex(node->_identifier.c_str());

            SymbolInfo& symbol = _symbol[symbolIndex];
            return (symbol._symbolCell);
        }
        case AST_NODE_LIST:
        {
            if (node->_list.empty())
                return (_null);

            CELLID listHeadCell;
            CELLID listPrevCell;

            for (auto& elemNode : node->_list)
            {
                CELLID listCell = AllocateCell(TYPE_CONS);
                CELLID nodeCell = EncodeTreeNode(elemNode);
                _cell[listCell]._data = nodeCell;

                if (listPrevCell.IsValid())
                    _cell[listPrevCell]._next = listCell;
                else
                    listHeadCell = listCell;

                listPrevCell = listCell;
                
            }

            return (listHeadCell);
        }
        default:
            break;
    }

    RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED, "unknown AST node type");
    return 0;
}

string Runtime::GetPrintedValue(CELLID index)
{
    assert(index.IsValid());
    if (index == _null)
        return ("nil");

    std::stringstream ss;

    switch (_cell[index]._type)
    {
        case TYPE_CONS:
        {
            if (_cell[index]._data == _quote)
            {
                ss << "'";
                ss << GetPrintedValue(_cell[_cell[index]._next]._data);
                
            }
            else if (_cell[index]._data == _unquote)
            {
                ss << ",";
                ss << GetPrintedValue(_cell[_cell[index]._next]._data);
                
            }
            else if (_cell[index]._data == _quasiquote)
            {
                ss << "`";
                ss << GetPrintedValue(_cell[_cell[index]._next]._data);
                
            }
            else
            {
                ss << '(';

                CELLID curr = index;
                while (curr != _null)
                {
                    ss << GetPrintedValue(_cell[curr]._data);

                    CELLID next = _cell[curr]._next;
                    assert(next.IsValid());

                    if ((next != _null) && (_cell[next]._type != TYPE_CONS))
                    {
                        ss << " . " << GetPrintedValue(next);
                        
                        break;
                    }

                    if (next != _null)
                        ss << " ";

                    curr = next;
                    
                }

                ss << ')';
                
            }
            break;
        }

        case TYPE_INT:    ss <<  LoadIntLiteral(index);                     break;
        case TYPE_FLOAT:  ss <<  LoadFloatLiteral(index);                   break;
        case TYPE_STRING: ss <<  '\"' << LoadStringLiteral(index) << '\"';  break;
        case TYPE_SYMBOL: ss <<  _symbol[_cell[index]._data]._ident;        break;
        case TYPE_LAMBDA: ss <<  "<lambda " << index << ">";                break;
        default:          
            RAISE_ERROR(ERROR_INTERNAL_CELL_TABLE_CORRUPT, "unrecognized cell type"); break;
    }

    return (ss.str());
}


