#pragma once

#define ull unsigned long long
namespace dia
{
class Parser
{
	ull line;

  public:
	vector<token> tokens;
	ull pos;
	Parser(vector<token> toks) : tokens(toks), pos(0), line(0){};

	token at(int i)
	{
		return tokens.at(i);
	}

	int size() { return tokens.size(); };

	unique_ptr<Base> parseNumber()
	{
		auto num = std::make_unique<Number>(stod(tokens.at(pos).val()));
		advance();
		return num;
	};

	unique_ptr<Base> parseParenExpr()
	{
		advance();
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		if (!tok().valis(")"))
			return LogError<Base>("expected ')'");
		advance();
		return expr;
	};

	unique_ptr<Base> parseFrom()
	{
		if (!tok().idis(tok_from))
			return LogError<Base>("Expected to see 'from'");
		advance();
		if (!tok().idis(iden))
			return LogError<Base>("Expected to see variable name");
		string idname = tok().val();
		advance();
		if (!tok().idis(tok_is))
			return LogError<Base>("Expected to see 'is'");
		advance();
		auto start = parseExpr();
		if (!start)
			return nullptr;
		if (!tok().idis(tok_to))
			return LogError<Base>("Expected to see 'to'");
		advance();
		// cout << tok().id << " " << tok().val() << endl;
		auto end = parseExpr();
		// cout << end->val << endl;
		if (!end)
			return nullptr;
		unique_ptr<Base> Step = nullptr;
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
		return std::make_unique<From>(idname, move(start), move(end), move(Step), move(body));
	};

	unique_ptr<Base> parseIden()
	{
		string idname = tok().val();
		advance();
		if (!tok().valis("("))
			return std::make_unique<Variable>(idname);
		advance();
		vector<unique_ptr<Base>> args;
		if (!tok().valis(")"))
			while (1)
			{
				auto Arg = parseExpr();
				if (Arg)
					args.push_back(move(Arg));
				else
					return nullptr;
				if (tok().valis(")"))
				{
					advance();
					break;
				}
				if (!tok().valis(","))
					return LogError<Base>(string("Expected ',' or ')', but got ") +
										  string(tok().val()));
				advance();
			}
		else
			advance();

		return std::make_unique<Call>(idname, move(args));
	};

	unique_ptr<Base> parsePrimary()
	{
		switch (tok().getid())
		{
		case iden:
			return parseIden();
		case tok_paren:
			return parseParenExpr();
		case tok_num:
			return parseNumber();
		case chr:
			return parseChar();
		// case tok_type:
		// 	return parseVar();
		case tok_if:
			return parseIf();
		case tok_from:
			return parseFrom();
		case tok_give:
			return parseGive();
		default:
			return LogError<Base>(string("Line: ") + to_string(tok().line) + string(" Unexpected Token: ") + tok().toString());
		}
	};

	// unique_ptr<Var> parseVar()
	// {
	// }

	void parseInclude()
	{
		advance();
		// cout << tok().idis(tok_str) << endl;
		if (!tok().idis(tok_str))
		{
			LogError<Base>("Expected to see string after include statement");
			return;
		}
		vector<token> ts = FileTokenizer(tools::getFile(tok().val())).getTokens();
		advance();
		tokens.insert(tokens.begin() + pos, ts.begin(), ts.end() - 1);
	}

	unique_ptr<Give> parseGive()
	{
		advance();
		auto ret = parseExpr();
		if (!ret)
			return nullptr;
		return std::make_unique<Give>(move(ret));
	}

	unique_ptr<Char> parseChar()
	{
		auto chr = move(std::make_unique<Char>(tokens.at(pos).val()[0]));
		advance();
		return chr;
	}

	unique_ptr<Base> parseExpr()
	{
		auto left = parsePrimary();
		if (!left)
			return nullptr;
		return parseRightBinop(0, move(left));
	};

	unique_ptr<Base> parseRightBinop(int oprec, unique_ptr<Base> left)
	{
		while (1)
		{
			int prec = getPrec();
			if (getPrec() < oprec)
				return left;
			string comp = tok().val();
			advance();
			auto right = parsePrimary();
			if (!right)
				return nullptr;
			int nprec = getPrec();
			if (prec < nprec)
			{
				right = parseRightBinop(prec + 1, move(right));
				if (!right)
					return nullptr;
			}
			left = std::make_unique<Binary>(comp, move(left), move(right));
		};
	};

