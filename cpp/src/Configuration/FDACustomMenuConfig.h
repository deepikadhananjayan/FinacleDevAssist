#pragma once

#include <string>
#include <vector>

class FDACustomMenuConfig
{
public:
    static bool load();

    static const std::vector<std::string>& getMenuTypes();
    static const std::vector<std::string>& getGenerateDesigns();
    static const std::vector<std::string>& getFieldPlacements();
    static const std::vector<std::string>& getFieldTypes();
    static const std::vector<std::string>& getSearchers();

private:
    static std::vector<std::string> menuTypes;
    static std::vector<std::string> generateDesigns;
    static std::vector<std::string> fieldPlacements;
    static std::vector<std::string> fieldTypes;
    static std::vector<std::string> searchers;
};