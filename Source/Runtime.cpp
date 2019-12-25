// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime()
{
    _primitive.emplace_back(); // element 0 is invalid

    _cellFreeList = 0;
    ExpandCellTable();

    _nil   = RegisterSymbol("nil");
    _true  = RegisterSymbol("t");
    _quote = RegisterPrimitive("quote", NULL);

    // Language primitives

    RegisterPrimitive("atom",    &Runtime::ATOM);
    RegisterPrimitive("car",     &Runtime::CAR);
    RegisterPrimitive("cdr",     &Runtime::CDR);
    RegisterPrimitive("cond",    &Runtime::COND);
    RegisterPrimitive("cons",    &Runtime::CONS);
    RegisterPrimitive("eq",      &Runtime::EQ);
    RegisterPrimitive("eval",    &Runtime::EVAL);
    RegisterPrimitive("list",    &Runtime::LIST);
    RegisterPrimitive("setq",    &Runtime::SETQ);

    RegisterPrimitive("+",       &Runtime::ADD);
    RegisterPrimitive("-",       &Runtime::SUB);
    RegisterPrimitive("*",       &Runtime::MUL);
    RegisterPrimitive("/",       &Runtime::DIV);
    RegisterPrimitive("%",       &Runtime::MOD);
    RegisterPrimitive("<",       &Runtime::LESS);

    _defmacro = RegisterPrimitive("defmacro",&Runtime::DEFMACRO);


    // Interpreter commands

    RegisterPrimitive("help",  &Runtime::Help);
    RegisterPrimitive("exit",  &Runtime::Exit);
    RegisterPrimitive("quit",  &Runtime::Exit);
}

Runtime::~Runtime()
{
}

CELL_INDEX Runtime::RegisterSymbol(const char* ident)
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
        symbol._ident = ident;
        symbol._symbolCell = cellIndex;
        symbol._valueCell = _nil;
    }

    return symbolIndex;
}

CELL_INDEX Runtime::RegisterPrimitive(const char* ident, PrimitiveFunc func)
{
    SYMBOL_INDEX symbolIndex = GetSymbolIndex(ident);
    SymbolInfo& symbol = _symbol[symbolIndex];

    symbol._type = SYMBOL_PRIMITIVE;
    symbol._primIndex = (TINDEX)_primitive.size();

    _primitive.emplace_back();
    PrimitiveInfo& primInfo = _primitive.back();

    primInfo._name = ident;
    primInfo._func = func;

    return symbol._symbolCell;
}


CELL_INDEX Runtime::EncodeSyntaxTree(const NodeRef& node)
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
                CELL_INDEX listCell = AllocateCell(TYPE_LIST);
                _cell[listCell]._data = EncodeSyntaxTree(elemNode);

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

    Cell& cell = _cell[index];
    std::stringstream ss;

    switch (cell._type)
    {
        case TYPE_LIST:
        {
            ss << '(';
            CELL_INDEX curr = index;
            while (curr)
            {
                ss << GetPrintedValue(_cell[curr]._data);
                if (_cell[curr]._next)
                    ss << " ";
                curr = _cell[curr]._next;
            }
            ss << ')';
            break;
        }

        case TYPE_INT:    ss << LoadIntLiteral(index); break;
        case TYPE_FLOAT:  ss << LoadFloatLiteral(index); break;
        case TYPE_STRING: ss << '\"' << LoadStringLiteral(index) << '\"'; break;
        case TYPE_SYMBOL: ss << _symbol[cell._data]._ident; break;

        default:          RAISE_ERROR(ERROR_INTERNAL_CELL_TABLE_CORRUPT); break;
    }

    return ss.str();
}



