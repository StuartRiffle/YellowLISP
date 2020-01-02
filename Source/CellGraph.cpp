// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

// This is for debugging. It generates a .dot file that can be rendered by GraphViz,
// to visualize cell connections. 

void Runtime::FormatCellLabel(CELLID cellIndex, std::stringstream& ss, set<CELLID>& cellsDone, set<SYMBOLIDX>& symbolsDone, bool expandSymbols)
{
    assert(cellIndex.IsValid());
    if (cellIndex == _nil)
        return;

    if (cellsDone.count(cellIndex))
        return;

    std::stringstream ssValue;

    string fillColor = "white";
    string cellTypeName;

    switch (_cell[cellIndex]._type)
    {
        case TYPE_CONS:   cellTypeName = "CONS";   ssValue << "cell " << _cell[cellIndex]._data; fillColor = "lemonchiffon"; break;
        case TYPE_INT:    cellTypeName = "INT";    ssValue << LoadIntLiteral(cellIndex); break;
        case TYPE_FLOAT:  cellTypeName = "FLOAT";  ssValue << LoadFloatLiteral(cellIndex); break;
        case TYPE_STRING: cellTypeName = "STRING"; ssValue << LoadStringLiteral(cellIndex);  break;
        case TYPE_SYMBOL: cellTypeName = "SYMBOL"; ssValue << "symbol " << _cell[cellIndex]._data; fillColor = "lightcyan";  break;
        case TYPE_LAMBDA: cellTypeName = "LAMBDA"; ssValue << "cell " << _cell[cellIndex]._data; fillColor = "lightpink";  break;
        default: break;
    }

    ss << "cell" << cellIndex << " [shape=record,style=filled,fillcolor=" << fillColor << ",label=\"<header>CELL " << cellIndex << " (";
    ss << cellTypeName;

    ss << ") | { { ";

    bool includeIdent = !expandSymbols && (_cell[cellIndex]._type == TYPE_SYMBOL);

    if (includeIdent)
        ss << "ident | ";

    ss << "next | data } | { ";
        
    if (includeIdent)
        ss << _symbol[_cell[cellIndex]._data]._ident << " | ";
        
    ss << "<next>";

    if (_cell[cellIndex]._next != _nil)
        ss << "cell " << _cell[cellIndex]._next;

    ss << "| <data>" << ssValue.str() << " } }\"];" << std::endl;

    cellsDone.insert(cellIndex);

    if (expandSymbols && (_cell[cellIndex]._type == TYPE_SYMBOL))
    {
        SYMBOLIDX symbolIndex = _cell[cellIndex]._data;
        ss << "cell" << cellIndex << ":data -> symbol" << symbolIndex << ":header;" << std::endl;

        FormatSymbolLabel(symbolIndex, ss, cellsDone, symbolsDone);
    }

    if (_cell[cellIndex]._type == TYPE_CONS)
    {
        ss << "cell" << cellIndex << ":data -> cell" << _cell[cellIndex]._data << ":header;" << std::endl;
        FormatCellLabel(_cell[cellIndex]._data, ss, cellsDone, symbolsDone, expandSymbols);
    }

    if (_cell[cellIndex]._next != _nil)
    {
        ss << "cell" << cellIndex << ":next -> cell" << _cell[cellIndex]._next << ":header;" << std::endl;
        FormatCellLabel(_cell[cellIndex]._next, ss, cellsDone, symbolsDone, expandSymbols);
    }
}

void Runtime::FormatSymbolLabel(SYMBOLIDX symbolIndex, std::stringstream& ss, set<CELLID>& cellsDone, set<SYMBOLIDX>& symbolsDone)
{
    if (symbolsDone.count(symbolIndex))
        return;

    SymbolInfo& symbol = _symbol[symbolIndex];

    std::stringstream ssValue;

    ss << "symbol" << symbolIndex << " [shape=Mrecord,style=filled,fillcolor=lightblue1,label=\"<header>SYMBOL " << symbolIndex << " | ";
    ss << "{ { ident | primIndex | symbolCell | valueCell | macroBindings } | ";
    ss << "{ <ident>" << symbol._ident;
    ss << " | <primIndex>"; 
    if (symbol._primIndex.IsValid()) 
        ss << symbol._primIndex;
    ss << " | <symbolCell>";
    if (symbol._symbolCell.IsValid())
        ss << symbol._symbolCell;
    ss << " | ";

    if (symbol._valueCell.IsValid() && (symbol._valueCell != _nil))
        ss << "<valueCell>cell " << symbol._valueCell;

    ss << "| <macroBindings>";
    
    if (symbol._macroBindings.IsValid() && (symbol._macroBindings != _nil))
        ss << symbol._macroBindings;

    ss << " } }\"];" << std::endl;

    symbolsDone.insert(symbolIndex);

    if (symbol._valueCell.IsValid() && (symbol._valueCell != _nil))
    {
        ss << "symbol" << symbolIndex << ":valueCell -> cell" << symbol._valueCell << ":header;" << std::endl;
        FormatCellLabel(symbol._valueCell, ss, cellsDone, symbolsDone, true);
    }

    if (symbol._macroBindings.IsValid() && (symbol._macroBindings != _nil))
    {
        ss << "symbol" << symbolIndex << ":macroBindings -> cell" << symbol._macroBindings << ":header;" << std::endl;
        FormatCellLabel(symbol._macroBindings, ss, cellsDone, symbolsDone, true);
    }
}

string Runtime::GenerateCellGraph(CELLID cellIndex, bool expandSymbols)
{
    std::stringstream ss;
    set<CELLID> cellsDone;
    set<SYMBOLIDX> symbolsDone;

    ss << "digraph G {" << std::endl;
    ss << "graph[rankdir = \"LR\"];" << std::endl;
    ss << "node [fontname = \"segoe ui semibold\"];" << std::endl;

    FormatCellLabel(cellIndex, ss, cellsDone, symbolsDone, expandSymbols);

    ss << "} " << std::endl;
    return ss.str();
}

void Runtime::DumpCellGraph(CELLID cellIndex, bool expandSymbols)
{
    string graph = GenerateCellGraph(cellIndex, expandSymbols);

    char filename[80];
    sprintf(filename, "cell%d.dot", (int) cellIndex);

    std::ofstream outFile(filename);
    if (outFile)
    {
        outFile << graph;
        outFile.close();
    }
}


