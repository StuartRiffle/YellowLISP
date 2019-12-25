// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

// This is for debugging. It generates a .dot file that can be rendered by GraphViz,
// to visualize cell connections. 

void Runtime::FormatCellLabel(CELL_INDEX cellIndex, std::stringstream& ss, set<CELL_INDEX>& cellsDone, set<SYMBOL_INDEX>& symbolsDone)
{
    if (cellsDone.count(cellIndex))
        return;

    Cell& cell = _cell[cellIndex];
    std::stringstream ssValue;

    ss << "cell" << cellIndex << " [shape=record,label=\"<header>CELL " << cellIndex << " (";

    switch (cell._type)
    {
        case TYPE_LIST:   ss << "LIST";   ssValue << "cell " << cell._data; break;
        case TYPE_INT:    ss << "INT";    ssValue << LoadIntLiteral(cellIndex); break;
        case TYPE_FLOAT:  ss << "FLOAT";  ssValue << LoadFloatLiteral(cellIndex); break;
        case TYPE_STRING: ss << "STRING"; ssValue << LoadStringLiteral(cellIndex); break;
        case TYPE_SYMBOL: ss << "SYMBOL"; ssValue << "symbol " << cell._data; break;
    }

    ss << ") | { { next | data } | { <next>";

    if (cell._next && (cell._next != _nil))
        ss << "cell " << cell._next;

    ss << "| <data>" << ssValue.str() << " } }\"];" << std::endl;

    cellsDone.insert(cellIndex);

    if (cell._type == TYPE_SYMBOL)
    {
        SYMBOL_INDEX symbolIndex = cell._data;
        ss << "cell" << cellIndex << ":data -> symbol" << symbolIndex << ":header;" << std::endl;

        FormatSymbolLabel(symbolIndex, ss, cellsDone, symbolsDone);
    }

    if (cell._type == TYPE_LIST)
    {
        ss << "cell" << cellIndex << ":data -> cell" << cell._data << ":header;" << std::endl;
        FormatCellLabel(cell._data, ss, cellsDone, symbolsDone);
    }

    if (cell._next)
    {
        ss << "cell" << cellIndex << ":next -> cell" << cell._next << ":header;" << std::endl;
        FormatCellLabel(cell._next, ss, cellsDone, symbolsDone);
    }
}

void Runtime::FormatSymbolLabel(SYMBOL_INDEX symbolIndex, std::stringstream& ss, set<CELL_INDEX>& cellsDone, set<SYMBOL_INDEX>& symbolsDone)
{
    if (symbolsDone.count(symbolIndex))
        return;

    SymbolInfo& symbol = _symbol[symbolIndex];

    std::stringstream ssValue;

    ss << "symbol" << symbolIndex << " [shape=record,label=\"<header>SYMBOL " << symbolIndex << " | ";
    ss << "{ { ident | primIndex | symbolCell | valueCell } | ";
    ss << "{ <ident>" << symbol._ident << " | <primIndex>" << symbol._primIndex << " | <symbolCell>" << symbol._symbolCell << " | ";

    if (symbol._valueCell && (symbol._valueCell != _nil))
        ss << "<valueCell>cell " << symbol._valueCell;

    ss << " } }\"];" << std::endl;

    symbolsDone.insert(symbolIndex);

    if (symbol._valueCell && (symbol._valueCell != _nil))
    {
        ss << "symbol" << symbolIndex << ":valueCell -> cell" << symbol._valueCell << ":header;" << std::endl;
        FormatCellLabel(symbol._valueCell, ss, cellsDone, symbolsDone);
    }
}

void Runtime::DumpCellGraph(CELL_INDEX cellIndex, const string& filename)
{
    std::stringstream ss;
    set<CELL_INDEX> cellsDone;
    set<SYMBOL_INDEX> symbolsDone;

    ss << "digraph G {" << std::endl;
    ss << "graph[rankdir = \"LR\"];" << std::endl;

    FormatCellLabel(cellIndex, ss, cellsDone, symbolsDone);

    ss << "} " << std::endl;

    std::ofstream outFile(filename);
    if (outFile)
    {
        outFile << ss.str();
        outFile.close();
    }
}

