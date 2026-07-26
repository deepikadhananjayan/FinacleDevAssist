#pragma once

#include <string>

enum class IssueType
{
    SCR_ERROR,
    SCR_WARNING,
};


struct Issue
{
    int line = 0;
    std::string message = "";
    IssueType type = IssueType::SCR_ERROR;
};