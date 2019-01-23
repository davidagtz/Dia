#include "headers/Lexer.h"
#include "headers/Parser.h"
#include "headers/codegen.h"

using namespace std;

int main(int argc, char **argv)
{
	vector<token> tokens = lex(argc, argv);

	for (int i = 0; i < tokens.size(); i++)
	{

		if (tokens.at(i).idis(errHandle))
		{
			return 1;
		}
	}

	Parser parse(move(tokens));
	unique_ptr<dia::Base> head = parse.parsePrimary();

	llvm::Value *code = head->codegen();

	return 0;
}
