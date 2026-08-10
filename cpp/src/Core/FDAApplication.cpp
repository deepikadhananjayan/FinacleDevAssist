#include "FDAApplication.h"
#include "../Threading/Worker.h"
#include "../Models/Task.h"
#include "../Utils/Logger.h"
#include "../Features/AutoCompleteManager.h"
#include "../Features/ScintillaHelper.h"
#include "../DockingFeature/ValidationPanel.h"
#include "../PluginDefinition.h"
#include "../Models/BeautifyData.h"
#include "../Dialogs/BeautifyLanguageDialog.h"
#include "../Dialogs/FIRequestDialog.h"
#include "../DockingFeature/resource.h"
#include "../Models/FIRequestData.h"

#pragma comment(lib, "Version.lib")

Worker* FDAApplication::worker = nullptr;
HMODULE FDAApplication::moduleHandle = nullptr;
FDAApplicationState FDAApplication::state = FDAApplicationState::NOT_INITIALIZED;

void FDAApplication::initialize(HANDLE hModule)
{
    if (state != FDAApplicationState::NOT_INITIALIZED)
    {
        MessageBox(
            nppData._nppHandle,
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

    ValidationPanel::hidePanel();

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
            nppData._nppHandle,
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
            nppData._nppHandle,
            TEXT("Please save the file before validating script."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    if (!ScintillaHelper::isValidFile(filePath))
    {
        MessageBox(
            nppData._nppHandle,
            TEXT("Only Finacle script files (.scr) can be validated."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    worker->submit({ TaskType::VALIDATE_SCRIPT, filePath });
}

std::string FDAApplication::selectBeautifyContentType()
{
    INT_PTR result =
        DialogBox(
            moduleHandle,
            MAKEINTRESOURCE(IDD_BEAUTIFY_LANGUAGE_DIALOG),
            nppData._nppHandle,
            BeautifyLanguageDialogProc
        );

    switch (result)
    {
    case 1:
        return "JAVA";

    case 2:
        return "JS";

    case 3:
        return "XML";

    case 4:
        return "SCRIPT";

    default:
        return "";
    }
}

void FDAApplication::handleBeautifyCode()
{
    if (state != FDAApplicationState::READY)
    {
        MessageBox(
            nppData._nppHandle,
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
            nppData._nppHandle,
            errorMessage.c_str(),
            "Finacle Dev Assist",
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    HWND editor = ScintillaHelper::getCurrentEditor();

    LRESULT selectionStart = ::SendMessage(editor, SCI_GETSELECTIONSTART, 0, 0);
    LRESULT selectionEnd = ::SendMessage(editor, SCI_GETSELECTIONEND, 0, 0);

    bool hasSelection = (selectionStart != selectionEnd);

    std::string content = ScintillaHelper::getSelectedOrAllText();

    if (content.empty())
    {
        MessageBox(
            nppData._nppHandle,
            TEXT("There is no code to beautify."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    std::string contentType = ScintillaHelper::getContentType(filePath);

    if (contentType.empty())
    {
        contentType = selectBeautifyContentType();

        if (contentType.empty())
        {
            return;
        }
    }

    BeautifyData beautifyData;
    beautifyData.contentType = contentType;
    beautifyData.content = content;
    beautifyData.hasSelection = hasSelection;

    worker->submit({ TaskType::BEAUTIFY_CODE, beautifyData });
}

void FDAApplication::handleGenerateMenuSource()
{
    if (state != FDAApplicationState::READY)
    {
        MessageBox(
            nppData._nppHandle,
            TEXT("FDA Plugin is not initialized."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    MessageBox(
        nppData._nppHandle,
        TEXT("Generate MenuSource Feature is Under Development!."),
        TEXT("Finacle Dev Assist"),
        MB_OK | MB_ICONINFORMATION
    );
}

void FDAApplication::handleFIExecution()
{   
    if (state != FDAApplicationState::READY)
    {
        MessageBox(
            nppData._nppHandle,
            TEXT("FDA Plugin is not initialized."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    FIRequestData fiRequestData;

    INT_PTR result = DialogBoxParam(
        moduleHandle,
        MAKEINTRESOURCE(IDD_FI_REQUEST_DIALOG),
        nppData._nppHandle,
        FIRequestDialogProc,
        reinterpret_cast<LPARAM>(&fiRequestData)
    );

    if (result != IDOK)
        return;

    fiRequestData.body = ScintillaHelper::getSelectedOrAllText();
    
    worker->submit({ TaskType::EXECUTE_FI_REQUEST, fiRequestData });
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
        nppData._nppHandle,
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