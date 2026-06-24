#pragma once
#include <string>

// string->wstring変換
std::wstring ConvertString(const std::string& str);

// wstring->string変換
std::string ConvertString(const std::wstring& wstr);

class MiscUtility {};
