#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

namespace tools
{
std::string getFile(std::string fileName)
{
	using namespace std;

	ifstream in;
	in.open(fileName);

	if (in.is_open())
	{
		string file("");
		while (!in.eof())
			file += in.get();
		in.close();
		return file;
	}
	else
	{
		cout << "Cannot Open File." << endl;
		return nullptr;
	}
}
void arrToString(char **oarg, std::string *args, int &argc)
{
	for (int i = 0; i < argc; i++)
		args[i] = std::string(oarg[i]);
}
}; // namespace tools