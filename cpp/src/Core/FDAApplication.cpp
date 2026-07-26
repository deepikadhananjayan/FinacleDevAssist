#include "FDAApplication.h"
#include "../Threading/Worker.h"
#include "../Threading/Task.h"
#include "../Utils/Logger.h"
#include "../Features/AutoCompleteManager.h"
#include "../Features/ScintillaHelper.h"
#include "../DockingFeature/ValidationPanel.h"
#include "../PluginDefinition.h"

#pragma comment(lib, "Version.lib")

Worker* FDAApplication::worker = nullptr;
HMODULE FDAApplication::moduleHandle = nullptr;
FDAApplicationState FDAApplication::state = FDAApplicationState::NOT_INITIALIZED;

void FDAApplication::initialize(HANDLE hModule)
{
    if (state != FDAApplicationState::NOT_INITIALIZED)
    {
        MessageBox(
            NULL,
            TEXT("FDA Plugin is already initialized."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );

        Logger::info("[APPLICATION] Already initialized");
        return;
    }

    FDAApplication::updateState(FDAApplicationState::STARTING);
    moduleHandle = (HMODULE)hModule;

    Logger::initialize();
    Logger::info("[APPLICATION] Initialize requested");

    INITCOMMONCONTROLSEX icex;

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;

    InitCommonControlsEx(&icex);

    Logger::info("[APPLICATION] Initializing Validation Panel");
    ValidationPanel::init(
        (HINSTANCE)hModule,
        nppData._nppHandle
    );

    Logger::info("[APPLICATION] Registering Validation Panel");
    ValidationPanel::registerPanel();

    if (worker != nullptr)
    {
        Logger::warn("[APPLICATION] Already initialized");
        return;
    }

    worker = new Worker();

    worker->start();

    worker->submit({ TaskType::START_JAVA });
}

FDAApplicationState FDAApplication::getState()
{
    return state;
}

void FDAApplication::updateState(FDAApplicationState currState)
{
    state = currState;

    switch (state)
    {
    case FDAApplicationState::NOT_INITIALIZED:
        Logger::info("[APPLICATION STATE] NOT_INITIALIZED");
        break;

    case FDAApplicationState::STARTING:
        Logger::info("[APPLICATION STATE] STARTING");
        break;

    case FDAApplicationState::READY:
        Logger::info("[APPLICATION STATE] READY");
        break;

    case FDAApplicationState::STOPPING:
        Logger::info("[APPLICATION STATE] STOPPING");
        break;

    default:
        Logger::info("[APPLICATION STATE] UNKNOWN");
        break;
    }
}

void FDAApplication::shutdown()
{
    Logger::info("[APPLICATION] Shutdown requested");

    if (state == FDAApplicationState::NOT_INITIALIZED)
    {
        MessageBox(
            NULL,
            TEXT("FDA Plugin is not initialized."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );

        return;
    }

    FDAApplication::updateState(FDAApplicationState::STOPPING);

    if (worker != nullptr)
    {
        worker->stop();

        delete worker;

        worker = nullptr;
    }

    Logger::shutdown();
}

void FDAApplication::handleAutoComplete(char ch)
{
    if (state != FDAApplicationState::READY)
    {
        return;
    }

    AutoCompleteManager::showSuggestions(ch);
}

void FDAApplication::handleValidateScript()
{
    if (state != FDAApplicationState::READY)
    {
        MessageBox(
            NULL,
            TEXT("FDA Plugin is not initialized."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    std::string filePath = ScintillaHelper::getCurrentFilePath();

    if (filePath.rfind("-1|", 0) == 0)
    {
        std::string errorMessage = filePath.substr(3);

        MessageBoxA(
            NULL,
            errorMessage.c_str(),
            "Finacle Dev Assist",
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    LRESULT isFileModified =
        ::SendMessage(
            ScintillaHelper::getCurrentEditor(),
            SCI_GETMODIFY,
            0,
            0
        );

    if (isFileModified)
    {
        MessageBox(
            NULL,
            TEXT("Please save the file before validating script."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    if (!ScintillaHelper::isScriptFile(filePath))
    {
        MessageBox(
            NULL,
            TEXT("Only Finacle script files (.scr) can be validated."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    worker->submit({ TaskType::VALIDATE_SCRIPT, filePath });
}

void FDAApplication::handleFormatScript()
{
    MessageBox(
        NULL,
        TEXT("Format Script Feature is Under Development!."),
        TEXT("Finacle Dev Assist"),
        MB_OK | MB_ICONINFORMATION
    );
}

void FDAApplication::aboutPlugin()
{
    std::string version = getPluginVersion();

    std::string about =
        "Finacle Dev Assist\n\n"
        "Version : " + version +
        "\n\n"
        "A developer assistant tool for Finacle scripting."
        "\n\n"
        "Currently under active development."
        "\n\n"
        "Developed by Sandy <3";

    MessageBoxA(
        NULL,
        about.c_str(),
        "About Finacle Dev Assist",
        MB_OK | MB_ICONINFORMATION
    );
}

std::string FDAApplication::getPluginVersion()
{
    HMODULE hModule = NULL;

    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCSTR)&FDAApplication::aboutPlugin,
        &hModule
    );

    char path[MAX_PATH];

    GetModuleFileNameA(
        hModule,
        path,
        MAX_PATH
    );

    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeA(path, &handle);

    if (size == 0)
        return "Unknown";

    std::vector<char> buffer(size);

    if (!GetFileVersionInfoA(path, 0, size, buffer.data()))
        return "Unknown";

    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT length = 0;

    VerQueryValueA(buffer.data(), "\\", (LPVOID*)&fileInfo, &length);

    if (fileInfo)
    {
        int major = HIWORD(fileInfo->dwFileVersionMS);
        int minor = LOWORD(fileInfo->dwFileVersionMS);
        int build = HIWORD(fileInfo->dwFileVersionLS);

        return std::to_string(major) + "." +
            std::to_string(minor) + "." +
            std::to_string(build);
    }

    return "Unknown";
}

HMODULE FDAApplication::getModuleHandle()
{
    return moduleHandle;
}