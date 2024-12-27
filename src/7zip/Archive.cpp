#include "7zip-impl.hpp"
#include "exception.hpp"
#include "string.hpp"
#include "logger.hpp"

#include <fcntl.h>

namespace sevenzip {
	const UInt64 max_check_size = 100 * 1024 * 1024;

	Time _to_time(const FILETIME& ft)
	{
		uint64_t date   = *(reinterpret_cast<const uint64_t*>(&ft));
		uint64_t adjust = 11644473600000 * 10000;
		date -= adjust;

		return Time{static_cast<decltype(Time::tv_sec)>(date / 10000000), static_cast<decltype(Time::tv_nsec)>(date)};
	}

	ImplArchive::~ImplArchive() noexcept
	{
		_arc->Close();
	}

	ImplArchive::ImplArchive(const ImplLib& lib, const Path& path, const IOpenCallback& callback, flags_type flags)
		: callback(callback)
		, _stream(new FileReadStream(path))
		, _flags(flags)
	{
		open_archive(lib);
		init_props();
	}

	void ImplArchive::open_archive(const ImplLib& lib)
	{
		for (auto& it : lib.formats()) {
			CheckCom(_stream->Seek(0, STREAM_SEEK_SET, nullptr), "Seek");

			CheckCom(lib.CreateObject(reinterpret_cast<const GUID*>(it->guid()), &IID_IInArchive, (void**)&_arc),
					 "CreateObject");

			auto res = _arc->Open(_stream, &max_check_size, static_cast<IArchiveOpenCallback*>(this));
			if (res == S_OK) {
				_format = it;
				log().printf("selected format: %s", it->name().c_str());
				return;
			}
		}
		throw std::runtime_error("ERROR_INVALID_DATA");
	}

	void ImplArchive::init_props()
	{
		CheckCom(_arc->GetNumberOfItems(&_size), "GetNumberOfItems");
		LogDebug("GetNumberOfItems: %d", _size);

		auto num_props = UInt32();
		CheckCom(_arc->GetNumberOfArchiveProperties(&num_props), "GetNumberOfArchiveProperties");
		LogDebug("GetNumberOfArchiveProperties: %d", num_props);
	}

	ULONG WINAPI ImplArchive::AddRef()
	{
		return UnknownImp::AddRef();
	}

	ULONG WINAPI ImplArchive::Release()
	{
		return UnknownImp::Release();
	}

	HRESULT WINAPI ImplArchive::QueryInterface(REFIID riid, void** object)
	{
		HRESULT ret = S_OK;

		if (object && (riid == IID_IArchiveOpenCallback)) {
			AddRef();
			*object = static_cast<IArchiveOpenCallback*>(this);
		} else if (object && (riid == IID_ICryptoGetTextPassword)) {
			AddRef();
			*object = static_cast<ICryptoGetTextPassword*>(this);
		} else {
			ret = UnknownImp::QueryInterface(riid, object);
		}
		return ret;
	}

	HRESULT WINAPI ImplArchive::SetTotal(const UInt64* files, const UInt64* bytes) noexcept
	{
		UNUSED(files);
		UNUSED(bytes);
		HRESULT ret = S_OK;

		return ret;
	}

	HRESULT WINAPI ImplArchive::SetCompleted(const UInt64* files, const UInt64* bytes) noexcept
	{
		UNUSED(files);
		UNUSED(bytes);
		HRESULT ret = S_OK;

		return ret;
	}

	HRESULT WINAPI ImplArchive::CryptoGetTextPassword(BSTR* pass) noexcept
	{
		LogDebug("%s %p", __PRETTY_FUNCTION__, pass);
		*pass = SysAllocString(wide_str(callback.request_password().c_str()).c_str());
		return S_OK;
	}

	const Path& ImplArchive::path() const noexcept
	{
		return _stream->path();
	}

	const Stat& ImplArchive::stat() const noexcept
	{
		return _stream->stat();
	}

	const Format& ImplArchive::format() const noexcept
	{
		return _format;
	}

