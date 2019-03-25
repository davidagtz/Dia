#pragma once

#define ull unsigned long long

enum tok_id
{
	// types

	tok_num,
	tok_str,
	tok_bln,
	tok_op,
	paren,
	iden,
	def,
	eol,
	keyword,
	ext,
	type,
	chr,
	tok_if,
	tok_else,
	cmt,
	inc,
	tok_from,
	tok_is,
	tok_to,
	tok_step,

	//error handling
	errHandle

};

std::vector<std::string> keywords({});
std::vector<std::string> types({"int",
								"fp"});

bool isparen(char a) { return a == '(' || a == ')'; }
bool isnewline(char a) { return a == '\n'; }
bool isquote(char a) { return a == 0x22; }
bool issinglequote(char a) { return a == '\''; }
bool isbslash(char a) { return a == '\\'; }
bool isfslash(char a) { return a == '/'; }
bool isuscore(char a) { return a == '_'; }

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
  public:
	std::string value;
	int id;
	unsigned long long line;

	std::string toString()
	{
		return (value == "\n" ? "eol" : value) + " " + std::to_string(id);
	}

	token(std::string val, int identifier, unsigned long long line) : value(val), id(identifier) {}
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

	void nextToken()
	{
		using namespace std;

#define curChar file.at(pos)
#define to(i) (file.at(pos + i))

		string value = string(1, curChar);
		int id = -1;

		if (isfslash(curChar))
		{
			if (isfslash(to(1)))
			{
				while (!isnewline(curChar))
				{
					pos++;
				}
				curChar = curChar;
				string value = string(1, curChar);
			}
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
			else if (contains(keywords, value))
				id = keyword;
			else if (contains(types, value))
				id = type;
			else
				id = iden;
		}
		else if (isdigit(curChar))
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
			id = paren;
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