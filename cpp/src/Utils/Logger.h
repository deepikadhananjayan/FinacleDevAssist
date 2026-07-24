#pragma once

#include <fstream>
#include <mutex>
#include <string>


class Logger
{
public:

    static bool initialize();

    static void shutdown();


    static void info(const std::string& message);

    static void debug(const std::string& message);

    static void warn(const std::string& message);

    static void error(const std::string& message);


private:

    static void write(
        const std::string& level,
        const std::string& message
    );


    static std::string getCurrentTimestamp();


private:

    static std::ofstream logFile;

    static std::mutex logMutex;

    static bool initialized;
};