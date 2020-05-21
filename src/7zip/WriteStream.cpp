#include "7zip-impl.hpp"
#include "exception.hpp"

namespace sevenzip {
	class FileWriteStream::FileHandle: public std::FILE {
	};

	HRESULT check_error(FileWriteStream::FileHandle* file)
	{
		if (std::ferror(file)) return 1;
		return S_OK;
	}

	FileWriteStream::~FileWriteStream() noexcept
	{
		std::fclose(_file);
	}

	FileWriteStream::FileWriteStream()
		: _file(static_cast<FileHandle*>(std::tmpfile()))
	{
		CheckErrno(_file, "tmpfile: ");
	}

	ULONG WINAPI FileWriteStream::AddRef()
	{
		return UnknownImp::AddRef();
	}

	ULONG WINAPI FileWriteStream::Release()
	{
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileWriteStream::QueryInterface(REFIID riid, void** object)
	{
		HRESULT ret = S_OK;

		if (object && (riid == IID_IOutStream)) {
			*object = static_cast<IOutStream*>(this);
			AddRef();
		} else if (object && (riid == IID_ISequentialOutStream)) {
			*object = static_cast<ISequentialOutStream*>(this);
			AddRef();
		} else {
			ret = UnknownImp::QueryInterface(riid, object);
		}
		return ret;
	}

	HRESULT WINAPI FileWriteStream::Write(const void* data, UInt32 size, UInt32* processedSize)
	{
		DWORD written = std::fwrite(data, 1, size, _file);
		if (processedSize) *processedSize = written;
		return check_error(_file);
	}

	HRESULT WINAPI FileWriteStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition)
	{
		std::fseek(_file, offset, seekOrigin);
		auto pos = std::ftell(_file);
		if (newPosition) *newPosition = pos;
		return check_error(_file);
	}

	HRESULT WINAPI FileWriteStream::SetSize(UInt64 newSize)
	{
		auto pos = std::ftell(_file);
		ftruncate(_file->_fileno, newSize);
		std::fseek(_file, pos, SEEK_SET);
		return check_error(_file);
	}

	uint64_t FileWriteStream::read(void* data, uint64_t size)
	{
		auto read = std::fread(data, 1, size, _file);
		return read;
	}

	uint64_t FileWriteStream::write(const void* data, uint64_t size)
	{
		auto written = std::fwrite(data, 1, size, _file);
		return written;
	}

	uint64_t FileWriteStream::seek(uint64_t pos, Whence whence)
	{
		std::fseek(_file, pos, whence);
		return std::ftell(_file);
	}
} // namespace sevenzip
