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
void shell();
llvm::Function *getMainFunction();

using namespace std;

int main(int argc, char **argv)
{
	TheModule = std::make_unique<llvm::Module>("Dia JIT", TheContext);

	if (argc == 1)
	{
		shell();
		TheModule->print(llvm::errs(), nullptr);
		return 0;
	}

	vector<token> tokens = lex(argc, argv);

	for (int i = 0; i < tokens.size(); i++)
		if (tokens.at(i).idis(errHandle))
			return 1;

	Parser parse(std::move(tokens));

	llvm::Function *fmain = getMainFunction();
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", fmain);

	while (parse.pos < parse.size() - 1)
	{
		switch (parse.at(parse.pos).getid())
		{
		case def:
			parse.handle_def();
			break;
		case ext:
			parse.handle_extern();
			break;
		case eol:
			parse.advance();
			break;
		default:
			parse.handle_top_level(fmain, BB);
			break;
		}
		cout << "\e[92m--------------------------------\e[0m" << std::endl;
	}

	Builder.CreateRet(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0)));
	llvm::verifyFunction(*fmain);

	llvm::StringRef sr(output);
	error_code ec;
	llvm::raw_fd_ostream os(sr, ec);

	TheModule->print(os, nullptr);

	return 0;
}

void shell()
{
	while (1)
	{
		// Input
		int pos_back = -1;
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
		if (t.size() == 0)
			return;

		Parser p(t);

		// for (token to : t)
		// {
		// 	cout << to.val() + " " << to.getid() << endl;
		// }

		switch (t.at(0).getid())
		{
		case def:
			p.handle_def();
			break;
		default:
			p.handle_top_level();
			break;
		}
	}
};

llvm::Function *getMainFunction()
{
	vector<llvm::Type *> argstypes({});
	llvm::FunctionType *t = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), argstypes, false);
	llvm::Function *f = llvm::Function::Create(t, llvm::Function::ExternalLinkage, "main", TheModule.get());
	// int i = 0;
	// for (auto &arg : f->args())
	// 	arg.setName((f->args[i++]).first);

	llvm::Function *f2 = TheModule->getFunction(f->getName());

	if (!f2)
		f2 = f;

	if (!f2)
		return nullptr;

	return f2;
}