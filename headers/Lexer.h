#pragma once

#include <vector>
#include <string>
#include "goodFiles.h"
#include "TokenTools.h"

std::string output = "";

std::vector<token> lex(int argc, char **argv)
{
	using namespace std;

	if (argc == 1)
	{
		cerr << "There must be an input file." << endl;
		cout << "Usage:" << endl;
		cout << "    dia [PROGRAM] [flags...]" << endl;
		return err_vector();
	}

	// Converts arguments to strings
	string args[sizeof(argv)];
	tools::arrToString(argv, args, argc);

	// Get the file
	string file = tools::getFile(args[1]);

	string out = "a.out";
	bool showTokens = false;

	for (int i = 2; i < argc; i++)
	{
		// Output
		if (args[i].compare("-o") == 0)
		{
			i++;
			output = args[i];
		}
		// Tokenized?
		else if (args[i].compare("-s") == 0)
		{
			i++;

			showTokens = (args[i].compare("both") == 0) || (args[i].compare("tokens") == 0);

			if (args[i].compare("file") == 0 || showTokens)
				cout << endl
					 << file
					 << endl;
			if (args[i].compare("none") == 0)
				showTokens = false;
		}
	}

	vector<token> tokens = FileTokenizer(file).getTokens();

	if (!showTokens)
		return tokens;

	string printTokens = "";

	for (int i = 0; i < tokens.size(); i++)
	{
		printTokens += to_string(i) + tokens.at(i).toString() + "\n";
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