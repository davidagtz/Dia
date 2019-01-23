#pragma once

#include "vector"
#include "TokenTools.h"

class ArrayListString : public std::vector<std::string>
{
  public:
	ArrayListString(int num) : vector(num){};
	ArrayListString(std::initializer_list<std::string> arr) : vector(arr){};
	bool contains(std::string elem)
	{
		for (int i = 0; i < size(); i++)
		{
			if (at(i) == elem)
				return true;
		}
		return false;
	};
};