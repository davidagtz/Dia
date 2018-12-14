#pragma once

#include <vector>
#include <memory>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

namespace AST
{

using namespace llvm;

class Base
{
  public:
	virtual ~Base() = default;
	virtual Value *codegen() = 0;
};

class Number : public Base
{
	long long val;

  public:
	Number(long long num) : val(num){};
	Value *codegen(){};
};

class Variable : public Base
{
	std::string name;

  public:
	Variable(std::string n) : name(n){};
	Value *codegen(){};
};

class Binary : public Base
{
	std::string op;
	std::unique_ptr<Base> LHS;
	std::unique_ptr<Base> RHS;

  public:
	Binary(std::string oper, std::unique_ptr<Base> left, std::unique_ptr<Base> right)
		: op(oper), LHS(std::move(left)), RHS(std::move(right)){};
	Value *codegen(){};
};

class Call : public Base
{
	std::string callee;
	std::vector<std::unique_ptr<Base>> args;

  public:
	Call(std::string callee, std::vector<std::unique_ptr<Base>> arguments)
		: callee(callee), args(std::move(arguments)){};
	Value *codegen(){};
};

class Prototype : public Base
{
	std::string name;
	std::vector<std::string> args;

  public:
	Prototype(std::string name, std::vector<std::string> args)
		: name(name), args(std::move(args)){};
	Value *codegen(){};
};

class Function : public Base
{
	std::unique_ptr<Prototype> prototype;
	std::unique_ptr<Base> body;

  public:
	Function(std::unique_ptr<Prototype> proto, std::unique_ptr<Base> body)
		: prototype(std::move(proto)), body(std::move(body)){};
	Value *codegen(){};
};
} // namespace AST
