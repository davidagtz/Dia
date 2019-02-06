#include <vector>
#include <memory>
#include <map>
#include <ctype.h>
#include <string>
#include <iostream>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "headers/TokenTools.h"
#include "headers/Expr.h"
#include "headers/Lexer.h"
#include "headers/Parser.h"
#include "headers/codegen.h"

using namespace std;

int main(int argc, char **argv)
{
	TheModule = std::make_unique<llvm::Module>("Dia JIT", TheContext);
	while (1)
	{
		// Input
		cout << "> ";
		string in = "";
		char last = 'a';
		while (int(last) != 10)
		{
			last = getchar();
			if (last != 10)
				in += last;
		}

		vector<token> t = FileTokenizer(in).getTokens();

		Parser p(t);

		for (token to : t)
		{
			cout << to.val() + " " << to.getid() << endl;
		}

		switch (t.at(0).getid())
		{
		case eol:
			return 0;
		case def:
			p.handle_def();
			break;
		default:
			p.handle_top_level();
			break;
		}

		return 0;
	}

	// vector<token> tokens = lex(argc, argv);

	// for (int i = 0; i < tokens.size(); i++)
	// 	if (tokens.at(i).idis(errHandle))
	// 		return 1;

	// Parser parse(move(tokens));
	// unique_ptr<dia::Base> head = parse.parsePrimary();

	// llvm::Value *code = head->codegen();

	return 0;
}
