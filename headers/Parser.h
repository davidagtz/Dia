#pragma once

#include <vector>
#include <memory>
#include <map>
#include <ctype.h>
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
#include "TokenTools.h"
#include "Expr.h"

#define ull unsigned long long

class Parser
{
	std::vector<token> tokens;
	ull pos;
	ull line;
	std::map<std::string, int> binop;

  public:
	Parser(std::vector<token> toks) : tokens(toks), binop()
	{
		binop["("] = 40;
		binop["*"] = 30;
		binop["/"] = 30;
		binop["+"] = 20;
		binop["-"] = 20;
	};

	std::unique_ptr<dia::Base> parseNumber()
	{
		advance();
		return std::move(std::make_unique<dia::Number>(std::stoi(tokens.at(pos - 1).val())));
	};

	std::unique_ptr<dia::Base> parseParenExpr()
	{
		advance();
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		if (!tok().valis(")"))
			return LogError<dia::Base>("expected ')'");
		advance();
		return expr;
	};

	std::unique_ptr<dia::Base> parseIden()
	{
		std::string idname = tok().val();
		advance();
		if (!tok().valis("("))
			return std::make_unique<dia::Variable>(idname);
		advance();
		std::vector<std::unique_ptr<dia::Base>> args;
		if (!tok().valis(")"))
			while (1)
			{
				auto Arg = parseExpr();
				if (Arg)
					args.push_back(std::move(Arg));
				else
					return nullptr;
				if (tok().valis(")"))
					break;
				if (!tok().valis(","))
					return LogError<dia::Base>("Expected ',' or ')'");
				advance();
			}
		return std::make_unique<dia::Call>(idname, std::move(args));
	};

	std::unique_ptr<dia::Base> parsePrimary()
	{
		switch (tok().getid())
		{
		case iden:
			return parseIden();
		case paren:
			return parseParenExpr();
		case num:
			return parseNumber();
		default:
			return LogError<dia::Base>("Unexpected Token");
		}
	};

	std::unique_ptr<dia::Base> parseExpr()
	{
		auto left = parsePrimary();
		if (!left)
			return nullptr;
		return parseRightBinop(0, std::move(left));
	};

	std::unique_ptr<dia::Base> parseRightBinop(int oprec, std::unique_ptr<dia::Base> left)
	{
		while (1)
		{
			int prec = getPrec();
			if (getPrec() < oprec)
			{
				return left;
			}
			std::string comp = tok().val();
			advance();
			auto right = parsePrimary();
			if (!right)
				return nullptr;
			int nprec = getPrec();
			if (prec < nprec)
			{
				right = parseRightBinop(prec + 1, std::move(right));
				if (!right)
				{
					return nullptr;
				}
			}
			left = std::make_unique<dia::Binary>(comp, std::move(left), std::move(right));
		};
	};

	std::unique_ptr<dia::Prototype> parsePrototype()
	{
		if (!tok().idis(iden))
			return LogError<dia::Prototype>("Expected function in prototype");
		std::string fname = tok().val();
		advance();
		if (!tok().valis("("))
			return LogError<dia::Prototype>("Expected '(' in header");
		std::vector<std::string> args;
		while (tok().idis(iden))
		{
			args.push_back(tok().val());
			advance();
		}
		if (!tok().valis(")"))
			return LogError<dia::Prototype>("Expected ')' in header");
		advance();
		return std::make_unique<dia::Prototype>(fname, std::move(args));
	};

	std::unique_ptr<dia::Function> parseDef()
	{
		if (!tok().valis("func"))
			return nullptr;
		auto proto = parsePrototype();
		if (!proto)
			return nullptr;
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		return std::make_unique<dia::Function>(std::move(proto), std::move(expr));
	};

	std::unique_ptr<dia::Function> parseTopLevel()
	{
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		auto proto = std::make_unique<dia::Prototype>("", std::vector<std::string>());
		return std::make_unique<dia::Function>(std::move(proto), std::move(expr));
	}

	int getPrec()
	{
		if (!isalpha(tok().val().at(0)))
			return -1;
		int prec = binop[tok().val()];
		if (prec <= 0)
			return -1;
		return prec;
	};

	token tok() { return tokens.at(pos); };
	void advance() { advance(1); };
	void advance(int i)
	{
		do
		{
			pos += i;
			if (tok().idis(eol))
			{
				line++;
			}
		} while (tok().idis(eol));
	};

	template <class T>
	std::unique_ptr<T> LogError(std::string msg)
	{
		std::cerr << "Line " << line << ": " << msg << std::endl;
		return nullptr;
	};
};
