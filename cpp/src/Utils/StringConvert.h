#pragma once

#include <string>
#include <windows.h>

std::wstring NarrowToWide(const std::string& s);
std::string  WideToNarrow(const TCHAR* wide);
std::string wideToUtf8(const std::wstring& value);