#pragma once

#include <memory>
#include <string>
#include "Expr.h"
#include "ReturnTypes.h"

namespace dia
{
using namespace llvm;
#define MakeFPType Type::getDoubleTy(TheContext)
#define Makei32Type Type::getInt32Ty(TheContext)

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static map<string, llvm::AllocaInst *> NamedValues;
static unique_ptr<llvm::Module> TheModule;

AllocaInst *CreateEntryAlloca(llvm::Function *TheFunction, std::string VarName, Type *t)
{
	IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
	if (t->isDoubleTy())
		return TmpB.CreateAlloca(MakeFPType, 0, VarName.c_str());
	else if (t->isIntegerTy())
		return TmpB.CreateAlloca(Makei32Type, 0, VarName.c_str());
	return nullptr;
}

llvm::Value *cast_codegen(shared_ptr<Base> val, Type *cast_to)
{
	if (!val->type.compare("if") || !val->type.compare("give"))
		return val->codegen(cast_to);
	else
		return val->codegen();
}

llvm::Value *LogError(string msg, unsigned long long line)
{
	cout << "Line: " << line << endl;
	cout << msg << endl;
	return nullptr;
}

llvm::Function *LogErrorP(string msg, unsigned long long line)
{
	cout << "Line: " << line << endl;
	cout << msg << endl;
	return nullptr;
}

llvm::Type *Base::getType(Return r)
{
	switch (r)
	{
	case Return::integer:
		return Makei32Type;
	case Return::decimal:
		return MakeFPType;
	}
}

llvm::Value *Base::type_cast(llvm::Value *from, llvm::Type *to, string msg = "")
{
	if (from->getType()->isIntegerTy() ^ to->isIntegerTy())
	{
		if (from->getType()->isIntegerTy())
			from = Builder.CreateSIToFP(from, MakeFPType, "casttmp");
		else if (from->getType()->isDoubleTy())
			from = Builder.CreateFPToSI(from, Makei32Type, "casttmp");
		else
			return LogError(string("Invalid Type ") + msg, line);
	}
	return from;
}

llvm::Value *Number::codegen()
{
	return llvm::ConstantFP::get(TheContext, llvm::APFloat(val));
};

llvm::Value *Variable::codegen()
{
	llvm::AllocaInst *V = NamedValues[name];
	if (!V)
		return LogError(std::string("Unknown Var Name: ") + name, line);
	return Builder.CreateLoad(V, name.c_str());
};

llvm::Value *Binary::codegen()
{
	if (op == "=")
	{
		dia::Variable *lhs = dynamic_cast<dia::Variable *>(LHS.get());
		if (!lhs)
			return LogError("Destination of '=' must be a variable", line);
		llvm::Value *val = RHS->codegen();
		if (!val)
			return nullptr;
		if (RHS->type == "function" ||
			RHS->type == "give" ||
			RHS->type == "from" ||
			RHS->type == "if" ||
			RHS->type == "prototype")
			return LogError("Invalid value for variable", line);
		llvm::AllocaInst *var = NamedValues[lhs->getName()];
		if (!var)
			return LogError("Unknown variable name", line);
		Builder.CreateStore(type_cast(val, var->getAllocatedType()), var);
		return val;
	}

	llvm::Value *L = LHS->codegen();
	llvm::Value *R = RHS->codegen();
	if (!L || !R)
		return nullptr;

	// Type cast to double if they are ints
	if (L->getType()->isIntegerTy())
		L = Builder.CreateSIToFP(L, MakeFPType, "casttmp");
	if (R->getType()->isIntegerTy())
		R = Builder.CreateSIToFP(R, MakeFPType, "casttmp");

	// Switch for greater than to less than
	if (op == ">")
	{
		llvm::Value *t = L;
		L = R;
		R = t;
		op = "<";
	}

	if (op == "+")
		return Builder.CreateFAdd(L, R, "addtmp");
	else if (op == "-")
		return Builder.CreateFSub(L, R, "subtmp");
	else if (op == "*")
		return Builder.CreateFMul(L, R, "multmp");
	else if (op == "/")
		return Builder.CreateFDiv(L, R, "fdivtmp");
	else if (op == "<")
	{
		L = Builder.CreateFCmpULT(L, R, "cmptmp");
		return Builder.CreateUIToFP(L, MakeFPType, "booltmp");
	}
	else if (op == "==")
	{
		L = Builder.CreateFCmpUEQ(L, R, "cmptmp");
		return Builder.CreateUIToFP(L, MakeFPType, "booltmp");
	}
	else if (op == ">=")
	{
		L = Builder.CreateFCmpUGE(L, R, "cmptmp");
		return Builder.CreateUIToFP(L, MakeFPType, "booltmp");
	}
	else if (op == "<=")
	{
		L = Builder.CreateFCmpULE(L, R, "cmptmp");
		return Builder.CreateUIToFP(L, MakeFPType, "booltmp");
	}
	else if (op == "!=")
	{
		L = Builder.CreateFCmpUNE(L, R, "cmptmp");
		return Builder.CreateUIToFP(L, MakeFPType, "booltmp");
	}
	else
		return LogError("invalid binary operator", line);
}

llvm::Value *Call::codegen()
{
	llvm::Function *func = TheModule->getFunction(callee);
	if (!func)
		return LogError(string("Unknown function: ") + callee, line);

	if (func->arg_size() != args.size())
		return LogError("Incorrect # arguments passed", line);

	vector<llvm::Value *> argsv;
	for (unsigned i = 0; i < args.size(); i++)
	{
		if (auto t = args[i]->codegen())
		{
			llvm::Value *c_arg = func->arg_begin() + i;
			if (c_arg->getType() != t->getType())
			{
				if (c_arg->getType()->isIntegerTy() && t->getType()->isDoubleTy())
					t = Builder.CreateFPToSI(t, Makei32Type, "casttmp");
				else if (c_arg->getType()->isDoubleTy() && t->getType()->isIntegerTy())
					t = Builder.CreateSIToFP(t, MakeFPType, "casttmp");
				else
					LogError(string("Invalid Type for Parameter ") +
								 to_string(i + 1) +
								 string(" of call to ") +
								 callee,
							 line);
			}
			argsv.push_back(t);
		}
		else
			return nullptr;
	}

	return Builder.CreateCall(func, argsv, "calltmp");
};

llvm::Function *Prototype::codegen()
{
	vector<llvm::Type *> Doubles({});
	for (int i = 0; i < args.size(); i++)
	{
		if (args.at(i).second == Return::decimal)
			Doubles.push_back(MakeFPType);
		else if (args.at(i).second == Return::integer)
			Doubles.push_back(Makei32Type);
		else
			return LogErrorP("Not a valid type for the argument in the prototype", line);
	}
	llvm::FunctionType *t;
	if (ret_type == Return::integer)
		t = llvm::FunctionType::get(Makei32Type, Doubles, false);
	else if (ret_type == Return::decimal)
		t = llvm::FunctionType::get(MakeFPType, Doubles, false);
	llvm::Function *f = llvm::Function::Create(t,
											   llvm::Function::ExternalLinkage,
											   name,
											   TheModule.get());

	unsigned i = 0;
	for (auto &arg : f->args())
		arg.setName(args[i++].first);

	return f;
};

llvm::Type *get_type(Return r)
{
	if (r == Return::integer)
		return Makei32Type;
	if (r == Return::decimal)
		return MakeFPType;
	return nullptr;
}

llvm::Function *Function::codegen()
{
	llvm::Function *f = TheModule->getFunction(prototype->getName());

	if (!f)
		f = prototype->codegen();

	if (!f)
		return nullptr;

	llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", f);
	Builder.SetInsertPoint(BB);

	NamedValues.clear();
	for (auto &arg : f->args())
	{
		AllocaInst *Alloca = CreateEntryAlloca(f, arg.getName(), arg.getType());
		Builder.CreateStore(&arg, Alloca);
		NamedValues[arg.getName()] = Alloca;
	}

	for (int i = 0; i < body.size(); i++)
	{
		cast_codegen(body.at(i), f->getReturnType());
		if (!body.at(i)->type.compare("give") && i != body.size() - 1)
		{
			LogError("Skipping everything after the return statment", line);
			break;
		}
	}

	llvm::verifyFunction(*f);
	if (!f)
	{
		f->eraseFromParent();
		return nullptr;
	}
	return f;
}

llvm::Value *Char::codegen()
{
	llvm::Value *chr = llvm::ConstantInt::get(TheContext, llvm::APInt(32, int(val)));
	return chr;
}

llvm::Value *If::codegen(Type *cast_to)
{
	llvm::Value *condv = Cond->codegen();
	if (!condv)
		return nullptr;

	condv = Builder.CreateFCmpONE(condv,
								  llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0)),
								  "ifcond");

	llvm::Function *function = Builder.GetInsertBlock()->getParent();
	llvm::BasicBlock *thenbb = llvm::BasicBlock::Create(TheContext, "then", function);
	llvm::BasicBlock *elsebb = llvm::BasicBlock::Create(TheContext, "else");
	llvm::BasicBlock *mergebb = llvm::BasicBlock::Create(TheContext, "ifcont");

	Builder.CreateCondBr(condv, thenbb, elsebb);
	Builder.SetInsertPoint(thenbb);

	bool has_doubles = false;
	bool has_ints = false;

	vector<llvm::Value *> thenv({});
	for (int i = 0; i < Then.size(); i++)
	{
		thenv.push_back(cast_codegen(Then.at(i), cast_to));
		if (!thenv.back())
			return nullptr;
		if (thenv.back()->getType()->isDoubleTy())
			has_doubles = true;
		if (thenv.back()->getType()->isIntegerTy())
			has_ints = true;
	}

	Builder.CreateBr(mergebb);
	thenbb = Builder.GetInsertBlock();

	function->getBasicBlockList().push_back(elsebb);
	Builder.SetInsertPoint(elsebb);

	vector<llvm::Value *> elsev({});
	for (int i = 0; i < Else.size(); i++)
	{
		elsev.push_back(cast_codegen(Else.at(i), cast_to));
		if (!elsev.back())
			return nullptr;
		if (thenv.back()->getType()->isDoubleTy())
			has_doubles = true;
		if (thenv.back()->getType()->isIntegerTy())
			has_ints = true;
	}

	Builder.CreateBr(mergebb);
	elsebb = Builder.GetInsertBlock();

	function->getBasicBlockList().push_back(mergebb);
	Builder.SetInsertPoint(mergebb);
	// llvm::PHINode *pnd;
	// llvm::PHINode *pni;
	// if (has_doubles)
	// {
	// 	pnd = Builder.CreatePHI(MakeFPType, 0, "iftmp");
	// 	for (llvm::Value *thenve : thenv)
	// 		if (thenve->getType()->isDoubleTy())
	// 			pnd->addIncoming(thenve, thenbb);

	// 	for (llvm::Value *elseve : elsev)
	// 		if (elseve->getType()->isDoubleTy())
	// 			pnd->addIncoming(elseve, elsebb);
	// }
	// if (has_ints)
	// {
	// 	pni = Builder.CreatePHI(Makei32Type, 0, "iftmp");
	// 	for (llvm::Value *thenve : thenv)
	// 		if (thenve->getType()->isIntegerTy())
	// 			pni->addIncoming(thenve, thenbb);

	// 	for (llvm::Value *elseve : elsev)
	// 		if (elseve->getType()->isIntegerTy())
	// 			pni->addIncoming(elseve, elsebb);
	// }
	// if (cast_to->isDoubleTy())
	// 	return pnd;
	// return pni;
}

