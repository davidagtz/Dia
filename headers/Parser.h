#pragma once

#include "Expr.h"
#include "TokenTools.h"
#include "vector"
#include "ArrayList.h"
#include "memory"
#include "../llvm/ADT/STLExtras.h"

#define ull unsigned long long

class Parser
{
	std::vector<token> tokens;
	ull pos;
	ull line;
	std::map<std::string, int> binop;

  public:
	Parser(std::vector<token> toks) : tokens(toks), opprec(3)
	{
		binop["("] = 40;
		binop["*"] = 30;
		binop["/"] = 30;
		binop["+"] = 20;
		binop["-"] = 20;
	};

	AST::BaseAST parse()
	{
		using namespace AST;

		BaseAST program = BaseAST();

		while (pos <= tokens.size())
		{
			if (tok().idis(eol))
			{
				line++;
			}
			else if (tok().idis(keyword))
			{
				// if (tok().valueis("if"))
				// {
				// 	advance();
				// 	if (!tok().valueis("("))
				// 	{
				// 		LogError(line, "Expected Parentheses After is Statement.");
				// 		return BlockAST("error");
				// 	}
				// 	program.addNode(IfAST(parseBinary(), parse()));
				// }
				// else if(tok().valueis("from")){

				// }
			}
			advance();
		}

		return program;
	};

	std::unique_ptr<AST::BaseAST> parseNumber()
	{
		advance();
		return std::move(llvm::make_unique<AST::NumberAST>(tokens.at(pos - 1)));
	};

	std::unique_ptr<AST::BaseAST> parseParenExpr()
	{
		advance();
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		if (tok() != ")")
			return LogError("expected ')'");
		advance();
		return expr;
	};

	std::unique_ptr<AST::BaseAST> parseIden()
	{
		std::string idname = tok();
		advance();
		if (tok() != "(")
			return llvm::make_unique<AST::VariableAST>(idname);
		advance();
		std::vector<std::unique_ptr<BaseAST>> args;
		if (tok() != ")")
			while (1)
			{
				auto Arg = parseExpr();
				if (Arg)
					args.push_back(Arg);
				else
					return nullptr;
				if (tok() == ")")
					break;
				if (tok() != ",")
					return LogError("Expected ',' or ')'");
				advance();
			}
		return llvm::make_unique<AST::CallAST>(idname, std::move(args));
	};

	std::unique_ptr<AST::BaseAST> parsePrimary()
	{
		switch (tok().id())
		{
		case iden:
			return parseIden();
		case paren:
			return parseParenExpr();
		case num:
			return parseNumber();
		default:
			return LogError("Unexpected Token");
		}
	};

	std::unique_ptr<AST::BaseAST> parseExpr()
	{
		auto left = parsePrimary();
		if (!left)
			return nullptr;
		return parseRightBinop(0, std::move(left));
	};

	std::unique_ptr<AST::BaseAST> parseRightBinop(int oprec, unique_ptr<AST::BaseAST> left)
	{
		while (1)
		{
			int prec = getPrec();
			if (getPrec() < oprec)
			{
				return LHS;
			}
			std::string comp = tok();
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
			left = llvm::make_unique<BinaryExprAST>(comp, std::left(left), std::move(right));
		};
	};

	std::unique_ptr<PrototypeAST> parsePrototype()
	{
		if (!tok.idis(iden))
			return LogError("Expected function in prototype");
		std::string fname = tok().val();
		advance();
		if (tok.val() != "(")
			return LogError("Expected '(' in header");
		std::vector<std::string> args;
		while (tok().idis(iden))
		{
			args.push_back(tok.val());
			advance();
		}
		if (tok().val() != ")")
			return LogError("Expected ')' in header");
		advance();
		return llvm::make_unique<PrototypeAST>(fname, std::move(args));
	};

	std::unique_ptr<FunctionAST> parseDef()
	{
		if (!tok.valis("func"))
			return nullptr;
		auto proto = parsePrototype();
		if (!proto)
			return nullptr;
		auto expr = parseExpression();
		if (!expr)
			return nullptr;
		return llvm::make_unique<FunctionAST>(std::move(proto), std::move(expr));
	};

	std::unique_ptr<FunctionAST> parseTopLevel()
	{
		auto expr = parseExpression();
		if (!expr)
			return nullptr;
		auto proto = llvm::make_unique<PrototypeAST>("", std::vector<std::string>());
		return llvm::make_unique(std::move(proto), std::move(expr));
	}

	int getPrec()
	{
		if (!isascii(tok()))
			return -1;
		int prec = binop[tok().val()];
		if (prec <= 0)
			return -1;
		return prec;
	};

	token tok(){return tokens.at(pos)};
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

	std::unique_ptr<AST::BaseAST> LogError(ull line, std::string msg)
	{
		std::cerr << "Line " << line << ": " << msg << std::endl;
		return nullptr;
	};
};
