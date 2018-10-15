#include "headers/TokenTools.h"
#include "headers/Lexer.h"
#include "headers/Parser.h"

using namespace std;

int main(int argc, char **argv)
{
	vector<token> tokens = lex(argc, argv);

	for (int i = 0; i < tokens.size(); i++)
	{
		cout << tokens.at(i).toString() << endl;
	}

	AST::ExprAST head = Parser(tokens).parsePrimary();

	return 0;
}
