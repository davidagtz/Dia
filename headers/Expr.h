#include <vector>
#include "memory"

namespace AST
{

class Base
{
  public:
	virtual ~Base() = default;
};

class Number : public Base
{
	long long val;

  public:
	Number(long long num) : val(num){};
};

class Variable : public Base
{
	std::string name;

  public:
	Variable(std::string n) : name(n){};
};

class Binary : public Base
{
	std::string op;
	std::unique_ptr<Base> LHS;
	std::unique_ptr<Base> RHS;

  public:
	Binary(std::string oper, std::unique_ptr<Base> left, std::unique_ptr<Base> right)
		: op(oper), LHS(std::move(left)), RHS(std::move(right)){};
};

class Call : public Base
{
	std::string callee;
	std::vector<std::unique_ptr<Base>> args;

  public:
	Call(std::string callee, std::vector<std::unique_ptr<Base>> arguments)
		: callee(callee), args(std::move(arguments)){};
};

class Prototype : public Base
{
	std::string name;
	std::vector<std::string> args;

  public:
	Prototype(std::string name, std::vector<std::string> args)
		: name(name), args(std::move(args)){};
};

class Function : public Base
{
	std::unique_ptr<Prototype> prototype;
	std::unique_ptr<Base> body;

  public:
	Function(std::unique_ptr<Prototype> proto, std::unique_ptr<Base> body)
		: prototype(std::move(proto)), body(std::move(body)){};
};
} // namespace AST
