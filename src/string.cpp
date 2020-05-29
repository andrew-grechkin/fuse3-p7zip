#include "string.hpp"
#include "exception.hpp"
#include <cstring>
#include <memory>
#include <iconv.h>

std::string utf8raw(const std::wstring& wstr, const char* encoding)
{
	return utf8(std::string(wstr.begin(), wstr.end()).c_str(), encoding);
}

std::string utf8(const wchar_t* wstr)
{
	auto conv = std::unique_ptr<void, decltype(&iconv_close)>(iconv_open("UTF-8", "UTF-32"), &iconv_close);
	CheckErrno(conv.get() != reinterpret_cast<iconv_t>(-1), "iconv_open: ");

	size_t in_size  = std::wcslen(wstr) * sizeof(wchar_t);
	size_t res_size = in_size * 6;
	auto   result   = std::unique_ptr<char>(new char[res_size + 1]());

	auto in_ptr  = (char*)wstr;
	auto res_ptr = result.get();

	iconv(conv.get(), NULL, NULL, &res_ptr, &res_size);
	CheckErrno(iconv(conv.get(), &in_ptr, &in_size, &res_ptr, &res_size) != static_cast<size_t>(-1), "iconv: ");

	return std::string(result.get());
}

std::string utf8(const char* str, const char* encoding)
{
	// https://gist.github.com/hakre/4188459
	auto conv = std::unique_ptr<void, decltype(&iconv_close)>(iconv_open("UTF-8", encoding), &iconv_close);
	CheckErrno(conv.get() != reinterpret_cast<iconv_t>(-1), "iconv_open: ");

	size_t in_size  = std::strlen(str);
	size_t res_size = in_size * 6;
	auto   result   = std::unique_ptr<char>(new char[res_size + 1]());

	auto in_ptr  = const_cast<char*>(str);
	auto res_ptr = result.get();

	iconv(conv.get(), NULL, NULL, &res_ptr, &res_size);
	CheckErrno(iconv(conv.get(), &in_ptr, &in_size, &res_ptr, &res_size) != static_cast<size_t>(-1), "iconv: ");

	return std::string(result.get());
}

std::vector<std::string> split(const std::string& d, std::string str)
{
	auto result = std::vector<std::string>();
	auto pos    = std::string::size_type();
	while ((pos = str.find(d)) != std::string::npos) {
		result.emplace_back(str.substr(0, pos));
		str.erase(0, pos + d.length());
	}
	result.emplace_back(str);
	return result;

	// TODO use regex (if needed)
	//auto re = std::regex(":+");

	//auto formats = std::string(override_formats_order);
	//auto it      = std::sregex_token_iterator(formats.begin(), formats.end(), re, -1);
	//// Keep a dummy end iterator - Needed to construct a vector
	//// using (start, end) iterators.
	//sregex_token_iterator end;

	//vector<string> vec(iter, end);

	//for (auto a : vec) {
	//cout << a << endl;
	//}
}
