#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#include <thread>
#include <atomic>
#include <memory>
#include "TaskQueue.h"

class JavaProcess;
class FDAClient;
class TokenHandler;
class ValidationHandler;
class CodeBeautifierHandler;

class Worker
{
public:
    Worker();
    ~Worker();

    void start();
    void stop();
    void submit(Task task);

private:
    void run();

private:
    std::atomic<bool> running;
    std::thread workerThread;
    TaskQueue taskQueue;
    std::unique_ptr<JavaProcess> javaProcess;
    std::unique_ptr<FDAClient> client;
    std::unique_ptr<TokenHandler> tokenHandler;
    std::unique_ptr<ValidationHandler> validationHandler;
    std::unique_ptr<CodeBeautifierHandler> codeBeautifierHandler;
};