#include "7zip-impl.hpp"
#include "exception.hpp"

namespace sevenzip {
	class FileReadStream::FileHandle: public std::FILE {};

	HRESULT check_error(FileReadStream::FileHandle* file)
	{
		if (std::ferror(file)) return 1;
		return S_OK;
	}

	ULONG WINAPI FileReadStream::AddRef()
	{
		return UnknownImp::AddRef();
	}

	ULONG WINAPI FileReadStream::Release()
	{
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileReadStream::QueryInterface(REFIID riid, void** object)
	{
		HRESULT ret = S_OK;
		if (object && (riid == IID_IInStream)) {
			AddRef();
			*object = static_cast<IInStream*>(this);
		} else if (object && (riid == IID_ISequentialInStream)) {
			AddRef();
			*object = static_cast<ISequentialInStream*>(this);
		} else {
			ret = UnknownImp::QueryInterface(riid, object);
		}
		return ret;
	}

	const Path& FileReadStream::path() const noexcept
	{
		return _path;
	}

	const Stat& FileReadStream::stat() const noexcept
	{
		return _stat;
	}

	FileReadStream::~FileReadStream() noexcept
	{
		std::fclose(_file);
	}

	FileReadStream::FileReadStream(const Path& path)
		: _path(path)
		, _file(static_cast<FileHandle*>(std::fopen(_path.c_str(), "r")))
	{
		CheckErrno(_file, std::string("fopen: ") + path.string());
		lstat(_path.c_str(), &_stat);
	}

	HRESULT WINAPI FileReadStream::Read(void* data, UInt32 size, UInt32* processedSize)
	{
		auto read = std::fread(data, 1, size, _file);
		if (processedSize) *processedSize = read;
		return check_error(_file);
	}

	HRESULT WINAPI FileReadStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition)
	{
		if (std::fseek(_file, offset, seekOrigin) == 0) {
			if (newPosition) *newPosition = std::ftell(_file);
			return S_OK;
		}
		return check_error(_file);
	}
} // namespace sevenzip
