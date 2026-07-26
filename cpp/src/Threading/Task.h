#pragma once

#include <string>

enum class TaskType
{
    START_JAVA,
    GET_KEYWORDS_AND_USERHOOKS,
    VALIDATE_SCRIPT,
    FORMAT_SCRIPT,
    GET_SUGGESTIONS,
    STOP_JAVA
};

struct Task
{
    TaskType type;
    std::string data;
};