#pragma once

#include <vector>
#include <string>
#include "goodFiles.h"
#include "TokenTools.h"

std::vector<token> lex(int argc, char **argv)
{
	using namespace std;

	// Converts arguments to strings
	string args[sizeof(argv)];
	tools::arrToString(argv, args, argc);

	string file = "";
	string out = "";
	bool showTokens = true;

	for (int i = 1; i < argc; i++)
	{
		// Input
		if (args[i].compare("-i") == 0)
		{
			i++;

			file = tools::getFile(args[i]);
		}
		// Output
		else if (args[i].compare("-o") == 0)
		{
			i++;

			out = args[i];

			cout << "Making File " << out << endl;
		}
		// Tokenized?
		else if (args[i].compare("-s") == 0)
		{
			i++;

			showTokens = args[i].compare("both") == 0;

			if (args[i].compare("file") == 0 || showTokens)
				cout << endl
					 << file
					 << endl;
			if (args[i].compare("none") == 0)
				showTokens = false;
		}
	}

	if (file == "")
	{
		cerr << "The lexer requires the file to be tokenized." << endl;
		cerr << "Usage: Lexer -i fileName.da" << endl
			 << endl;
		return err_vector();
	}

	vector<token> tokens = FileTokenizer(file).getTokens();

	string printTokens = "";

	if (!showTokens)
		return tokens;

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