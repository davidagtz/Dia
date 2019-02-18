void Parser::handle_top_level()
{
    auto fn = parseTopLevel();
    if (fn)
    {
        auto *ir = fn->codegen();
        if (ir)
        {
            std::cout << "Read top-level expression:" << std::endl;
            ir->print(llvm::outs());
            std::cout << std::endl;
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
            std::cout << "Read function definition:";
            ir->print(llvm::outs());
            std::cout << std::endl;
        }
    }
    else
    {
        advance();
    }
    return;
};