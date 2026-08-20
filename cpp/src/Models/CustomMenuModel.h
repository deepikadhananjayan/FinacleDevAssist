#pragma once

#include <string>
#include <vector>

struct Option
{
    std::string value;
    std::string label;
};

struct Field
{
    std::string fieldPlacement;
    std::string fieldType;
    std::string fieldLabel;
    std::string fieldId;
    std::string fieldDescription;
    std::string searcher;
    bool isDisabled = false;
    bool isMandatory = false;
    std::vector<Option> options;
};

struct CustomMenuData
{
    std::string menuType;
    std::string menuName;
    std::string menuDescription;
    std::vector<std::string> generateDesign;
    std::vector<Field> fields;
};