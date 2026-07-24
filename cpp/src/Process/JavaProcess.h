#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#include <string>

class JavaProcess
{
public:
    JavaProcess();
    ~JavaProcess();

    bool start();
    void stop();
    bool isRunning();

private:
    HANDLE javaProcess;
};