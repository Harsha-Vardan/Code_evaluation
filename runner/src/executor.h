#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>
#include <vector>

struct ExecutionResult {
    std::string verdict; // AC, WA, RE, TLE
    double timeTaken;    // in seconds
    std::string message; // Error details or output mismatch
};

class Executor {
public:
    static ExecutionResult Execute(const std::string& executable, 
                                 const std::string& inputFile, 
                                 const std::string& expectedOutputFile,
                                 int timeLimitMs);
};

#endif
