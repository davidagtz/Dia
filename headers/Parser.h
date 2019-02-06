#pragma once
#define ull unsigned long long

class Parser
{
	std::vector<token> tokens;
	ull pos;
	ull line;
	std::map<std::string, int> binop;

  public:
	Parser(std::vector<token> toks) : tokens(toks), binop(), pos(0), line(0)
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
		// std::cout << "Postion: " << pos << std::endl;
		// std::cout << "Token ID: " << tok().getid() << std::endl;
		// std::cout << "Token Value: " << tok().val() << std::endl
		// 		  << std::endl;
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
			char comp = tok().val()[0];
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
		advance();
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
		advance();
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
		// if (!isalpha(tok().val().at(0)))
		// 	return -1;
		int prec = binop[tok().val()];
		if (prec <= 0)
			return -1;
		return prec;
	};

	token tok() { return tokens.at(pos); };
	void advance() { advance(1); };
	void advance(int i)
	{
		int count = 0;
		do
		{
			pos += 1;
			count += 1;
			if (tok().idis(eol))
			{
				line++;
			}
		} while (tok().idis(eol) && count < i);
		if (pos >= tokens.size())
			std::cout << "Reached end of file." << std::endl;
	};

	template <class T>
	std::unique_ptr<T> LogError(std::string msg)
	{
		std::cerr << "Line " << line << ": " << msg << std::endl;
		return nullptr;
	};
	void handle_top_level();
	void handle_def();
};

#include "handlers.h"