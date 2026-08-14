#pragma once

#include <string>
#include <variant>
#include "BeautifyData.h"
#include "FIRequestData.h"

enum class TaskType
{
    START_JAVA,
    GET_KEYWORDS_AND_USERHOOKS,
    VALIDATE_SCRIPT,
    BEAUTIFY_CODE,
    GENERATE_CUSTOM_MENU,
    DEPLOY_CUSTOM_MENU,
    EXECUTE_FI_REQUEST,
    GET_SUGGESTIONS,
    STOP_JAVA
};

struct Task
{
    TaskType type;

    std::variant<
        std::monostate,
        std::string,
        BeautifyData,
        FIRequestData
    > data;
};