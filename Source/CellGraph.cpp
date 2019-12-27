// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

// This is for debugging. It generates a .dot file that can be rendered by GraphViz,
// to visualize cell connections. 

void Runtime::FormatCellLabel(CELL_INDEX cellIndex, std::stringstream& ss, set<CELL_INDEX>& cellsDone, set<SYMBOL_INDEX>& symbolsDone, bool expandSymbols)
{
    if (cellsDone.count(cellIndex))
        return;

    Cell& cell = _cell[cellIndex];

    std::stringstream ssValue;

    string fillColor = "white";
    string cellTypeName;

    switch (cell._type)
    {
        case TYPE_LIST:   cellTypeName = "LIST";   ssValue << "cell " << cell._data; fillColor = "lemonchiffon"; break;
        case TYPE_INT:    cellTypeName = "INT";    ssValue << LoadIntLiteral(cellIndex); break;
        case TYPE_FLOAT:  cellTypeName = "FLOAT";  ssValue << LoadFloatLiteral(cellIndex); break;
        case TYPE_STRING: cellTypeName = "STRING"; ssValue << LoadStringLiteral(cellIndex);  break;
        case TYPE_SYMBOL: cellTypeName = "SYMBOL"; ssValue << "symbol " << cell._data; fillColor = "lightcyan";  break;
    }

    ss << "cell" << cellIndex << " [shape=record,style=filled,fillcolor=" << fillColor << ",label=\"<header>CELL " << cellIndex << " (";
    ss << cellTypeName;

    ss << ") | { { ";

    bool includeIdent = !expandSymbols && (cell._type == TYPE_SYMBOL);

    if (includeIdent)
        ss << "ident | ";

    ss << "next | data } | { ";
        
    if (includeIdent)
        ss << _symbol[cell._data]._ident << " | ";
        
    ss << "<next>";

    if (VALID_CELL(cell._next))
        ss << "cell " << cell._next;

    ss << "| <data>" << ssValue.str() << " } }\"];" << std::endl;

    cellsDone.insert(cellIndex);

    if (expandSymbols && (cell._type == TYPE_SYMBOL))
    {
        SYMBOL_INDEX symbolIndex = cell._data;
        ss << "cell" << cellIndex << ":data -> symbol" << symbolIndex << ":header;" << std::endl;

        FormatSymbolLabel(symbolIndex, ss, cellsDone, symbolsDone);
    }

    if (cell._type == TYPE_LIST)
    {
        ss << "cell" << cellIndex << ":data -> cell" << cell._data << ":header;" << std::endl;
        FormatCellLabel(cell._data, ss, cellsDone, symbolsDone, expandSymbols);
    }

    if (VALID_CELL(cell._next))
    {
        ss << "cell" << cellIndex << ":next -> cell" << cell._next << ":header;" << std::endl;
        FormatCellLabel(cell._next, ss, cellsDone, symbolsDone, expandSymbols);
    }
}

void Runtime::FormatSymbolLabel(SYMBOL_INDEX symbolIndex, std::stringstream& ss, set<CELL_INDEX>& cellsDone, set<SYMBOL_INDEX>& symbolsDone)
{
    if (symbolsDone.count(symbolIndex))
        return;

    SymbolInfo& symbol = _symbol[symbolIndex];

    std::stringstream ssValue;

    ss << "symbol" << symbolIndex << " [shape=Mrecord,style=filled,fillcolor=lightblue1,label=\"<header>SYMBOL " << symbolIndex << " | ";
    ss << "{ { ident | primIndex | symbolCell | valueCell | bindingListCell } | ";
    ss << "{ <ident>" << symbol._ident << " | <primIndex>" << symbol._primIndex << " | <symbolCell>" << symbol._symbolCell << " | ";

    if (VALID_CELL(symbol._valueCell))
        ss << "<valueCell>cell " << symbol._valueCell;

    ss << "| <bindingListCell>" << symbol._bindingListCell;

    ss << " } }\"];" << std::endl;

    symbolsDone.insert(symbolIndex);

    if (VALID_CELL(symbol._valueCell))
    {
        ss << "symbol" << symbolIndex << ":valueCell -> cell" << symbol._valueCell << ":header;" << std::endl;
        FormatCellLabel(symbol._valueCell, ss, cellsDone, symbolsDone, true);
    }

    if (VALID_CELL(symbol._bindingListCell))
    {
        ss << "symbol" << symbolIndex << ":bindingListCell -> cell" << symbol._bindingListCell << ":header;" << std::endl;
        FormatCellLabel(symbol._bindingListCell, ss, cellsDone, symbolsDone, true);
    }
}

void Runtime::DumpCellGraph(CELL_INDEX cellIndex, const string& filename, bool expandSymbols)
{
    std::stringstream ss;
    set<CELL_INDEX> cellsDone;
    set<SYMBOL_INDEX> symbolsDone;

    ss << "digraph G {" << std::endl;
    ss << "graph[rankdir = \"LR\"];" << std::endl;

    FormatCellLabel(cellIndex, ss, cellsDone, symbolsDone, expandSymbols);

    ss << "} " << std::endl;

    std::ofstream outFile(filename);
    if (outFile)
    {
        outFile << ss.str();
        outFile.close();
    }
}

