#include <vector>
#include "memory"

namespace AST
{

class BaseAST
{
  public:
	virtual ~BaseAST(){};
};

class NumberAST : public BaseAST
{
	long long val;

  public:
	NumberAST(long long num) : val(num){};
};

class VariableAST : public BaseAST
{
	std::string name;

  public:
	VariableAST(std::string n) : name(n){};
};

class BinaryAST : public BaseAST
{
	std::string op;
	std::unique_ptr<BaseAST> LHS;
	std::unique_ptr<BaseAST> RHS;

  public:
	BinaryAST(std::string oper, std::unique_ptr<BaseAST> left, std::unique_ptr<BaseAST> right)
		: op(oper), LHS(std::move(left)), RHS(std::move(right)){};
};

class CallAST : public BaseAST
{
	std::string callee;
	std::vector<std::unique_ptr<BaseAST>> args;

  public:
	CallAST(std::string callee, std::vector<std::unique_ptr<BaseAST>> arguments)
		: callee(callee), args(std::move(arguments)){};
};

class PrototypeAST : public BaseAST
{
	std::string name;
	std::vector<std::string> args;

  public:
	PrototypeAST(std::string name, std::vector<std::string> args)
		: name(name), args(std::move(args)){};
};

class FunctionAST : public BaseAST
{
	std::unique_ptr<PrototypeAST> prototype;
	std::unique_ptr<BaseAST> body;

  public:
	FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<BaseAST> body)
		: prototype(std::move(proto)), body(std::move(body)){};
};
} // namespace AST
