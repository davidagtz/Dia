#pragma once

#define ull unsigned long long

enum tok_id
{
	// types

	num,
	str,
	bln,
	op,
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

	//error handling
	errHandle

};

std::vector<std::string> keywords({"elif",
								   "string",
								   "bool",
								   "from",
								   "to",
								   "as"});
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

	std::string toString()
	{
		return (value == "\n" ? "eol" : value) + " " + std::to_string(id);
	}

	token(std::string val, int identifier) : value(val), id(identifier) {}
	token() {}

	bool valis(std::string val) { return value.compare(val) == 0; }
	int getid() { return id; };
	std::string val() { return value; };
	bool idis(int a) { return a == id; }
};

std::vector<token> err_vector()
{
	return std::vector<token>({token("Error", errHandle)});
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

		char curChar = file.at(pos);

		string value = string(1, curChar);
		int id = -1;

		if (isfslash(curChar))
		{
			if (isfslash(file.at(pos + 1)))
			{
				while (!isnewline(file.at(pos)))
				{
					pos++;
				}
				curChar = file.at(pos);
				string value = string(1, curChar);
			}
		}

		if (isalpha(curChar))
		{
			while (pos + 1 < file.size() && (isalpha(file.at(pos + 1)) || isdigit(file.at(pos + 1)) || isuscore(file.at(pos + 1))))
			{
				pos++;
				value += file.at(pos);
			}

			if (equals(value, "true") || equals(value, "false"))
				id = bln;
			else if (equals(value, "fn"))
				id = def;
			else if (equals(value, "import"))
				id = ext;
			else if (equals(value, "if"))
				id = tok_if;
			else if (equals(value, "else"))
				id = tok_else;
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

			while (pos + 1 < file.size() && (((file.at(pos + 1) == '.') && !hasdec) || isdigit(file.at(pos + 1))))
			{
				pos++;

				value += file.at(pos);

				if (file.at(pos) == '.')
				{
					hasdec = true;
				}
			}

			id = num;
		}
		else if (isparen(curChar))
		{
			id = paren;
		}
		else if (isnewline(curChar))
		{
			line++;
			while (isnewline(file.at(pos + 1)))
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
			while (file.size() > pos && !isquote(file.at(pos + 1)))
			{
				pos++;

				if (file.at(pos) == '\\')
				{
					pos++;
					if (file.at(pos) == '\"')
					{
						value += "\"";
					}
					else if (file.at(pos) == '\\')
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

				value += file.at(pos);
			}
			pos++;
			id = str;
		}
		else if (issinglequote(curChar))
		{
			pos++;
			if (issinglequote(file.at(pos)))
			{
				LogError("Character declaration empty.", line);
				return;
			}
			if (isbslash(file.at(pos)))
			{
				pos++;
				switch (file.at(pos))
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
				value = string(1, file.at(pos));
			id = chr;
			pos++;
			if (!issinglequote(file.at(pos)))
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

		CurTok = token(value, id);
		pos++;
	}

	void LogError(std::string err, ull l)
	{
		std::cerr << "line " << l << ": " << err << std::endl;

		CurTok = token(err, errHandle);
	}

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

std::vector<std::string> split(std::string arg, std::string delim)
{
	std::vector<std::string> splitted({});
	int i = 0;
	do
	{
		while (equals(arg.substr(i, i + delim.size()), delim))
		{
			arg = arg.substr(1);
		}
		i = arg.find(delim);
		splitted.push_back(arg.substr(0, i));
		arg = arg.substr(i + delim.size());
	} while (i != -1);
	return splitted;
};