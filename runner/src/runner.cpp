#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "compiler.h"
#include "executor.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: runner <source_file> <problem_id> [time_limit_ms]" << std::endl;
        return 1;
    }

    std::string sourceFile = argv[1];
    std::string problemId = argv[2];
    int timeLimitMs = (argc > 3) ? std::stoi(argv[3]) : 1000;

    #ifdef _WIN32
    std::string executable = ".\\user_program.exe";
    #else
    std::string executable = "./user_program";
    #endif

    // 1. Compile
    CompileResult cr = Compiler::Compile(sourceFile, executable);
    if (!cr.success) {
        std::cout << "{\"verdict\": \"CE\", \"message\": \"" << cr.message << "\"}" << std::endl;
        return 0;
    }

    // 2. Iterate through test cases
    std::string testcaseDir = "../testcases/" + problemId;
    int passed = 0;
    int total = 0;
    double maxTime = 0;

    std::vector<std::string> inputs;
    for (const auto& entry : fs::directory_iterator(testcaseDir)) {
        std::string path = entry.path().string();
        if (path.find("input") != std::string::npos) {
            inputs.push_back(path);
        }
    }
    std::sort(inputs.begin(), inputs.end());

    std::string finalVerdict = "AC";

    for (const auto& inputFile : inputs) {
        total++;
        std::string expectedOutputFile = inputFile;
        // Replace "input" with "output" in the filename
        size_t pos = expectedOutputFile.find("input");
        expectedOutputFile.replace(pos, 5, "output");

        ExecutionResult er = Executor::Execute(executable, inputFile, expectedOutputFile, timeLimitMs);
        
        if (er.timeTaken > maxTime) maxTime = er.timeTaken;

        if (er.verdict != "AC") {
            finalVerdict = er.verdict;
            std::cout << "{\"verdict\": \"" << er.verdict << "\", \"message\": \"" << er.message << "\", \"passed\": " << passed << ", \"total\": " << total << ", \"executionTime\": \"" << er.timeTaken << "s\"}" << std::endl;
            // Cleanup
            fs::remove(executable);
            fs::remove("temp_output.txt");
            return 0;
        }
        passed++;
    }

    // 3. Print Final JSON
    std::cout << "{"
              << "\"verdict\": \"" << finalVerdict << "\", "
              << "\"passed\": " << passed << ", "
              << "\"total\": " << total << ", "
              << "\"executionTime\": \"" << maxTime << "s\""
              << "}" << std::endl;

    // Cleanup
    fs::remove(executable);
    fs::remove("temp_output.txt");

    return 0;
}
