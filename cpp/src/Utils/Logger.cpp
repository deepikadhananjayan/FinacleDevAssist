#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include "Logger.h"
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <chrono>

std::ofstream Logger::logFile;
std::mutex Logger::logMutex;
bool Logger::initialized = false;

bool Logger::initialize()
{
    std::lock_guard<std::mutex> lock(logMutex);

    if (initialized)
        return true;

    char userProfile[MAX_PATH] = { 0 };

    if (GetEnvironmentVariableA("USERPROFILE", userProfile, MAX_PATH) == 0)
    {
        return false;
    }

    std::string fdaFolder = std::string(userProfile) + "\\FDA";
    std::string logsFolder = fdaFolder + "\\logs";
    std::string logPath = logsFolder + "\\fda-cpp.log";

    DWORD attr = GetFileAttributesA(fdaFolder.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        CreateDirectoryA(fdaFolder.c_str(), NULL);
        SetFileAttributesA(fdaFolder.c_str(), FILE_ATTRIBUTE_HIDDEN);
    }

    attr = GetFileAttributesA(logsFolder.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        CreateDirectoryA(logsFolder.c_str(), NULL);
    }

    logFile.open(logPath, std::ios::out | std::ios::app);

    if (!logFile.is_open())
    {
        return false;
    }

    initialized = true;

    logFile << "==================================================\n"
        << getCurrentTimestamp()
        << " [INFO ] FDA Logger Initialized\n"
        << "==================================================\n";
    logFile.flush();

    return true;
}

void Logger::shutdown()
{
    std::lock_guard<std::mutex> lock(logMutex);

    if (!initialized)
        return;

    logFile << "===============================================\n"
        << getCurrentTimestamp()
        << " [INFO ] FDA Logger Shutdown\n"
        << "===============================================\n";
    logFile.flush();
    logFile.close();
    initialized = false;
}

void Logger::info(const std::string& message)
{
    write("INFO ", message);
}

void Logger::debug(const std::string& message)
{
    write("DEBUG", message);
}

void Logger::warn(const std::string& message)
{
    write("WARN ", message);
}

void Logger::error(const std::string& message)
{
    write("ERROR", message);
}

void Logger::write(const std::string& level, const std::string& message)
{
    if (!initialized)
        return;

    std::lock_guard<std::mutex> lock(logMutex);

    logFile << getCurrentTimestamp()
        << " [" << level << "] "
        << message
        << std::endl;
    logFile.flush();
}

std::string Logger::getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    tm localTime;
    localtime_s(&localTime, &time);

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    return stream.str();
}