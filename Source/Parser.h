// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "SyntaxTree.h"

struct ParsingError
{
	int _line;
	int _column;
	string _message;
};

class Parser
{
	const char* _code;

	inline void SkipWhitespace()
	{
		while (isspace(*_code))
			_code++;
	}

	inline bool Peek(char c)
	{
		for (;; )
		{
			SkipWhitespace();

			if (*_code != ';')
				break;

			while (*_code && *_code != '\n')
				_code++;
		}

		return (*_code == c);
	}

	inline bool Consume(char c)
	{
		if (!Peek(c))
			return false;

		_code++;
		return true;
	}

	NodeRef ParseElement();
	NodeRef ParseList();
	NodeRef ParseAtom();
	NodeRef ParseString();
	NodeRef ParseNumber();
	NodeRef ParseIdentifier();

public:
	list<NodeRef> ParseExpressions(const string& code, ParsingError* outError = NULL);

	static void TestParsing(const string& code);
	static void RunUnitTest();
};

