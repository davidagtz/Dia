#pragma once

#include "Expr.h"
#include "TokenTools.h"
#include "ArrayList.h"
#include "llvm/ADT/STLExtras.h"
#include <vector>
#include <memory>
#include <map>

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

	std::unique_ptr<AST::Base> parseNumber()
	{
		advance();
		return std::move(llvm::make_unique<AST::Number>(std::stoi(tokens.at(pos - 1).val())));
	};

	std::unique_ptr<AST::Base> parseParenExpr()
	{
		advance();
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		if (!tok().valis(")"))
			return LogError<AST::Base>("expected ')'");
		advance();
		return expr;
	};

	std::unique_ptr<AST::Base> parseIden()
	{
		std::string idname = tok().val();
		advance();
		if (!tok().valis("("))
			return llvm::make_unique<AST::Variable>(idname);
		advance();
		std::vector<std::unique_ptr<AST::Base>> args;
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
					return LogError<AST::Base>("Expected ',' or ')'");
				advance();
			}
		return llvm::make_unique<AST::Call>(idname, std::move(args));
	};

	std::unique_ptr<AST::Base> parsePrimary()
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
			return LogError<AST::Base>("Unexpected Token");
		}
	};

	std::unique_ptr<AST::Base> parseExpr()
	{
		auto left = parsePrimary();
		if (!left)
			return nullptr;
		return parseRightBinop(0, std::move(left));
	};

	std::unique_ptr<AST::Base> parseRightBinop(int oprec, std::unique_ptr<AST::Base> left)
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
			left = llvm::make_unique<AST::Binary>(comp, std::move(left), std::move(right));
		};
	};

	std::unique_ptr<AST::Prototype> parsePrototype()
	{
		if (!tok().idis(iden))
			return LogError<AST::Prototype>("Expected function in prototype");
		std::string fname = tok().val();
		advance();
		if (!tok().valis("("))
			return LogError<AST::Prototype>("Expected '(' in header");
		std::vector<std::string> args;
		while (tok().idis(iden))
		{
			args.push_back(tok().val());
			advance();
		}
		if (!tok().valis(")"))
			return LogError<AST::Prototype>("Expected ')' in header");
		advance();
		return llvm::make_unique<AST::Prototype>(fname, std::move(args));
	};

	std::unique_ptr<AST::Function> parseDef()
	{
		if (!tok().valis("func"))
			return nullptr;
		auto proto = parsePrototype();
		if (!proto)
			return nullptr;
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		return llvm::make_unique<AST::Function>(std::move(proto), std::move(expr));
	};

	std::unique_ptr<AST::Function> parseTopLevel()
	{
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		auto proto = llvm::make_unique<AST::Prototype>("", std::vector<std::string>());
		return llvm::make_unique<AST::Function>(std::move(proto), std::move(expr));
	}

	int getPrec()
	{
		if (!isascii(tok().val().at(0)))
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
