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
#include "ReturnTypes.h"

namespace dia
{

class Base
{
public:
	virtual ~Base() = default;
	virtual llvm::Value *codegen() = 0;
	virtual llvm::Value *codegen(llvm::Type *type) { codegen(); };
	virtual llvm::Value *type_cast(llvm::Value *from, llvm::Type *to, std::string msg);
	std::string type = "";
	unsigned long long line = 0;
	double val = 0;
};

class Number : public Base
{
	double val;

public:
	Number(double num)
	{
		type = "number";
		val = num;
	};
	llvm::Value *codegen() override;
};

class Variable : public Base
{
	std::string name;

public:
	Variable(std::string n) : name(n) { type = "variable"; };
	llvm::Value *codegen() override;
};

class Binary : public Base
{
	char op;
	std::unique_ptr<Base> LHS;
	std::unique_ptr<Base> RHS;

public:
	Binary(char oper, std::unique_ptr<Base> left, std::unique_ptr<Base> right)
			: op(oper), LHS(std::move(left)), RHS(std::move(right)) { type = "expression"; };
	llvm::Value *codegen() override;
};

class Call : public Base
{
	std::string callee;
	std::vector<std::unique_ptr<Base>> args;

public:
	Call(std::string callee, std::vector<std::unique_ptr<Base>> arguments)
			: callee(callee), args(std::move(arguments)) { type = "call"; };
	llvm::Value *codegen() override;
	std::string toString()
	{
		return callee;
	}
};

class Prototype : public Base
{
	std::string name;
	std::vector<std::pair<std::string, Return>> args;

public:
	Return ret_type;
	Prototype(std::string name, std::vector<std::pair<std::string, Return>> args, Return ret_type)
			: name(name), args(std::move(args)), ret_type(ret_type) { type = "prototype"; };
	llvm::Function *codegen() override;
	std::string getName()
	{
		return name;
	}
};

class Function : public Base
{
	std::unique_ptr<Prototype> prototype;
	std::vector<std::unique_ptr<Base>> body;

public:
	Function(std::unique_ptr<Prototype> proto, std::vector<std::unique_ptr<Base>> body)
			: prototype(std::move(proto)), body(std::move(body)) { type = "function"; };
	llvm::Function *codegen() override;
};

class Char : public Base
{
	char val;

public:
	Char(char val) : val(val) { type = "character"; };
	llvm::Value *codegen() override;
};

class If : public Base
{
	std::unique_ptr<Base> Cond;
	std::vector<std::unique_ptr<Base>> Then, Else;
	// std::string type;

public:
	If(std::unique_ptr<Base> cond,
		 std::vector<std::unique_ptr<Base>> then,
		 std::vector<std::unique_ptr<Base>> Else)
			: Cond(std::move(cond)), Then(std::move(then)), Else(std::move(Else)) { type = "if"; };
	llvm::Value *codegen() override;
	llvm::Value *codegen(llvm::Type *cast_to);
};

class From : public Base
{
	std::string idname;
	std::unique_ptr<Base> Start, End, Step;
	std::vector<std::unique_ptr<Base>> Body;

public:
	From(std::string idname,
			 std::unique_ptr<Base> Start,
			 std::unique_ptr<Base> End,
			 std::unique_ptr<Base> Step,
			 std::vector<std::unique_ptr<Base>> Body)
			: idname(idname),
				Start(std::move(Start)),
				End(std::move(End)),
				Step(std::move(Step)),
				Body(std::move(Body)) { type = "from"; };
	llvm::Value *codegen() override;
};

class Give : public Base
{
	std::unique_ptr<Base> give;

public:
	Give(std::unique_ptr<Base> give) : give(std::move(give)) { type = "give"; };
	llvm::Value *codegen() override;
	llvm::Value *codegen(llvm::Type *cast_to);
};
} // namespace dia
