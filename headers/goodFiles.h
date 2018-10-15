#ifndef iostream
#include <iostream>
#endif

#ifndef string
#include <string>
#endif

#ifndef fstream
#include <fstream>
#endif

#ifndef vector
#include <vector>
#endif
namespace tools
{
std::string getFile(std::string &fileName)
{
	using namespace std;

	ifstream in;

	in.open(fileName);

	if (in.is_open())
	{
		cout << "Opened File." << endl;

		string file("");

		while (!in.eof())
		{
			file += in.get();
		}

		in.close();

		return file;
	}
	else
	{
		cout << "Cannot Open File." << endl;
		return nullptr;
	}
}

void arrToString(char **oarg, std::string *args, int &argco)
{
	using namespace std;
	for (int i = 0; i < argco; i++)
	{
		args[i] = string(oarg[i]);
	}
}
}; // namespace tools