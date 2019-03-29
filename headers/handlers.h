#pragma once

namespace dia
{
void Parser::handle_top_level(llvm::Function *f = nullptr, llvm::BasicBlock *BB = nullptr)
{
    if (f && BB)
    {
        Builder.SetInsertPoint(BB);

        NamedValues.clear();
        for (auto &arg : f->args())
            NamedValues[arg.getName()] = &arg;

        auto expr = parseExpr();
        // std::cout << "Start codegen" << std::endl;
        expr->codegen();
        // std::cout << "End codegen" << std::endl;
        return;
    }
    auto fn = parseTopLevel();
    if (fn)
    {
        auto *ir = fn->codegen();
        if (ir)
        {
            // std::cout << "Read top-level expression:" << std::endl;
            // ir->print(llvm::outs());
            // std::cout << std::endl;
        }
        else
        {
            std::cout << "definition error" << std::endl;
        }
    }
    else
    {
        advance();
    }
    return;
};

void Parser::handle_def()
{
    auto fn = parseDef();
    if (fn)
    {
        auto *ir = fn->codegen();
        if (ir)
        {
            // std::cout << "Read function definition:";
            // ir->print(llvm::outs());
            // std::cout << std::endl;
        }
        else
        {
            std::cout << "definition error" << std::endl;
        }
    }
    else
    {
        advance();
    }
    return;
};

void Parser::handle_extern()
{
    advance();
    auto fn = parsePrototype(true);
    if (fn)
    {
        auto *ir = fn->codegen();
        if (ir)
        {
            // std::cout << "Read extern definition:";
            // ir->print(llvm::outs());
            // std::cout << std::endl;
        }
        else
            std::cout << "no extern" << std::endl;
    }
    else
    {
        advance();
    }
    return;
};
} // namespace dia