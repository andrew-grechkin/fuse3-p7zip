#include "7zip-impl.hpp"
#include "exception.hpp"
#include "logger.hpp"

namespace sevenzip {
	class FileWriteStream::FileHandle: public std::FILE {
	};

	HRESULT check_error(FileWriteStream::FileHandle* file)
	{
		if (std::ferror(file)) {
			LogDebug("FileWriteStream error occured");
			return 1;
		}
		return S_OK;
	}

	FileWriteStream::~FileWriteStream() noexcept
	{
		LogTrace();
		std::fclose(_file);
	}

	FileWriteStream::FileWriteStream()
		: _file(static_cast<FileHandle*>(std::tmpfile()))
	{
		LogTrace();
		CheckErrno(_file, "tmpfile: ");
	}

	ULONG WINAPI FileWriteStream::AddRef()
	{
		LogTrace();
		return UnknownImp::AddRef();
	}

	ULONG WINAPI FileWriteStream::Release()
	{
		LogTrace();
		return UnknownImp::Release();
	}

	HRESULT WINAPI FileWriteStream::QueryInterface(REFIID riid, void** object)
	{
		LogTrace();
		HRESULT ret = S_OK;

		if (object && (riid == IID_IOutStream)) {
			LogDebug("FileWriteStream::QueryInterface IID_IOutStream");
			*object = static_cast<IOutStream*>(this);
			AddRef();
		} else if (object && (riid == IID_ISequentialOutStream)) {
			LogDebug("FileWriteStream::QueryInterface IID_ISequentialOutStream");
			*object = static_cast<ISequentialOutStream*>(this);
			AddRef();
		} else {
			LogDebug("FileWriteStream::QueryInterface UnknownImp");
			ret = UnknownImp::QueryInterface(riid, object);
		}
		return ret;
	}

	HRESULT WINAPI FileWriteStream::Write(const void* data, UInt32 size, UInt32* processedSize)
	{
		LogTrace();
		DWORD written = std::fwrite(data, 1, size, _file);
		if (processedSize) *processedSize = written;
		LogDebug("Write: %d, %d", size, written);
		return check_error(_file);
	}

	HRESULT WINAPI FileWriteStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition)
	{
		LogTrace();
		std::fseek(_file, offset, seekOrigin);
		auto pos = std::ftell(_file);
		if (newPosition) *newPosition = pos;
		LogDebug("Seek: %d, %d, %d", offset, seekOrigin, pos);
		return check_error(_file);
	}

	HRESULT WINAPI FileWriteStream::SetSize(UInt64 newSize)
	{
		LogTrace();
		auto pos = std::ftell(_file);
		ftruncate(_file->_fileno, newSize);
		std::fseek(_file, pos, SEEK_SET);
		LogDebug("SetSize: %d", newSize);
		return check_error(_file);
	}

	uint64_t FileWriteStream::read(void* data, uint64_t size)
	{
		LogTrace();
		auto read = std::fread(data, 1, size, _file);
		LogDebug("read: %ld", size);
		return read;
	}

	uint64_t FileWriteStream::write(const void* data, uint64_t size)
	{
		LogTrace();
		auto written = std::fwrite(data, 1, size, _file);
		LogDebug("write: %ld", size);
		return written;
	}

	uint64_t FileWriteStream::seek(uint64_t pos, Whence whence)
	{
		LogTrace();
		std::fseek(_file, pos, whence);
		LogDebug("seek: %ld", pos);
		return std::ftell(_file);
	}
} // namespace sevenzip
