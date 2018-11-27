#pragma clang diagnostic ignored "-Wmicrosoft-include"
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

	Parser parse(move(tokens));
	unique_ptr<AST::Base> head = parse.parsePrimary();

	return 0;
}
