#ifndef INITGUID
#	define INITGUID
#endif

#include "CPP/Common/Common.h"
#include "CPP/Windows/Defs.h"
#include "CPP/7zip/MyVersion.h"

#include "7zip-impl.hpp"
#include "string.hpp"

namespace sevenzip {
	Lib open(const Path& path)
	{
		return std::make_unique<ImplLib>(path);
	}

	using namespace NWindows::NCOM;
	ImplLib::~ImplLib() noexcept {}

	ImplLib::ImplLib(const Path& path)
		: library::ImplDynamic(path)
	{
		GET_LIBRARY_SYM(Func_, CreateObject);
		GET_LIBRARY_SYM(Func_, GetMethodProperty);
		GET_LIBRARY_SYM(Func_, GetNumberOfFormats);
		GET_LIBRARY_SYM(Func_, GetNumberOfMethods);
		GET_LIBRARY_SYM_NT(Func_, GetHandlerProperty);
		GET_LIBRARY_SYM_NT(Func_, GetHandlerProperty2);
		GET_LIBRARY_SYM_NT(Func_, SetLargePageMode);

		// CheckApiThrowError(GetHandlerProperty2 || GetHandlerProperty,

		fill_formats();
		fill_methods();
	}

	const Path& ImplLib::path() const noexcept
	{
		return library::ImplDynamic::path();
	}

	const Formats& ImplLib::formats() const noexcept
	{
		return m_formats;
	}

	const Methods& ImplLib::methods() const noexcept
	{
		return m_methods;
	}

	Archive ImplLib::open(const Path& path, const IOpenCallback& callback, flags_type flags) const
	{
		return std::make_unique<ImplArchive>(*this, path, callback, flags);
	}

	HRESULT ImplLib::get_prop(UInt32 index, PROPID prop_id, NWindows::NCOM::CPropVariant& prop) const noexcept
	{
		return GetHandlerProperty2 ? GetHandlerProperty2(index, prop_id, &prop) : GetHandlerProperty(prop_id, &prop);
	}

	HRESULT ImplLib::get_prop(UInt32 index, PROPID prop_id, bool& value) const noexcept
	{
		CPropVariant prop;
		auto         res = get_prop(index, prop_id, prop);
		if (res == S_OK) value = prop.boolVal;
		return res;
	}

	HRESULT ImplLib::get_prop(UInt32 index, PROPID prop_id, std::string& value) const noexcept
	{
		CPropVariant prop;
		auto         res = get_prop(index, prop_id, prop);
		if (res == S_OK && prop.bstrVal) value = utf8((const wchar_t*)prop.bstrVal);

		return res;
	}
} // namespace sevenzip
