g++ -ggdb -std=gnu++17  dia-c.cpp -o dia `llvm-config --system-libs --cppflags --ldflags --libs  mcjit  native core`
