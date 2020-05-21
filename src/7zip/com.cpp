#include "com.hpp"

namespace com {
	HRESULT ConvertErrorToHRESULT(LONG error)
	{
		return (error == 0) ? S_OK : HRESULT_FROM_WIN32(error);
	}

	HRESULT ConvertBoolToHRESULT(bool result)
	{
		return result ? S_OK : ConvertErrorToHRESULT(::GetLastError());
	}

	UnknownImp::UnknownImp()
		: _ref_counter(1)
	{}

	uint32_t WINAPI UnknownImp::AddRef()
	{
		return ++_ref_counter;
	}

	ULONG WINAPI UnknownImp::Release()
	{
		if (--_ref_counter) {
			return use_count();
		}
		delete this;
		return 0;
	}

	HRESULT WINAPI UnknownImp::QueryInterface(REFIID riid, void** object)
	{
		if (object && (riid == IID_IUnknown)) {
			*object = static_cast<IUnknown*>(this);
			AddRef();
			return S_OK;
		}
		if (object) *object = nullptr;
		return E_NOINTERFACE;
	}

	ULONG UnknownImp::use_count() const
	{
		return _ref_counter.load();
	}
} // namespace com
