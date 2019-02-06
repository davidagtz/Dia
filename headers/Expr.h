#pragma once

#include <vector>
#include <memory>
#include "llvm/IR/LLVMContext.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Module.h"

namespace dia
{

class Base
{
public:
	virtual ~Base() = default;
	virtual llvm::Value *codegen() = 0;
};

class Number : public Base
{
	double val;

public:
	Number(double num) : val(num){};
	llvm::Value *codegen() override;
};

class Variable : public Base
{
	std::string name;

public:
	Variable(std::string n) : name(n){};
	llvm::Value *codegen() override;
};

class Binary : public Base
{
	char op;
	std::unique_ptr<Base> LHS;
	std::unique_ptr<Base> RHS;

public:
	Binary(char oper, std::unique_ptr<Base> left, std::unique_ptr<Base> right)
			: op(oper), LHS(std::move(left)), RHS(std::move(right)){};
	llvm::Value *codegen() override;
};

class Call : public Base
{
	std::string callee;
	std::vector<std::unique_ptr<Base>> args;

public:
	Call(std::string callee, std::vector<std::unique_ptr<Base>> arguments)
			: callee(callee), args(std::move(arguments)){};
	llvm::Value *codegen() override;
};

class Prototype : public Base
{
	std::string name;
	std::vector<std::string> args;

public:
	Prototype(std::string name, std::vector<std::string> args)
			: name(name), args(std::move(args)){};
	llvm::Function *codegen() override;
	std::string getName()
	{
		return name;
	}
};

class Function : public Base
{
	std::unique_ptr<Prototype> prototype;
	std::unique_ptr<Base> body;

public:
	Function(std::unique_ptr<Prototype> proto, std::unique_ptr<Base> body)
			: prototype(std::move(proto)), body(std::move(body)){};
	llvm::Function *codegen() override;
};
} // namespace dia
