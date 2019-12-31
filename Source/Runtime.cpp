// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime()
{
    _primitive.emplace_back(); // element 0 is invalid

    _nil = 0;
    InitCellTable();
    _garbageCollectionNeeded = false;

    _nil = RegisterReserved("nil");
    _dot = RegisterReserved(".");
    _true = RegisterReserved("t");
    _eval = RegisterPrimitive("eval", &Runtime::EVAL);
    _quote = RegisterPrimitive("quote", NULL, SYMBOLFLAG_DONT_EVAL_ARGS);
    _unquote = RegisterPrimitive("unquote", NULL, SYMBOLFLAG_DONT_EVAL_ARGS);
    _quasiquote = RegisterPrimitive("quasiquote", NULL, SYMBOLFLAG_DONT_EVAL_ARGS);

    // Language primitives

    RegisterPrimitive("cond",    &Runtime::COND,     SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("cons",    &Runtime::CONS,     SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("defmacro",&Runtime::DEFMACRO, SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("defun",   &Runtime::DEFUN,    SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("lambda",  &Runtime::LAMBDA,   SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("list",    &Runtime::LIST,     SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("progn",   &Runtime::PROGN,    SYMBOLFLAG_DONT_EVAL_ARGS);
    RegisterPrimitive("setq",    &Runtime::SETQ,     SYMBOLFLAG_DONT_EVAL_ARGS);

    RegisterPrimitive("atom",    &Runtime::ATOM);
    RegisterPrimitive("car",     &Runtime::CAR);
    RegisterPrimitive("cdr",     &Runtime::CDR);
    RegisterPrimitive("eq",      &Runtime::EQ);
    RegisterPrimitive("eql",     &Runtime::EQL);
    RegisterPrimitive("equal",   &Runtime::EQUAL);
    RegisterPrimitive("equalp",  &Runtime::EQUALP);

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
    RegisterPrimitive("expt",    &Runtime::EXPT);
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

CELL_INDEX Runtime::RegisterReserved(const char* ident)
{
    SYMBOL_INDEX symbolIndex = GetSymbolIndex(ident);
    CELL_INDEX cellIndex = _symbol[symbolIndex]._symbolCell;

    _symbol[symbolIndex]._type = SYMBOL_RESERVED;
    return cellIndex;
}

SYMBOL_INDEX Runtime::GetSymbolIndex(const char* ident)
{
    THASH hash = HashString(ident);
    SYMBOL_INDEX symbolIndex = _globalScope[hash];

    if (!symbolIndex)
    {
        symbolIndex = (SYMBOL_INDEX)_symbol.Alloc();
        _globalScope[hash] = symbolIndex;

        CELL_INDEX cellIndex = AllocateCell(TYPE_SYMBOL);
        _cell[cellIndex]._data = symbolIndex;

        SymbolInfo& symbol = _symbol[symbolIndex];
        symbol._type = SYMBOL_INVALID;
        symbol._ident = ident;
        symbol._symbolCell = cellIndex;
        symbol._valueCell = 0;
    }

    return symbolIndex;
}

CELL_INDEX Runtime::RegisterPrimitive(const char* ident, PrimitiveFunc func, SymbolFlags flags)
{
    SYMBOL_INDEX symbolIndex = GetSymbolIndex(ident);
    SymbolInfo& symbol = _symbol[symbolIndex];

    symbol._type = SYMBOL_PRIMITIVE;
    symbol._flags = flags;
    symbol._primIndex = (TINDEX)_primitive.size();

    _primitive.emplace_back();
    PrimitiveInfo& primInfo = _primitive.back();

    primInfo._name = ident;
    primInfo._func = func;

    return symbol._symbolCell;
}

CELL_INDEX Runtime::EncodeSyntaxTree(const NodeRef& node)
{
    DebugValidateCells();

    CELL_INDEX result = EncodeTreeNode(node);
    //DumpCellGraph(result, true);

    DebugValidateCells();
    return result;
}

CELL_INDEX Runtime::EncodeTreeNode(const NodeRef& node)
{
    switch (node->_type)
    {
        case AST_NODE_INT_LITERAL:
        {
            CELL_INDEX intCell = AllocateCell(TYPE_INT);

            StoreIntLiteral(intCell, node->_int);
            return intCell;
        }
        case AST_NODE_FLOAT_LITERAL:
        {
            CELL_INDEX floatCell = AllocateCell(TYPE_FLOAT);

            StoreFloatLiteral(floatCell, node->_float);
            return floatCell;
        }
        case AST_NODE_STRING_LITERAL:
        {
            CELL_INDEX stringCell = AllocateCell(TYPE_STRING);

            StoreStringLiteral(stringCell, node->_string.c_str());
            return stringCell;
        }
        case AST_NODE_IDENTIFIER:
        {
            CELL_INDEX symbolCell = GetSymbolIndex(node->_identifier.c_str());

            SymbolInfo& symbol = _symbol[symbolCell];
            return symbol._symbolCell;
        }
        case AST_NODE_LIST:
        {
            if (node->_list.empty())
                return _nil;

            CELL_INDEX listHeadCell = 0;
            CELL_INDEX listPrevCell = 0;

            for (auto& elemNode : node->_list)
            {
                CELL_INDEX listCell = AllocateCell(TYPE_CONS);

                _cell[listCell]._data = EncodeTreeNode(elemNode);

                if (listPrevCell)
                    _cell[listPrevCell]._next = listCell;
                else
                    listHeadCell = listCell;

                listPrevCell = listCell;
            }

            return listHeadCell;
        }
    }

    RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED);
    return 0;
}

string Runtime::GetPrintedValue(CELL_INDEX index)
{
    if (index == 0)
        return "";

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

                CELL_INDEX curr = index;
                while (VALID_CELL(curr))
                {
                    ss << GetPrintedValue(_cell[curr]._data);
                    CELL_INDEX next = _cell[curr]._next;

                    if (VALID_CELL(next) && (_cell[next]._type != TYPE_CONS))
                    {
                        ss << " . " << GetPrintedValue(next);
                        break;
                    }

                    if (VALID_CELL(next))
                        ss << " ";

                    curr = next;
                }

                ss << ')';
            }
            break;
        }

        case TYPE_INT:    ss << LoadIntLiteral(index); break;
        case TYPE_FLOAT:  ss << LoadFloatLiteral(index); break;
        case TYPE_STRING: ss << '\"' << LoadStringLiteral(index) << '\"'; break;
        case TYPE_SYMBOL: ss << _symbol[_cell[index]._data]._ident; break;
        case TYPE_LAMBDA: ss << "<lambda " << index << ">"; break;

        default:          RAISE_ERROR(ERROR_INTERNAL_CELL_TABLE_CORRUPT); break;
    }

    return ss.str();
}



