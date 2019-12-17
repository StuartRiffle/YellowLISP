#include "Parser.h"

const char QUOTE_CHAR = '\'';
const char STRING_DELIM = '\"';
const char OPEN_PAREN = '(';
const char CLOSE_PAREN = ')';
const char MINUS_SIGN = '-';
const char* SYMBOL_CHARS = "!$%&*+-./:<=>?@^_~";

list<NodeRef> Parser::ParseExpressions(const string& code, ParsingError* outError)
{
	list<NodeRef> result;
	_code = code.c_str();

	try
	{
		list<NodeRef> expressions;
		while (*_code)
		{
			expressions.push_back(ParseElement());
			SkipWhitespace();
		}

		result = expressions;
	}
	catch (const char* errorMessage)
	{
		if (outError)
		{
			int line = 1;
			int column = 1;
			const char* errorLine = _code;

			for (const char* cursor = code.c_str(); cursor < _code; cursor++)
			{
				if (*cursor == '\n')
				{
					line++;
					column = 1;
					errorLine = _code;
				}
				else
					column++;
			}

			stringstream ss;

			while (*errorLine && *errorLine != '\n')
				ss << *errorLine++;

			ss << endl;

			for (int i = 1; i < column - 1; i++)
				ss << ' ';

			ss << '^' << endl;
			ss << "ERROR: " << errorMessage;

			outError->_message = ss.str();
			outError->_line = line;
			outError->_column = column;

			cout << outError->_message;
		}
	}

	return result;
}

NodeRef Parser::ParseElement()
{
	if (Consume(QUOTE_CHAR))
		return NodeRef(new QuoteNode(ParseElement()));

	if (Peek(OPEN_PAREN))
		return ParseList();

	return ParseAtom();
}

NodeRef Parser::ParseList()
{
	if (!Consume(OPEN_PAREN))
		throw "List expected";

	vector<NodeRef> elements;
	while (!Peek(CLOSE_PAREN))
		elements.push_back(ParseElement());

	if (!Consume(CLOSE_PAREN))
		throw "List is unterminated";

	return NodeRef(new ListNode(elements));
}

NodeRef Parser::ParseAtom()
{
	if (Peek(STRING_DELIM))
		return ParseString();

	if (isdigit(*_code))
		return ParseNumber();

	return ParseIdentifier();
}

NodeRef Parser::ParseString()
{
	if (!Consume(STRING_DELIM))
		throw "String expected";

	// TODO: handle escape chars

	const char* end = strchr(_code, STRING_DELIM);
	if (!end)
		throw "String is unterminated";

	string str(_code, end - _code);
	_code = end + 1;

	return NodeRef(new StringNode(str));
}

NodeRef Parser::ParseNumber()
{
	assert(!isspace(*_code));

	char* end = NULL;
	double val = strtod(_code, &end);

	if (end == _code)
		throw "Number expected";

	_code = end;

	int64_t integer = (int64_t)val;
	if (integer == val)
		return NodeRef(new IntNode(integer));

	return NodeRef(new FloatNode(val));
}

NodeRef Parser::ParseIdentifier()
{
	assert(!isspace(*_code));

	const char* end = _code;
	while (*end && (isalnum(*end) || strchr(SYMBOL_CHARS, *end)))
		end++;

	if (end == _code)
		throw "Invalid identifier";

	string ident(_code, end - _code);
	_code = end;

	return NodeRef(new IdentNode(ident));
}

// static
void Parser::TestParsing(const string& code)
{
	Parser parser;
	ParsingError error;

	list<NodeRef> expressions = parser.ParseExpressions(code, &error);
	assert(expressions.size() == 1);
	assert(expressions.front() != NULL);

	string rebuilt = expressions.front()->Serialize();
	assert(rebuilt == code);
}

// static
void Parser::RunUnitTest()
{
	TestParsing("()");
	TestParsing("'()");

	TestParsing("(())");
	TestParsing("('())");
	TestParsing("'(())");
	TestParsing("'((a))");

	TestParsing("(a)");
	TestParsing("'(a)");
	TestParsing("('a)");
	TestParsing("'('a)");

	TestParsing("(ab)");
	TestParsing("(ab (c de))");
	TestParsing("(abc123)");

	TestParsing("(a b)");
	TestParsing("(a 'b)");
	TestParsing("('a b)");
	TestParsing("'(a b)");

	TestParsing("(a (b))");
	TestParsing("(a () b)");
	TestParsing("((a) b)");
	TestParsing("((a) (b))");
	TestParsing("(a '(b))");
	TestParsing("(a ('b))");

	TestParsing("(a b c)");
	TestParsing("((a) b c)");
	TestParsing("(a (b) c)");
	TestParsing("(a b (c))");
	TestParsing("((a b) c)");
	TestParsing("(a (b c))");
	TestParsing("((a b c))");

	TestParsing("(123)");
	TestParsing("('123)");
	TestParsing("'(123)");

	TestParsing("(123.45)");
	TestParsing("('123.45)");
	TestParsing("'(123.45)");

	TestParsing("(123 45)");
	TestParsing("(123 45.67)");
	TestParsing("(123 four (5.67 eight))");
}