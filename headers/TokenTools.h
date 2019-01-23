#pragma once

#include <string>
#include <vector>
#include <ctype.h>
#include <iostream>

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
	eol,
	keyword,

	//error handling
	errHandle

};

std::vector<std::string> keywords({"if",
								   "else",
								   "int",
								   "elif",
								   "string",
								   "func",
								   "bool",
								   "from",
								   "to",
								   "as"});

// string ops[] = {};
bool isparen(char a) { return a == '(' || a == ')'; }
bool isnewline(char a) { return a == 0x0A; }
bool isquote(char a) { return a == 0x22; }
// bool isop(char a) { return }

bool equals(std::string s, std::string e) { return s.compare(e) == 0; }

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
	FileTokenizer(std::string thefile) : file(thefile), pos(0), line(1) {}

	void nextToken()
	{
		using namespace std;

		char curChar = file.at(pos);

		string value = string(1, curChar);
		int id = -1;

		if (isalpha(curChar))
		{
			while (isalpha(file.at(pos + 1)) || isdigit(file.at(pos + 1)))
			{
				pos++;
				value += file.at(pos);
			}

			if (equals(value, "true") || equals(value, "false"))
				id = bln;
			else
				id = iden;
		}
		else if (isdigit(curChar))
		{
			bool hasdec = false;

			while (pos < file.size() && (((file.at(pos + 1) == '.') && !hasdec) || isdigit(file.at(pos + 1))))
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
			while (file.at(pos + 1) == 0x0A)
			{
				pos++;
				line++;
			}
			id = eol;
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
		else if (isspace(curChar) && !isnewline(curChar))
		{
			pos++;
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
