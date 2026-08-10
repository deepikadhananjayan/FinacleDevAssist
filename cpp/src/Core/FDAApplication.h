#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

#include <Windows.h>
#include <string>
#include "FDAApplicationState.h"

class Worker;

class FDAApplication
{
public:
    static void initialize(HANDLE hModule);
    static void shutdown();
    static void handleAutoComplete(char ch);
    static void handleValidateScript();
    static std::string selectBeautifyContentType();
    static void handleBeautifyCode();
    static void handleGenerateMenuSource();
    static void handleFIExecution();
    static std::string getPluginVersion();
    static void aboutPlugin();
    static void updateState(FDAApplicationState currState);
    static FDAApplicationState getState();
    static HMODULE getModuleHandle();

private:
    static FDAApplicationState state;
    static Worker* worker;
    static HMODULE moduleHandle;
};