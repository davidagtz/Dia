#pragma once

#include <memory>
#include <string>
#include "Expr.h"
#include "ReturnTypes.h"

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::map<std::string, llvm::Value *> NamedValues;
static std::unique_ptr<llvm::Module> TheModule;

llvm::Value *cast_codegen(std::unique_ptr<dia::Base> val, llvm::Type *cast_to)
{
	if (val->type.compare("if") == 0 ||
		val->type.compare("give") == 0)
		return val->codegen(cast_to);
	else
		return val->codegen();
}

llvm::Value *LogError(std::string msg, unsigned long long line)
{
	std::cout << "Line: " << line << std::endl;
	std::cout << msg << std::endl;
	return nullptr;
}

llvm::Function *LogErrorP(std::string msg, unsigned long long line)
{
	std::cout << "Line: " << line << std::endl;
	std::cout << msg << std::endl;
	return nullptr;
}

llvm::Value *dia::Base::type_cast(llvm::Value *from, llvm::Type *to, std::string msg = "")
{
	// std::cout << "Start" << std::endl;
	if (from->getType()->isIntegerTy() ^ to->isIntegerTy())
	{
		if (from->getType()->isIntegerTy())
			from = Builder.CreateSIToFP(from, llvm::Type::getDoubleTy(TheContext), "casttmp");
		else if (from->getType()->isDoubleTy())
			from = Builder.CreateFPToSI(from, llvm::Type::getInt32Ty(TheContext), "casttmp");
		else
			return LogError(std::string("Invalid Type ") + msg, line);
	}
	// std::cout << "Success" << std::endl;
	return from;
}

llvm::Value *dia::Number::codegen()
{
	return llvm::ConstantFP::get(TheContext, llvm::APFloat(val));
};

llvm::Value *dia::Variable::codegen()
{
	llvm::Value *V = NamedValues[name];
	if (V)
		return V;
	return LogError("Unknown Var Name", line);
};

llvm::Value *dia::Binary::codegen()
{
	llvm::Value *L = LHS->codegen();
	llvm::Value *R = RHS->codegen();
	if (!L || !R)
		return nullptr;

	// Type cast to double if they are ints
	if (L->getType()->isIntegerTy())
		L = Builder.CreateSIToFP(L, llvm::Type::getDoubleTy(TheContext), "casttmp");
	if (R->getType()->isIntegerTy())
		R = Builder.CreateSIToFP(R, llvm::Type::getDoubleTy(TheContext), "casttmp");

	// Switch for greater than to less than
	if (op == '>')
	{
		llvm::Value *t = L;
		L = R;
		R = t;
		op = '<';
	}

	switch (op)
	{
	case '+':
		return Builder.CreateFAdd(L, R, "addtmp");
	case '-':
		return Builder.CreateFSub(L, R, "subtmp");
	case '*':
		return Builder.CreateFMul(L, R, "multmp");
	case '/':
		return Builder.CreateFDiv(L, R, "fdivtmp");
	case '<':
		L = Builder.CreateFCmpULT(L, R, "cmptmp");
		return Builder.CreateUIToFP(L, llvm::Type::getDoubleTy(TheContext),
									"booltmp");
	default:
		return LogError("invalid binary operator", line);
	}
};

