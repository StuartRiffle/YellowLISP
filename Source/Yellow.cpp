// YellowLISP (c) 2019 Stuart Riffle

#include "Yellow.h"
#include "Bootstrap.h"
#include "Parser.h"

int main(int argc, char** argv)
{
	Parser::RunUnitTest();

	Parser parser;
	ParsingError error;

	list<NodeRef> expressions = parser.ParseExpressions(gBootstrapCode, &error);
	for (auto expr : expressions)
	{
		cout << expr->Serialize() << std::endl;
	}


	return 0;
}
