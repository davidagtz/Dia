#pragma once
#include <vector>
#include "goodFiles.h"
#include <string>
#include "TokenTools.h"

std::vector<token> lex(int argc, char **argv)
{
	using namespace std;
	using namespace tools;
	if (argc < 3)
	{
		cerr << "The lexer takes in only one argument which is the file to be tokenized.";
		cerr << "Usage: Lexer -i fileName.da";
		return err_vector();
	}

	string args[sizeof(argv)];
	arrToString(argv, args, argc);

	string file;
	string out = "";
	bool showTokens = false;

	for (int i = 0; i < argc; i++)
	{
		if (args[i].compare("-i") == 0)
		{
			i++;

			file = getFile(args[i]);

			continue;
		}
		else if (args[i].compare("-o") == 0)
		{
			i++;

			out = args[i];

			cout << "Making File " << out << endl;

			continue;
		}
		else if (args[i].compare("-s") == 0)
		{
			i++;

			showTokens = args[i].compare("both") == 0;

			if (args[i].compare("file") == 0 || showTokens)
				cout << endl
					 << file
					 << endl;

			continue;
		}
	}

	vector<token> tokens = FileTokenizer(file).getTokens();

	string printTokens = "";

	if (!showTokens)
		return tokens;

	for (token t : tokens)
	{
		printTokens += t.toString() + "\n";
	}

	if (out.compare("") == 0)
	{
		cout << "tokens:" << endl;
		cout << printTokens << endl;
	}
	else
	{
		cout << "Printing..." << endl;

		ofstream output(out);

		if (output.is_open())
		{
			output << printTokens;
			output.close();
		}
	}

	return tokens;
}