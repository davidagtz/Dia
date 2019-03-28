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
using namespace std;

class Base
{
public:
	virtual ~Base() = default;
	virtual llvm::Value *codegen() = 0;
	virtual llvm::Value *codegen(llvm::Type *type) { codegen(); };
	virtual llvm::Value *type_cast(llvm::Value *from, llvm::Type *to, string msg);
	string type = "";
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
	string name;

public:
	Variable(string n) : name(n) { type = "variable"; };
	llvm::Value *codegen() override;
};

class Binary : public Base
{
	char op;
	unique_ptr<Base> LHS;
	unique_ptr<Base> RHS;

public:
	Binary(char oper, unique_ptr<Base> left, unique_ptr<Base> right)
			: op(oper), LHS(move(left)), RHS(move(right)) { type = "expression"; };
	llvm::Value *codegen() override;
};

class Call : public Base
{
	string callee;
	vector<unique_ptr<Base>> args;

public:
	Call(string callee, vector<unique_ptr<Base>> arguments)
			: callee(callee), args(move(arguments)) { type = "call"; };
	llvm::Value *codegen() override;
	string toString()
	{
		return callee;
	}
};

class Prototype : public Base
{
	string name;
	vector<pair<string, Return>> args;

public:
	Return ret_type;
	Prototype(string name, vector<pair<string, Return>> args, Return ret_type)
			: name(name), args(move(args)), ret_type(ret_type) { type = "prototype"; };
	llvm::Function *codegen() override;
	string getName()
	{
		return name;
	}
};

class Function : public Base
{
	unique_ptr<Prototype> prototype;
	vector<shared_ptr<Base>> body;

public:
	Function(unique_ptr<Prototype> proto, vector<unique_ptr<Base>> Body)
			: prototype(move(proto)), body(move(body))
	{
		type = "function";
		for (int i = 0; i < Body.size(); i++)
			body.push_back(shared_ptr(move(Body.at(i))));
	};
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
	unique_ptr<Base> Cond;
	vector<shared_ptr<Base>> Then, Else;
	// string type;

public:
	If(unique_ptr<Base> cond, vector<unique_ptr<Base>> then, vector<unique_ptr<Base>> Else_a) : Cond(move(cond))
	{
		type = "if";
		for (int i = 0; i < then.size(); i++)
			Then.push_back(shared_ptr(move(then.at(i))));
		for (int i = 0; i < Else_a.size(); i++)
			Else.push_back(shared_ptr(move(Else_a.at(i))));
	};
	llvm::Value *codegen() override;
	llvm::Value *codegen(llvm::Type *cast_to);
};

class From : public Base
{
	string idname;
	unique_ptr<Base> Start, End, Step;
	vector<shared_ptr<Base>> Body;

public:
	From(string idname,
			 unique_ptr<Base> Start,
			 unique_ptr<Base> End,
			 unique_ptr<Base> Step,
			 vector<unique_ptr<Base>> Bodyarg)
			: idname(idname), Start(move(Start)), End(move(End)), Step(move(Step))
	{
		type = "from";
		for (int i = 0; i < Bodyarg.size(); i++)
		{
			Body.push_back(shared_ptr(move(Bodyarg.at(i))));
		}
	};
	llvm::Value *codegen() override;
};

class Give : public Base
{
	unique_ptr<Base> give;

public:
	Give(unique_ptr<Base> give) : give(move(give)) { type = "give"; };
	llvm::Value *codegen() override;
	llvm::Value *codegen(llvm::Type *cast_to);
};
} // namespace dia
