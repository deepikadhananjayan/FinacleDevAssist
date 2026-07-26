#pragma once

#include "Issue.h"
#include <vector>


struct ValidationResult
{
    std::vector<Issue> errors;
    std::vector<Issue> warnings;

    bool noErrors = false;
    bool excpOccr = false;
};