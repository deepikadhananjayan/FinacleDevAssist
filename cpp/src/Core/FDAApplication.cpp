#include "FDAApplication.h"

#include "../Threading/Worker.h"
#include "../Threading/Task.h"
#include "../Utils/Logger.h"
#include "../Editor/AutoCompleteManager.h"

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

HMODULE FDAApplication::getModuleHandle()
{
    return moduleHandle;
}