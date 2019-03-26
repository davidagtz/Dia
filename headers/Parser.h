#pragma once

#include <vector>
#include <memory>
#include <map>
#include "TokenTools.h"
#include "Expr.h"
#include "ReturnTypes.h"

#define ull unsigned long long

class Parser
{
	ull line;
	std::map<std::string, int> binop;

  public:
	std::vector<token> tokens;
	ull pos;
	Parser(std::vector<token> toks) : tokens(toks), binop(), pos(0), line(0)
	{
		binop["("] = 40;
		binop["*"] = 30;
		binop["/"] = 30;
		binop["+"] = 20;
		binop["-"] = 20;
		binop[">"] = 10;
		binop["<"] = 10;
	};

	token at(int i)
	{
		return tokens.at(i);
	}

	int size() { return tokens.size(); };

	std::unique_ptr<dia::Base> parseNumber()
	{
		auto num = std::move(std::make_unique<dia::Number>(std::stoi(tokens.at(pos).val())));
		advance();
		return num;
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

	std::unique_ptr<dia::Base> parseFrom()
	{
		if (!tok().idis(tok_from))
			return LogError<dia::Base>("Expected to see 'from'");
		advance();
		if (!tok().idis(iden))
			return LogError<dia::Base>("Expected to see variable name");
		std::string idname = tok().val();
		advance();
		if (!tok().idis(tok_is))
			return LogError<dia::Base>("Expected to see 'is'");
		advance();
		auto start = parseExpr();
		if (!start)
			return nullptr;
		if (!tok().idis(tok_to))
			return LogError<dia::Base>("Expected to see 'to'");
		advance();
		std::cout << tok().id << " " << tok().val() << std::endl;
		auto end = parseExpr();
		std::cout << end->val << std::endl;
		if (!end)
			return nullptr;
		std::unique_ptr<dia::Base> Step = nullptr;
		if (tok().idis(tok_step))
		{
			advance();
			Step = parseExpr();
			if (!Step)
				return nullptr;
		}
		auto body = parseExprBlock();
		if (!body.back())
			return nullptr;
		return std::make_unique<dia::From>(idname,
										   std::move(start),
										   std::move(end),
										   std::move(Step),
										   std::move(body));
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
				{
					advance();
					break;
				}
				if (!tok().valis(","))
					return LogError<dia::Base>(std::string("Expected ',' or ')', but got ") +
											   std::string(tok().val()));
				advance();
			}
		else
			advance();

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
		case tok_num:
			return parseNumber();
		case chr:
			return parseChar();
		case tok_if:
			return parseIf();
		case tok_from:
			return parseFrom();
		case tok_give:
			return parseGive();
		default:
			return LogError<dia::Base>(std::string("Unexpected Token") + tok().toString() + tok().val());
		}
	};

	std::unique_ptr<dia::Give> parseGive()
	{
		advance();
		auto ret = parseExpr();
		if (!ret)
			return nullptr;
		return std::make_unique<dia::Give>(std::move(ret));
	}

	std::unique_ptr<dia::Char> parseChar()
	{
		auto chr = std::move(std::make_unique<dia::Char>(tokens.at(pos).val()[0]));
		advance();
		return chr;
	}

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

	std::unique_ptr<dia::Prototype> parsePrototype(bool isExtern = false)
	{
		// Get the Return Type
		Return ret_type = Return::decimal;
		if (!tok().idis(iden))
		{
			ret_type = getReturnType(tok().val());
			if (ret_type == Return::not_a_type)
				return LogError<dia::Prototype>("Expected function in prototype");
			advance();
		}

		// Get the name
		std::string fname = tok().val();
		advance();

		// Get the arguments
		if (!tok().valis("("))
			return LogError<dia::Prototype>("Expected '(' in header");
		advance();
		std::vector<std::pair<std::string, Return>> args;
		// used for externs if there is no name
		char id = 'A';
		while (tok().idis(iden) || tok().idis(type))
		{
			std::pair<std::string, Return> arg("", Return::not_a_type);
			Return t = getReturnType(tok().val());
			if (t == Return::not_a_type)
			{
				if (isExtern)
					return LogError<dia::Prototype>("Expected type declaration in extern header.");
				t = Return::decimal;
			}
			else
				advance();
			arg.second = t;

			if (tok().idis(iden))
				arg.first = tok().val();
			else
			{
				if (isExtern && (tok().valis(",") || tok().valis(")")))
				{
					arg.first = "" + id;
					id++;
					args.push_back(arg);
					goto end_define;
				}
				return LogError<dia::Prototype>("Not a valid identifier in the arguments");
			}
			args.push_back(arg);
			advance();
		end_define:
			if (!tok().valis(","))
				break;
			advance();
		}
		if (!tok().valis(")"))
			return LogError<dia::Prototype>("Expected ')' in header. Saw " + tok().val());
		advance();
		return std::make_unique<dia::Prototype>(fname, std::move(args), ret_type);
	};

	std::unique_ptr<dia::If> parseIf()
	{
		if (!tok().valis("if"))
			return LogError<dia::If>("Expected 'if'");
		advance();
		if (!tok().valis("("))
			return LogError<dia::If>("Expected condition starting with '('");
		auto cond_expr = parseExpr();
		if (!cond_expr)
			return nullptr;
		auto then_expr_block = parseExprBlock();
		if (!then_expr_block.back())
			return nullptr;
		if (!tok().valis("else"))
			return LogError<dia::If>("Expected else statement.");
		advance();
		auto else_expr_block = parseExprBlock();
		if (!else_expr_block.back())
			return nullptr;
		return std::make_unique<dia::If>(std::move(cond_expr),
										 std::move(then_expr_block),
										 std::move(else_expr_block));
	}

	std::unique_ptr<dia::Function> parseDef()
	{
		if (!tok().valis("fn"))
			return LogError<dia::Function>("Expected Function declaration to start with 'fn'");
		advance();
		auto proto = parsePrototype();
		if (!proto)
			return nullptr;
		std::vector<std::unique_ptr<dia::Base>> body(parseExprBlock());
		if (!body.back())
			return nullptr;
		return std::make_unique<dia::Function>(std::move(proto), std::move(body));
	};

	std::vector<std::unique_ptr<dia::Base>> parseExprBlock()
	{
		std::vector<std::unique_ptr<dia::Base>> body({});
		if (tok().valis("{"))
		{
			advance();
			while (!tok().valis("}"))
			{
				auto expr = parseExpr();
				body.push_back(std::move(expr));
				if (!body.back())
					return body;
			}
			advance();
		}
		else
		{
			auto expr = parseExpr();
			body.push_back(std::move(expr));
			if (!body.back())
				return body;
		}
		return body;
	}

	std::unique_ptr<dia::Function> parseTopLevel()
	{
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		std::vector<std::unique_ptr<dia::Base>> body({});
		body.push_back(std::move(expr));
		auto proto = std::make_unique<dia::Prototype>("",
													  std::vector<std::pair<std::string, Return>>(),
													  Return::decimal);
		return std::make_unique<dia::Function>(std::move(proto), std::move(body));
	}

	Return getReturnType(std::string ret)
	{
		if (equals(ret, "int"))
			return Return::integer;
		else if (equals(ret, "fp"))
			return Return::decimal;
		else if (equals(ret, "chr"))
			return Return::chr;
		else if (equals(ret, "void"))
			return Return::null;
		return Return::not_a_type;
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
		// int j = 0;
		// while (i != j && pos < size())
		// {
		// 	do
		// 	{
		// 		pos++;
		// 		j++;
		// 	} while (pos < size() && tok().idis(eol));
		// }
		if (pos + i >= tokens.size())
		{
			pos = tokens.size() - 1;
		}
		else
		{
			pos += i;
			while (tok().idis(eol))
			{
				pos += 1;
				line += 1;
			}
		}
	};

	template <class T>
	std::unique_ptr<T> LogError(std::string msg)
	{
		std::cerr << "Line " << line << ": " << msg << std::endl;
		return nullptr;
	};
	void handle_top_level(llvm::Function *f, llvm::BasicBlock *BB);
	void handle_def();
	void handle_extern();
};

#include "handlers.h"