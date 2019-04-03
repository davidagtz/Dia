#pragma once

#define ull unsigned long long

enum tok_id
{
	// types

	tok_num,
	tok_str,
	tok_bln,
	tok_op,
	tok_paren,
	iden,
	def,
	eol,
	keyword,
	ext,
	tok_type,
	chr,
	tok_if,
	tok_else,
	tok_inc,
	tok_from,
	tok_is,
	tok_to,
	tok_step,
	tok_give,

	//error handling
	errHandle

};

std::vector<std::string> keywords({});
std::vector<std::string> types({"int",
								"fp"});
std::map<std::string, int> binop({
	{"(", 40},
	{"*", 30},
	{"/", 30},
	{"+", 20},
	{"-", 20},
	{">", 10},
	{"<", 10},
	{"==", 10},
	{"<=", 10},
	{">=", 10},
	{"!=", 10},
	{"=", 5},
});
// If some contain others, put the longer ones first
std::vector<std::string> ops({"*", "/", "+", "-", "<=", ">=", ">", "<", "==", "=", "!="});

bool isparen(char a)
{
	return a == '(' || a == ')';
}
bool isnewline(char a) { return a == '\n'; }
bool isquote(char a) { return a == 0x22; }
bool issinglequote(char a) { return a == '\''; }
bool isbslash(char a) { return a == '\\'; }
bool isfslash(char a) { return a == '/'; }
bool isuscore(char a) { return a == '_'; }
bool ishyphen(char a) { return a == '-'; }

bool equals(std::string s, std::string e) { return s.compare(e) == 0; }
bool contains(std::vector<std::string> list, std::string str)
{
	for (int i = 0; i < list.size(); i++)
		if (equals(list.at(i), str))
			return true;
	return false;
}

class token
{
#define curChar file.at(pos)
#define to(i) (file.at(pos + i))
  public:
	std::string value;
	int id;
	unsigned long long line;

	std::string toString()
	{
		return (value == "\n" ? "eol" : value) + " " + std::to_string(id);
	}

	token(std::string val, int identifier, unsigned long long line) : value(val), id(identifier), line(line) {}
	token() {}

	bool valis(std::string val) { return value.compare(val) == 0; }
	int getid() { return id; };
	std::string val() { return value; };
	bool idis(int a) { return a == id; }
};

std::vector<token> err_vector()
{
	return std::vector<token>({token("Error", errHandle, 0)});
};

class FileTokenizer
{
	std::string file;
	std::vector<token> tokens;
	token CurTok;
	ull pos;
	ull line;

  public:
	FileTokenizer(std::string thefile) : tokens(), file(thefile), pos(0), line(1) {}
	std::string getop()
	{
		for (auto arg : ops)
		{
			for (int i = 0; i < arg.size(); i++)
			{
				if (to(i) != arg[i])
					break;
				if (arg.size() - 1 == i)
					return arg;
			}
		}
		return "";
	}

	void nextToken()
	{
		using namespace std;

		string value = string(1, curChar);
		int id = -1;

		if (isfslash(curChar) && isfslash(to(1)))
		{
			while (pos < file.size() && !isnewline(curChar))
			{
				if (isnewline(curChar))
					line++;
				pos++;
			}
			if (pos == file.size())
			{
				CurTok = token("\n", eol, line);
				return;
			}
			curChar = curChar;
			string value = string(1, curChar);
		}

		if (isalpha(curChar))
		{
			while (pos + 1 < file.size() && (isalpha(to(1)) || isdigit(to(1)) || isuscore(to(1))))
			{
				pos++;
				value += curChar;
			}

			if (equals(value, "true") || equals(value, "false"))
				id = tok_bln;
			else if (equals(value, "fn"))
				id = def;
			else if (equals(value, "import"))
				id = ext;
			else if (equals(value, "if"))
				id = tok_if;
			else if (equals(value, "else"))
				id = tok_else;
			else if (equals(value, "from"))
				id = tok_from;
			else if (equals(value, "is"))
				id = tok_is;
			else if (equals(value, "to"))
				id = tok_to;
			else if (equals(value, "step"))
				id = tok_step;
			else if (equals(value, "give"))
				id = tok_give;
			else if (equals(value, "include"))
				id = tok_inc;
			else if (contains(keywords, value))
				id = keyword;
			else if (contains(types, value))
				id = tok_type;
			else
				id = iden;
		}
		else if (isdigit(curChar) ||
				 (ishyphen(curChar) && !isdigit(to(-1)) && (isdigit(to(1)) || to(1) == '.')))
		{
			bool hasdec = false;

			while (pos + 1 < file.size() && (((to(1) == '.') && !hasdec) || isdigit(to(1))))
			{
				pos++;

				value += curChar;

				if (curChar == '.')
				{
					hasdec = true;
				}
			}

			id = tok_num;
		}
		else if (isparen(curChar))
		{
			id = tok_paren;
		}
		else if (auto op = getop(); op != "")
		{
			id = tok_op;
			value = op;
			pos += value.size();
		}
		else if (isnewline(curChar))
		{
			line++;
			while (isnewline(to(1)))
			{
				pos++;
				line++;
			}
			id = eol;
			value = "\n";
		}
		else if (isquote(curChar))
		{
			value = "";
			while (file.size() > pos && !isquote(to(1)))
			{
				pos++;

				if (curChar == '\\')
				{
					pos++;
					if (curChar == '\"')
					{
						value += "\"";
					}
					else if (curChar == '\\')
					{
						value += "\\";
					}
					else
					{
						LogError("Invalid Escape Sequence", line);
						return;
					}
					pos++;
				}

				value += curChar;
			}
			pos++;
			id = tok_str;
		}
		else if (issinglequote(curChar))
		{
			pos++;
			if (issinglequote(curChar))
			{
				LogError("Character declaration empty.", line);
				return;
			}
			if (isbslash(curChar))
			{
				pos++;
				switch (curChar)
				{
				case 'a':
					value = string(1, '\a');
					break;
				case 'b':
					value = string(1, '\b');
					break;
				case 'f':
					value = string(1, '\f');
					break;
				case 'n':
					value = string(1, '\n');
					break;
				case 'r':
					value = string(1, '\r');
					break;
				case 't':
					value = string(1, '\t');
					break;
				case 'v':
					value = string(1, '\v');
					break;
				case '\\':
					value = string(1, '\\');
					break;
				case '\'':
					value = string(1, '\'');
					break;
				}
			}
			else
				value = string(1, curChar);
			id = chr;
			pos++;
			if (!issinglequote(curChar))
			{
				LogError("Character declaration has more than one character declared.", line);
				return;
			}
		}
		else if (isspace(curChar) && !isnewline(curChar))
		{
			pos++;
			if (pos >= file.size())
				return;
			nextToken();
			return;
		}

		CurTok = token(value, id, line);
		pos++;
#undef curChar
#undef to
	}

	void LogError(std::string err, ull l)
	{
		std::cerr << "line " << l << ": " << err << std::endl;

		CurTok = token(err, errHandle, l);
	}

	char get(int off) { return file.at(pos + off); }

	std::vector<token> getTokens()
	{
		while (pos < file.size())
		{
			nextToken();
			tokens.push_back(CurTok);

			if (CurTok.id == errHandle)
			{
				tokens = std::vector<token>();
				tokens.push_back(CurTok);
			}
		}
		return tokens;
	}
};