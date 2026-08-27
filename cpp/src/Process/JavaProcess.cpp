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

static std::string ws2s(const std::wstring& value)
{
    if (value.empty())
        return "";

    int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        value.c_str(),
        static_cast<int>(value.length()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (size == 0) return "";

    std::string result(size, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        value.c_str(),
        static_cast<int>(value.length()),
        &result[0],
        size,
        nullptr,
        nullptr
    );

    return result;
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
    std::wstring javaPath = basePath + L"jre-17\\bin\\java.exe";

    Logger::info("[JAVA] Runtime: " + ws2s(javaPath));
    Logger::info("[JAVA] Jar: " + ws2s(jarPath));

    if (GetFileAttributesW(javaPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        Logger::error("[JAVA] Runtime not found");

        MessageBoxW(
            NULL,
            L"Java Runtime not found",
            L"FinacleDevAssist",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

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

    std::wstring command = L"\"" + javaPath + L"\" "
        L"--add-exports=jdk.compiler/com.sun.tools.javac.api=ALL-UNNAMED "
        L"--add-exports=jdk.compiler/com.sun.tools.javac.file=ALL-UNNAMED "
        L"--add-exports=jdk.compiler/com.sun.tools.javac.parser=ALL-UNNAMED "
        L"--add-exports=jdk.compiler/com.sun.tools.javac.tree=ALL-UNNAMED "
        L"--add-exports=jdk.compiler/com.sun.tools.javac.util=ALL-UNNAMED "
        L"--add-opens=jdk.compiler/com.sun.tools.javac.code=ALL-UNNAMED "
        L"--add-opens=jdk.compiler/com.sun.tools.javac.comp=ALL-UNNAMED "
        L"-jar \"" + jarPath + L"\"";

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