#ifndef __FUSE3_7Z__7ZIP_PRIVATE_HPP_
#define __FUSE3_7Z__7ZIP_PRIVATE_HPP_

#include "7zip.hpp"
#include "com.hpp"
#include "exception.hpp"
#include "library-impl.hpp"

#define ENV_HAVE_WCTYPE_H
#include "CPP/Common/Common.h"
#include "CPP/Windows/Defs.h"
#include "CPP/7zip/MyVersion.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "CPP/Windows/PropVariant.h"

#define UNUSED(val) ((void)val)

namespace sevenzip {
	class FileReadStream
		: public IInStream
		, private com::UnknownImp {
	public:
		class FileHandle;

		~FileReadStream() noexcept;
		explicit FileReadStream(const Path& path);

		const Path& path() const noexcept;
		const Stat& stat() const noexcept;

		// Unknown
		ULONG WINAPI   AddRef() override;
		ULONG WINAPI   Release() override;
		HRESULT WINAPI QueryInterface(REFIID riid, void** object) override;

		// ISequentialInStream
		HRESULT WINAPI Read(void* data, UInt32 size, UInt32* processedSize) noexcept override;

		// IInStream
		HRESULT WINAPI Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) noexcept override;

	private:
		Path        _path;
		Stat        _stat;
		FileHandle* _file;
	};

	class FileWriteStream
		: public IOutStream
		, public IFile
		, private com::UnknownImp {
	public:
		class FileHandle;

		~FileWriteStream() noexcept;
		FileWriteStream();

		// Unknown
		ULONG WINAPI   AddRef() override;
		ULONG WINAPI   Release() override;
		HRESULT WINAPI QueryInterface(REFIID riid, void** object) override;

		// ISequentialOutStream
		HRESULT WINAPI Write(const void* data, UInt32 size, UInt32* processedSize) noexcept override;

		// IOutStream
		HRESULT WINAPI Seek(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) noexcept override;
		HRESULT WINAPI SetSize(UInt64 newSize) noexcept override;

		// IFile
		uint64_t read(void* data, uint64_t size) override;
		uint64_t write(const void* data, uint64_t size) override;
		uint64_t seek(uint64_t pos, Whence whence) override;

	private:
		FileHandle* _file;
	};

	class ImplLib
		: public ILib
		, private library::ImplDynamic {
	public:
		DEFINE_SYM(Func_, CreateObject);
		DEFINE_SYM(Func_, GetHandlerProperty);
		DEFINE_SYM(Func_, GetHandlerProperty2);
		DEFINE_SYM(Func_, GetMethodProperty);
		DEFINE_SYM(Func_, GetNumberOfFormats);
		DEFINE_SYM(Func_, GetNumberOfMethods);
		DEFINE_SYM(Func_, SetLargePageMode);

		~ImplLib() noexcept;
		ImplLib(const Path& path);

		const Path&    path() const noexcept override;
		const Formats& formats() const noexcept override;
		const Methods& methods() const noexcept override;

		Archive open(const Path& path, const IOpenCallback& callback, flags_type flags) const override;

		HRESULT get_prop(UInt32 index, PROPID prop_id, NWindows::NCOM::CPropVariant& prop) const noexcept;
		HRESULT get_prop(UInt32 index, PROPID prop_id, bool& value) const noexcept;
		HRESULT get_prop(UInt32 index, PROPID prop_id, std::string& value) const noexcept;

	private:
		void fill_formats();
		void fill_methods();

		Formats m_formats;
		Methods m_methods;
	};

	class ImplArchive
		: public IArchive
		, public IArchiveOpenCallback
		, public ICryptoGetTextPassword
		, private com::UnknownImp {
	public:
		~ImplArchive() noexcept;
		ImplArchive(const ImplLib& lib, const Path& path, const IOpenCallback& callback,
					flags_type flasgs = flags_type());

		// IUnknown
		ULONG WINAPI   AddRef() override;
		ULONG WINAPI   Release() override;
		HRESULT WINAPI QueryInterface(REFIID riid, void** object) override;

		// IArchiveOpenCallback
		HRESULT WINAPI SetTotal(const UInt64* files, const UInt64* bytes) noexcept override;
		HRESULT WINAPI SetCompleted(const UInt64* files, const UInt64* bytes) noexcept override;

		// ICryptoGetTextPassword
		HRESULT WINAPI CryptoGetTextPassword(BSTR* password) noexcept override;

		const Path&   path() const noexcept override;
		const Stat&   stat() const noexcept override;
		const Format& format() const noexcept override;
		std::string   proposed_name() const noexcept override;

		flags_type flags() const noexcept override;
		bool       empty() const noexcept override;
		size_t     size() const noexcept override;

		const_iterator begin() const noexcept override;
		const_iterator end() const noexcept override;
		const_iterator at(size_t index) const override;

		com::Object<IInArchive> operator->() const;
		operator com::Object<IInArchive>() const;

	protected:
		void open_archive(const ImplLib& lib);
		void init_props();

		const IOpenCallback&        callback;
		com::Object<FileReadStream> _stream;
		com::Object<IInArchive>     _arc;
		Format                      _format;
		UInt32                      _size;
		flags_type                  _flags;
	};

	struct IArchive::ci_iterator::Impl {
		template <typename... Args> static std::unique_ptr<Impl> create(Args&&... args)
		{
			return std::make_unique<Impl>(std::forward<Args>(args)...);
		}

		Impl();
		explicit Impl(const ImplArchive& seq);
		Impl(const ImplArchive& seq, UInt32 index);

		HRESULT get_prop(PROPID id, NWindows::NCOM::CPropVariant& prop) const;
		File    extract(const IExtractCallback& callback) const;

		const ImplArchive* _seq;
		UInt32             _index;
		bool               _end;
	};

	class ImplArchiveExtractor
		: public IArchiveExtractor
		, public IArchiveExtractCallback
		, public ICryptoGetTextPassword
		, private com::UnknownImp {
	public:
		~ImplArchiveExtractor() noexcept;
		ImplArchiveExtractor(const ImplArchive& arc, const IExtractCallback& callback) noexcept;

		// IUnknown
		ULONG WINAPI   AddRef() override;
		ULONG WINAPI   Release() override;
		HRESULT WINAPI QueryInterface(REFIID riid, void** object) override;

		// IProgress
		HRESULT WINAPI SetTotal(UInt64 size) noexcept override;
		HRESULT WINAPI SetCompleted(const UInt64* completeValue) noexcept override;

		// IArchiveExtractCallback
		HRESULT WINAPI GetStream(UInt32 index, ISequentialOutStream** outStream,
								 Int32 askExtractMode) noexcept override;
		HRESULT WINAPI PrepareOperation(Int32 askExtractMode) noexcept override;
		HRESULT WINAPI SetOperationResult(Int32 resultEOperationResult) noexcept override;

		// ICryptoGetTextPassword
		HRESULT WINAPI CryptoGetTextPassword(BSTR* pass) noexcept override;

		// IArchiveExtractor
		void execute(bool test) override;
		File execute(uint32_t index, bool test);

	private:
		const ImplArchive&               arc;
		const IExtractCallback&          callback;
		Path                             m_dest;
		UInt64                           total;
		std::unique_ptr<FileWriteStream> file;
	};
} // namespace sevenzip

#endif