	std::string ImplArchive::proposed_name() const noexcept
	{
		auto name    = path().filename();
		auto ext     = path().extension();
		auto ext_def = _format->default_extension();
		auto ext_add = _format->additional_extension();

		for (auto it = ext_def.begin(); it != ext_def.end(); ++it) {
			auto ext_try = (std::string(".") + *it);
			if (ext == ext_try) {
				auto ext_new = ext_add[std::distance(ext_def.begin(), it)];
				if (ext_new != "*") {
					name.replace_extension(ext_new);
					break;
				}
			}
		}
		return std::string(name);
	}

	com::Object<IInArchive> ImplArchive::operator->() const
	{
		return _arc;
	}

	ImplArchive::operator com::Object<IInArchive>() const
	{
		return _arc;
	}

	bool ImplArchive::empty() const noexcept
	{
		return _size == 0;
	}

	size_t ImplArchive::size() const noexcept
	{
		return _size;
	}

	ImplArchive::const_iterator ImplArchive::begin() const noexcept
	{
		return const_iterator(ci_iterator::Impl::create(*this));
	}

	ImplArchive::const_iterator ImplArchive::end() const noexcept
	{
		return const_iterator(ci_iterator::Impl::create());
	}

	ImplArchive::const_iterator ImplArchive::at(size_t index) const
	{
		if (index >= ImplArchive::size()) CheckCom(1, "out of bounds");
		return const_iterator(ci_iterator::Impl::create(*this, index));
	}

	flags_type ImplArchive::flags() const noexcept
	{
		return _flags;
	}

	ImplArchive::ci_iterator::~ci_iterator() noexcept = default;

	ImplArchive::ci_iterator::ci_iterator(const ci_iterator& other)
		: impl(ci_iterator::Impl::create(*other.impl))
	{}

	ImplArchive::ci_iterator::ci_iterator(ci_iterator&& other) noexcept
		: impl(std::move(other.impl))
	{}

	ImplArchive::ci_iterator& ImplArchive::ci_iterator::operator=(const ci_iterator& other)
	{
		if (impl != other.impl) ci_iterator(other).swap(*this);
		return *this;
	}

	ImplArchive::ci_iterator& ImplArchive::ci_iterator::operator=(ci_iterator&& other) noexcept
	{
		if (impl != other.impl) ci_iterator(std::move(other)).swap(*this);
		return *this;
	}

	ImplArchive::ci_iterator& ImplArchive::ci_iterator::operator++()
	{
		//flags_type flags = impl->_seq->flags();
		while (true) {
			if (++impl->_index >= impl->_seq->size()) {
				impl->_end = true;
				break;
			}

			// if ((flags & skipHidden) && ((attr() & FILE_ATTRIBUTE_HIDDEN) ==
			// FILE_ATTRIBUTE_HIDDEN)) { continue;
			//}

			//if ((flags & skipDirs) && is_dir()) {
			//continue;
			//}

			//if ((flags & skipFiles) && is_file()) {
			//continue;
			//}
			break;
		}
		return *this;
	}

	ImplArchive::ci_iterator ImplArchive::ci_iterator::operator++(int)
	{
		ci_iterator ret(*this);
		operator++();
		return ret;
	}

