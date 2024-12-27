#ifndef __FUSE3_7Z__STRING_HPP_
#define __FUSE3_7Z__STRING_HPP_

#include <string>
#include <vector>

std::string chomp(const std::string& str);

std::string utf8raw(const std::wstring& wstr, const char* encoding);

std::string utf8(const wchar_t* str);

std::string utf8(const char* str, const char* encoding);

std::wstring wide_str(const char* str);

std::vector<std::string> split(const std::string& d, std::string str);

#endif
