#include <memory>
#include <string>
#include "Expr.h"

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *> NamedValues;

llvm::Value *dia::Number::codegen()
{
	return llvm::ConstantFP::get(TheContext, llvm::APFloat(val));
};

llvm::Value *dia::Variable::codegen(){

};

llvm::Value *dia::Binary::codegen(){

};

llvm::Value *dia::Call::codegen(){

};

llvm::Value *dia::Prototype::codegen(){

};

llvm::Value *dia::Function::codegen(){

};