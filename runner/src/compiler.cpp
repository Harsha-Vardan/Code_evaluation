#include "compiler.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

CompileResult Compiler::Compile(const std::string& sourceFile, const std::string& outputFile) {
    // Construct the command: g++ <source> -o <output>
    // We use 2>&1 to capture stderr in the same stream as stdout
    std::string command = "g++ " + sourceFile + " -o " + outputFile + " 2>&1";
    
    std::array<char, 128> buffer;
    std::string result = "";
    
    // Open pipe to capture output
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        return {false, "Failed to start g++ compiler process."};
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    int exitCode = _pclose(pipe);
    
    if (exitCode == 0) {
        return {true, "Compilation Successful"};
    } else {
        return {false, result};
    }
}
