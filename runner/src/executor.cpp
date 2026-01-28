#include "executor.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdio>
#include <thread>
#include <future>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

// Helper to compare files
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1), f2(file2);
    if (!f1.is_open() || !f2.is_open()) return false;
    
    std::string s1, s2;
    while (f1 >> s1 && f2 >> s2) {
        if (s1 != s2) return false;
    }
    return !(f1 >> s1 || f2 >> s2);
}

ExecutionResult Executor::Execute(const std::string& executable, 
                                 const std::string& inputFile, 
                                 const std::string& expectedOutputFile,
                                 int timeLimitMs) {
    
    std::string actualOutputFile = "temp_output.txt";
    auto start = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
    // Windows Implementation using CreateProcess
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    
    // Setup redirection
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hIn = CreateFileA(inputFile.c_str(), GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hOut = CreateFileA(actualOutputFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE) {
        if (hIn != INVALID_HANDLE_VALUE) CloseHandle(hIn);
        if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);
        return {"RE", 0, "Failed to open input/output files"};
    }

    si.hStdInput = hIn;
    si.hStdOutput = hOut;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    char cmd[1024];
    strncpy(cmd, executable.c_str(), sizeof(cmd));

    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hIn);
        CloseHandle(hOut);
        return {"RE", 0, "Failed to create process"};
    }

    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeLimitMs);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    double timeTaken = elapsed.count();

    if (waitResult == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hIn);
        CloseHandle(hOut);
        return {"TLE", timeTaken, "Time Limit Exceeded"};
    }

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hIn);
    CloseHandle(hOut);

    if (exitCode != 0) {
        return {"RE", timeTaken, "Runtime Error (Exit Code: " + std::to_string(exitCode) + ")"};
    }

#else
    // Linux Implementation using fork/exec and setrlimit
    pid_t pid = fork();

    if (pid == -1) {
        return {"RE", 0, "Failed to fork process"};
    }

    if (pid == 0) {
        // Child Process
        
        // Redirect stdin
        int inFd = open(inputFile.c_str(), O_RDONLY);
        if (inFd == -1) exit(1);
        dup2(inFd, STDIN_FILENO);
        close(inFd);

        // Redirect stdout
        int outFd = open(actualOutputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outFd == -1) exit(1);
        dup2(outFd, STDOUT_FILENO);
        close(outFd);

        // Set memory limit (example: 128MB)
        struct rlimit memLimit;
        memLimit.rlim_cur = 128 * 1024 * 1024;
        memLimit.rlim_max = 128 * 1024 * 1024;
        setrlimit(RLIMIT_AS, &memLimit);

        // Execute user program
        char* args[] = {(char*)executable.c_str(), NULL};
        execv(args[0], args);
        exit(1); // Should not reach here
    } else {
        // Parent Process
        int status;
        auto start = std::chrono::high_resolution_clock::now();
        
        // Wait with a watchdog or use waitpid with WNOHANG in a loop
        // Simple approach: sleep and check
        int totalWaitMs = 0;
        bool finished = false;
        while (totalWaitMs < timeLimitMs) {
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                totalWaitMs += 10;
            } else if (result == pid) {
                finished = true;
                break;
            } else {
                return {"RE", 0, "Waitpid error"};
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double timeTaken = elapsed.count();

        if (!finished) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0); // Cleanup zombie
            return {"TLE", timeTaken, "Time Limit Exceeded"};
        }

        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            if (exitCode != 0) {
                return {"RE", timeTaken, "Runtime Error (Exit Code: " + std::to_string(exitCode) + ")"};
            }
        } else {
            return {"RE", timeTaken, "Program terminated abnormally"};
        }
    }
#endif

    if (compareFiles(actualOutputFile, expectedOutputFile)) {
        return {"AC", timeTaken, "Accepted"};
    } else {
        return {"WA", timeTaken, "Wrong Answer"};
    }
}