	std::string ImplArchive::ci_iterator::path() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidPath, prop);
		//log().printf("prop path: %d\n", res);
		//std::setlocale(LC_ALL, "");
		//if (res == S_OK && prop.bstrVal) log().printf("prop bstr (l): '%ls'\n", (const wchar_t*)prop.bstrVal);
		//if (res == S_OK && prop.bstrVal) log().printf("prop bstr (u): '%s'\n", utf8((const wchar_t*)prop.bstrVal).c_str());
		if (res == S_OK && prop.bstrVal) {
			auto override_encoding = std::getenv("FUSE3_P7ZIP_FORCE_ENCODING");
			return override_encoding ? utf8raw((const wchar_t*)prop.bstrVal, override_encoding)
									 : utf8((const wchar_t*)prop.bstrVal);
		};

		return std::string();
	}

	std::string ImplArchive::ci_iterator::name() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidName, prop);
		if (res == S_OK && prop.vt == 8 && prop.bstrVal) return utf8((const wchar_t*)prop.bstrVal);
		return std::string();
	}

	std::string ImplArchive::ci_iterator::extension() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidExtension, prop);
		if (res == S_OK && prop.vt == 8 && prop.bstrVal) return utf8((const wchar_t*)prop.bstrVal);
		return std::string();
	}

	std::string ImplArchive::ci_iterator::comment() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidComment, prop);
		if (res == S_OK && prop.vt == 8 && prop.bstrVal) return utf8((const wchar_t*)prop.bstrVal);
		return std::string();
	}

	Size ImplArchive::ci_iterator::size() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidSize, prop);
		if (res == S_OK && prop.vt == 21) return prop.uhVal.QuadPart;
		return 0;
	}

	Size ImplArchive::ci_iterator::size_packed() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidPackSize, prop);
		if (res == S_OK && prop.vt == 21) return prop.uhVal.QuadPart;
		return 0;
	}

	Time ImplArchive::ci_iterator::atime() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidATime, prop);
		if (res == S_OK && prop.vt == VT_FILETIME && (prop.filetime.dwLowDateTime || prop.filetime.dwHighDateTime))
			return _to_time(prop.filetime);
		return Time{0, 0};
	}

	Time ImplArchive::ci_iterator::mtime() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidMTime, prop);
		if (res == S_OK && prop.vt == VT_FILETIME && (prop.filetime.dwLowDateTime || prop.filetime.dwHighDateTime))
			return _to_time(prop.filetime);
		return Time{0, 0};
	}

	Time ImplArchive::ci_iterator::ctime() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidCTime, prop);
		if (res == S_OK && prop.vt == VT_FILETIME && (prop.filetime.dwLowDateTime || prop.filetime.dwHighDateTime))
			return _to_time(prop.filetime);
		return Time{0, 0};
	}

	bool ImplArchive::ci_iterator::is_file() const
	{
		return !is_dir();
	}

	bool ImplArchive::ci_iterator::is_dir() const
	{
		NWindows::NCOM::CPropVariant prop;
		auto                         res = impl->get_prop(kpidIsDir, prop);
		if (res == S_OK && prop.vt == 11) return prop.boolVal;
		return false;
	}

	File ImplArchive::ci_iterator::extract(const IExtractCallback& callback) const
	{
		return impl->extract(callback);
	}

	size_t ImplArchive::ci_iterator::get_props_count() const
	{
		UInt32 props = 0;
		static_cast<com::Object<IInArchive>>(*impl->_seq)->GetNumberOfProperties(&props);
		return props;
	}

	bool ImplArchive::ci_iterator::operator==(const ci_iterator& rhs) const noexcept
	{
		if (impl->_end && rhs.impl->_end) return true;
		return impl->_seq == rhs.impl->_seq && impl->_index == rhs.impl->_index;
	}

	bool ImplArchive::ci_iterator::operator!=(const ci_iterator& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	void ImplArchive::ci_iterator::swap(ci_iterator& other) noexcept
	{
		using std::swap;
		swap(impl, other.impl);
	}

	ImplArchive::ci_iterator::ci_iterator(Impl_type&& impl) noexcept
		: impl(std::move(impl))
	{}

	ImplArchive::ci_iterator::Impl::Impl()
		: _seq(nullptr)
		, _index(0)
		, _end(true)
	{}

	ImplArchive::ci_iterator::Impl::Impl(const ImplArchive& seq)
		: _seq((ImplArchive*)&seq)
		, _index(0)
		, _end(!_seq->size())
	{}

	ImplArchive::ci_iterator::Impl::Impl(const ImplArchive& seq, UInt32 index)
		: _seq((ImplArchive*)&seq)
		, _index(index)
		, _end(!_seq->size() || index >= _seq->size())
	{}

	HRESULT ImplArchive::ci_iterator::Impl::get_prop(PROPID id, NWindows::NCOM::CPropVariant& prop) const
	{
		return static_cast<com::Object<IInArchive>>(*_seq)->GetProperty(_index, id, &prop);
	}

	File ImplArchive::ci_iterator::Impl::extract(const IExtractCallback& callback) const
	{
		auto extractor = com::Object<ImplArchiveExtractor>(new ImplArchiveExtractor(*_seq, callback));
		return extractor->execute(_index, false);
	}
} // namespace sevenzip