llvm::Value *If::codegen()
{
	return codegen(Makei32Type);
}

llvm::Value *From::codegen(Type *cast_to)
{
	llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
	llvm::BasicBlock *PreheaderBB = Builder.GetInsertBlock();
	llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

	llvm::AllocaInst *Alloca = CreateEntryAlloca(TheFunction, idname, MakeFPType);

	llvm::Value *StartVal = Start->codegen();
	if (!StartVal)
		return nullptr;

	Builder.CreateStore(StartVal, Alloca, MakeFPType);

	Builder.CreateBr(LoopBB);
	Builder.SetInsertPoint(LoopBB);

	// llvm::PHINode *Variable = Builder.CreatePHI(MakeFPType, 2, idname);
	// Variable->addIncoming(StartVal, PreheaderBB);

	llvm::AllocaInst *OldVal = NamedValues[idname];
	NamedValues[idname] = Alloca;

	for (int i = 0; i < Body.size(); i++)
		if (!cast_codegen(Body.at(i), cast_to))
			return nullptr;

	llvm::Value *StepVal = nullptr;
	if (Step)
	{
		StepVal = Step->codegen();
		if (!StepVal)
			return nullptr;
	}
	else
		StepVal = llvm::ConstantFP::get(TheContext, llvm::APFloat(1.0));

	llvm::Value *CurVar = Builder.CreateLoad(Alloca, idname.c_str());
	llvm::Value *NextVar = Builder.CreateFAdd(CurVar, StepVal, "nextvar");
	Builder.CreateStore(NextVar, Alloca);

	llvm::Value *EndCond = End->codegen();
	if (!EndCond)
		return LogError("End condition not compiled", line);

	// cout << "End cond start" << endl;
	auto EndCondC = Builder.CreateFCmpONE(NextVar, EndCond, "loopcond");
	// cout << "End cond end" << endl;

	llvm::BasicBlock *LoopEndBB = Builder.GetInsertBlock();
	llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(TheContext, "afterloop", TheFunction);

	Builder.CreateCondBr(EndCondC, LoopBB, AfterBB);
	Builder.SetInsertPoint(AfterBB);

	// Variable->addIncoming(NextVar, LoopEndBB);

	if (OldVal)
		NamedValues[idname] = OldVal;
	else
		NamedValues.erase(idname);

	return llvm::Constant::getNullValue(MakeFPType);
}
llvm::Value *From::codegen()
{
	return codegen(Makei32Type);
}

llvm::Value *Give::codegen()
{
	return codegen(Makei32Type);
}
llvm::Value *Give::codegen(Type *cast_to)
{
	return Builder.CreateRet(type_cast(give->codegen(), cast_to));
}

llvm::Value *VarInit::codegen()
{
	AllocaInst *OldBinding;

	llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

	const std::string &VarName = dynamic_cast<Variable *>(dynamic_cast<Binary *>(inst.get())->LHS.get())->name;

	AllocaInst *Alloca = CreateEntryAlloca(TheFunction, VarName, getType(r_type));
	NamedValues[VarName] = Alloca;
	Builder.CreateStore(type_cast(inst->codegen(), getType(r_type)), Alloca);

	return Alloca;
}
} // namespace dia