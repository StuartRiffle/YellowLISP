// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

struct Node
{
    virtual void Print(stringstream& ss) const {}

    string Serialize() const
    {
        stringstream ss;
        Print(ss);

        return ss.str();
    }
};

typedef shared_ptr< Node > NodeRef;

struct IdentNode : public Node
{
    string _ident;
    IdentNode(const string& ident) : _ident(ident) {}

    virtual void Print(stringstream& ss) const
    { 
        ss << _ident; 
    }
};

struct ListNode : public Node
{
    vector<NodeRef> _elements;
    ListNode(const vector<NodeRef>& elements) : _elements(elements) {}

    virtual void Print(stringstream& ss) const
    {
        ss << '(';

        for (size_t i = 0; i < _elements.size(); i++)
        {
            if (i > 0)
                ss << " ";

            _elements[i]->Print(ss);
        }

        ss << ')';
    }
};

struct QuoteNode : public Node
{
    NodeRef _quoted;
    QuoteNode(const NodeRef& quoted) : _quoted(quoted) {}

    virtual void Print(stringstream& ss) const
    {
        ss << '\'';
        _quoted->Print(ss);
    }
};

struct StringNode : public Node
{
    string _str;
    StringNode(const string& str) : _str(str) {}

    virtual void Print(stringstream& ss) const
    {
        ss << '\"' << _str << '\"';
    }
};

struct IntNode : public Node
{
    int64_t _value;
    IntNode(int64_t value) : _value(value) {}

    virtual void Print(stringstream& ss) const
    {
        ss << _value;
    }
};

struct FloatNode : public Node
{
    double _value;
    FloatNode(double value) : _value(value) {}

    virtual void Print(stringstream& ss) const
    {
        ss << _value;
    }
};