	unique_ptr<Prototype> parsePrototype(bool isExtern = false)
	{
		// Get the Return Type
		Return ret_type = Return::decimal;
		if (!tok().idis(iden))
		{
			ret_type = getReturnType(tok().val());
			if (ret_type == Return::not_a_type)
				return LogError<Prototype>("Expected function in prototype");
			advance();
		}

		// Get the name
		string fname = tok().val();
		advance();

		// Get the arguments
		if (!tok().valis("("))
			return LogError<Prototype>("Expected '(' in header");
		advance();
		vector<pair<string, Return>> args;
		// used for externs if there is no name
		char id = 'A';
		while (tok().idis(iden) || tok().idis(tok_type))
		{
			pair<string, Return> arg("", Return::not_a_type);
			Return t = getReturnType(tok().val());
			if (t == Return::not_a_type)
			{
				if (isExtern)
					return LogError<Prototype>("Expected type declaration in extern header.");
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
				return LogError<Prototype>("Not a valid identifier in the arguments");
			}
			args.push_back(arg);
			advance();
		end_define:
			if (!tok().valis(","))
				break;
			advance();
		}
		if (!tok().valis(")"))
			return LogError<Prototype>("Expected ')' in header. Saw " + tok().val());
		advance();
		return std::make_unique<Prototype>(fname, move(args), ret_type);
	};

	unique_ptr<If> parseIf()
	{
		advance();
		if (!tok().valis("("))
			return LogError<If>("Expected condition starting with '('");
		auto cond_expr = parseExpr();
		if (!cond_expr)
			return nullptr;
		auto then_expr_block = parseExprBlock();
		if (!then_expr_block.back())
			return nullptr;
		vector<unique_ptr<Base>> else_expr_block({});
		if (!tok().idis(tok_else))
			else_expr_block.push_back(move(std::make_unique<Number>(1)));
		else
		{
			advance();
			else_expr_block = parseExprBlock();
			if (!else_expr_block.back())
				return nullptr;
		}
		return std::make_unique<If>(move(cond_expr), move(then_expr_block), move(else_expr_block));
	}

	unique_ptr<Function> parseDef()
	{
		advance();
		auto proto = parsePrototype();
		if (!proto)
			return nullptr;
		vector<unique_ptr<Base>> body(parseExprBlock());
		if (!body.back())
			return nullptr;
		return std::make_unique<Function>(move(proto), move(body));
	};

	vector<unique_ptr<Base>> parseExprBlock()
	{
		vector<unique_ptr<Base>> body({});
		if (tok().valis("{"))
		{
			advance();
			while (!tok().valis("}"))
			{
				auto expr = parseExpr();
				body.push_back(move(expr));
				if (!body.back())
					return body;
			}
			advance();
		}
		else
		{
			auto expr = parseExpr();
			body.push_back(move(expr));
			if (!body.back())
				return body;
		}
		return body;
	}

	unique_ptr<Function> parseTopLevel()
	{
		auto expr = parseExpr();
		if (!expr)
			return nullptr;
		vector<unique_ptr<Base>> body({});
		body.push_back(move(expr));
		auto proto = std::make_unique<Prototype>("", vector<pair<string, Return>>(), Return::decimal);
		return std::make_unique<Function>(move(proto), move(body));
	}

	Return getReturnType(string ret)
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
		return (binop[tok().val()] != 0 && (tok().idis(tok_op) || tok().idis(tok_paren))) ? binop[tok().val()] : -1;
	};

	token tok() { return tokens.at(pos); };
	void advance()
	{
		do
			pos++;
		while (tok().idis(eol) && pos < size());
	};

	template <class T>
	unique_ptr<T> LogError(string msg)
	{
		cerr << "Line " << line << ": " << msg << endl;
		return nullptr;
	};
	void handle_top_level(llvm::Function *f, llvm::BasicBlock *BB);
	void handle_def();
	void handle_extern();
};
} // namespace dia