llvm::Value *dia::Call::codegen()
{
	llvm::Function *func = TheModule->getFunction(callee);
	if (!func)
		return LogError(std::string("Unknown function: ") + callee, line);

	if (func->arg_size() != args.size())
		return LogError("Incorrect # arguments passed", line);

	std::vector<llvm::Value *> argsv;
	for (unsigned i = 0; i < args.size(); i++)
	{
		if (auto t = args[i]->codegen())
		{
			llvm::Value *c_arg = func->arg_begin() + i;
			if (c_arg->getType() != t->getType())
			{
				if (c_arg->getType()->isIntegerTy() && t->getType()->isDoubleTy())
					t = Builder.CreateFPToSI(t, llvm::Type::getInt32Ty(TheContext), "casttmp");
				else if (c_arg->getType()->isDoubleTy() && t->getType()->isIntegerTy())
					t = Builder.CreateSIToFP(t, llvm::Type::getDoubleTy(TheContext), "casttmp");
				else
					LogError(std::string("Invalid Type for Parameter ") +
								 std::to_string(i + 1) +
								 std::string(" of call to ") +
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

llvm::Function *dia::Prototype::codegen()
{
	std::vector<llvm::Type *> Doubles({});
	for (int i = 0; i < args.size(); i++)
	{
		if (args.at(i).second == Return::decimal)
			Doubles.push_back(llvm::Type::getDoubleTy(TheContext));
		else if (args.at(i).second == Return::integer)
			Doubles.push_back(llvm::Type::getInt32Ty(TheContext));
		else
			return LogErrorP("Not a valid type for the argument in the prototype", line);
	}
	llvm::FunctionType *t;
	if (ret_type == Return::integer)
		t = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), Doubles, false);
	else if (ret_type == Return::decimal)
		t = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext), Doubles, false);
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
		return llvm::Type::getInt32Ty(TheContext);
	if (r == Return::decimal)
		return llvm::Type::getDoubleTy(TheContext);
	return nullptr;
	// if(r == Return::integer)
	// 	return llvm::Type::getInt32Ty(TheContext);
}

llvm::Function *dia::Function::codegen()
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
		NamedValues[arg.getName()] = &arg;

	for (int i = 0; i < body.size(); i++)
	{
		bool br = false;
		if (!body.at(i)->type.compare("give"))
			br = true;
		cast_codegen(std::move(body.at(i)), f->getReturnType());
		if (br)
		{
			if (i != body.size() - 1)
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

llvm::Value *dia::Char::codegen()
{
	llvm::Value *chr = llvm::ConstantInt::get(TheContext, llvm::APInt(32, int(val)));
	return chr;
}

llvm::Value *dia::If::codegen(llvm::Type *cast_to)
{
	llvm::Value *condv = Cond->codegen();
	if (!condv)
		return nullptr;

	condv = Builder.CreateFCmpONE(condv,
								  llvm::ConstantFP::get(TheContext,
														llvm::APFloat(0.0)),
								  "ifcond");

	llvm::Function *function = Builder.GetInsertBlock()->getParent();
	llvm::BasicBlock *thenbb = llvm::BasicBlock::Create(TheContext, "then", function);
	llvm::BasicBlock *elsebb = llvm::BasicBlock::Create(TheContext, "else");
	llvm::BasicBlock *mergebb = llvm::BasicBlock::Create(TheContext, "ifcont");

	Builder.CreateCondBr(condv, thenbb, elsebb);
	Builder.SetInsertPoint(thenbb);

	bool has_doubles = false;
	bool has_ints = false;

	std::vector<llvm::Value *> thenv({});
	for (int i = 0; i < Then.size(); i++)
	{
		thenv.push_back(cast_codegen(std::move(Then.at(i)), cast_to));
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

	std::vector<llvm::Value *> elsev({});
	for (int i = 0; i < Else.size(); i++)
	{
		elsev.push_back(cast_codegen(std::move(Else.at(i)), cast_to));
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
	// 	pnd = Builder.CreatePHI(llvm::Type::getDoubleTy(TheContext), 0, "iftmp");
	// 	for (llvm::Value *thenve : thenv)
	// 		if (thenve->getType()->isDoubleTy())
	// 			pnd->addIncoming(thenve, thenbb);

	// 	for (llvm::Value *elseve : elsev)
	// 		if (elseve->getType()->isDoubleTy())
	// 			pnd->addIncoming(elseve, elsebb);
	// }
	// if (has_ints)
	// {
	// 	pni = Builder.CreatePHI(llvm::Type::getInt32Ty(TheContext), 0, "iftmp");
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

llvm::Value *dia::If::codegen()
{
	return codegen(llvm::Type::getInt32Ty(TheContext));
}

llvm::Value *dia::From::codegen()
{
	llvm::Value *StartVal = Start->codegen();
	if (!StartVal)
		return nullptr;

	llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
	llvm::BasicBlock *PreheaderBB = Builder.GetInsertBlock();
	llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(TheContext, "loop", TheFunction);

	Builder.CreateBr(LoopBB);
	Builder.SetInsertPoint(LoopBB);

	llvm::PHINode *Variable =
		Builder.CreatePHI(llvm::Type::getDoubleTy(TheContext), 2, idname);
	Variable->addIncoming(StartVal, PreheaderBB);

	llvm::Value *OldVal = NamedValues[idname];
	NamedValues[idname] = Variable;

	for (int i = 0; i < Body.size(); i++)
		if (!Body.at(i)->codegen())
			return nullptr;

	llvm::Value *StepVal = nullptr;
	if (Step)
	{
		StepVal = Step->codegen();
		if (!StepVal)
			return nullptr;
	}
	else
	{
		StepVal = llvm::ConstantFP::get(TheContext, llvm::APFloat(1.0));
	}

	llvm::Value *NextVar = Builder.CreateFAdd(Variable, StepVal, "nextvar");

	llvm::Value *EndCond = End->codegen();
	std::cout << End->type << " " << End->val << std::endl;
	if (!EndCond)
		return LogError("End condition not compiled", line);

	std::cout << "End cond start" << std::endl;
	auto EndCondC = Builder.CreateFCmpONE(
		NextVar, EndCond, "loopcond");
	std::cout << "End cond end" << std::endl;

	llvm::BasicBlock *LoopEndBB = Builder.GetInsertBlock();
	llvm::BasicBlock *AfterBB =
		llvm::BasicBlock::Create(TheContext, "afterloop", TheFunction);

	Builder.CreateCondBr(EndCondC, LoopBB, AfterBB);
	Builder.SetInsertPoint(AfterBB);

	Variable->addIncoming(NextVar, LoopEndBB);

	if (OldVal)
		NamedValues[idname] = OldVal;
	else
		NamedValues.erase(idname);

	return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(TheContext));
}

llvm::Value *dia::Give::codegen()
{
	return codegen(llvm::Type::getInt32Ty(TheContext));
}
llvm::Value *dia::Give::codegen(llvm::Type *cast_to)
{
	return Builder.CreateRet(type_cast(give->codegen(), cast_to));
}