#ifndef COMPILER_H
#define COMPILER_H

#include <string>

struct CompileResult {
    bool success;
    std::string message; // Errors if success is false
};

class Compiler {
public:
    static CompileResult Compile(const std::string& sourceFile, const std::string& outputFile);
};

#endif
