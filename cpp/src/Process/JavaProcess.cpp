#include "JavaProcess.h"
#include "../Utils/Logger.h"
#include "../Core/FDAApplication.h"
#include <vector>

JavaProcess::JavaProcess()
{
    javaProcess = NULL;
}

JavaProcess::~JavaProcess()
{
    stop();
}

bool JavaProcess::start()
{
    if (isRunning())
    {
        Logger::info("[JAVA] Already running");
        return true;
    }

    wchar_t dllPath[MAX_PATH] = { 0 };
    GetModuleFileNameW(FDAApplication::getModuleHandle(), dllPath, MAX_PATH);

    std::wstring basePath(dllPath);
    size_t slash = basePath.find_last_of(L"\\");

    if (slash == std::wstring::npos)
    {
        Logger::error("[JAVA] Invalid DLL path");
        return false;
    }

    basePath = basePath.substr(0, slash + 1);
    std::wstring jarPath = basePath + L"finacle-dev-assist.jar";

    if (GetFileAttributesW(jarPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        Logger::error("[JAVA] Jar not found");
        MessageBoxW(
            NULL,
            L"Finacle Dev Assist JAR not found",
            L"FinacleDevAssist",
            MB_OK | MB_ICONERROR
        );
        return false;
    }

    Logger::info("[JAVA] Starting process");

    std::wstring command = L"java -jar \"" + jarPath + L"\"";
    std::vector<wchar_t> cmdLine(command.begin(), command.end());
    cmdLine.push_back(0);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    BOOL result = CreateProcessW(
        NULL,
        cmdLine.data(),
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        basePath.c_str(),
        &si,
        &pi
    );

    if (!result)
    {
        Logger::error("[JAVA] CreateProcess failed");
        return false;
    }

    CloseHandle(pi.hThread);
    javaProcess = pi.hProcess;

    Logger::info("[JAVA] Process started");
    return true;
}

bool JavaProcess::isRunning()
{
    if (javaProcess == NULL)
        return false;

    DWORD exitCode;
    if (GetExitCodeProcess(javaProcess, &exitCode))
    {
        return exitCode == STILL_ACTIVE;
    }

    return false;
}

void JavaProcess::stop()
{
    if (javaProcess == NULL)
        return;

    if (isRunning())
    {
        TerminateProcess(javaProcess, 0);
        Logger::info("[JAVA] Process stopped");
    }

    CloseHandle(javaProcess);
    javaProcess = NULL;
}