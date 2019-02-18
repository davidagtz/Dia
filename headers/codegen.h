#include <memory>
#include <string>
#include "Expr.h"
#include "ReturnTypes.h"

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::map<std::string, llvm::Value *> NamedValues;
static std::unique_ptr<llvm::Module> TheModule;
// static std::unique_ptr<KaleidoscopeJIT> TheJIT;

llvm::Value *LogError(std::string msg)
{
	std::cout << msg << std::endl;
	return nullptr;
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
	return LogError("Unknown Var Name");
};

llvm::Value *dia::Binary::codegen()
{
	llvm::Value *L = LHS->codegen();
	llvm::Value *R = RHS->codegen();
	if (!L || !R)
		return nullptr;

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
		return LogError("invalid binary operator");
	}
};

llvm::Value *dia::Call::codegen()
{
	llvm::Function *func = TheModule->getFunction(callee);
	if (!func)
		return LogError("Unknown function.");

	if (func->arg_size() != args.size())
		return LogError("Incorrect # arguments passed");

	std::vector<llvm::Value *> argsv;
	for (unsigned i = 0; i < args.size(); i++)
	{
		if (auto t = args[i]->codegen())
			argsv.push_back(t);
		else
			return nullptr;
	}

	return Builder.CreateCall(func, argsv, "calltmp");
};

llvm::Function *dia::Prototype::codegen()
{
	std::vector<llvm::Type *> Doubles(args.size(), llvm::Type::getDoubleTy(TheContext));
	llvm::FunctionType *t;
	if (ret_type == Return::integer)
		t = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), Doubles, false);
	else if (ret_type == Return::decimal)
		t = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext), Doubles, false);
	llvm::Function *f = llvm::Function::Create(t, llvm::Function::ExternalLinkage, name, TheModule.get());

	unsigned i = 0;
	for (auto &arg : f->args())
		arg.setName(args[i++]);

	return f;
};

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

	if (llvm::Value *ret = body->codegen())
	{
		Builder.CreateRet(ret);
		llvm::verifyFunction(*f);
		return f;
	}

	f->eraseFromParent();
	return nullptr;
}
