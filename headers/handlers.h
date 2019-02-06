void Parser::handle_top_level()
{

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
            ir->print(llvm::errs());
            std::cout << std::endl;
        }
    }
    else
    {
        advance();
    }
    return;
};