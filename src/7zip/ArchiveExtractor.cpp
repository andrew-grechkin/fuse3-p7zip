#include "7zip-impl.hpp"
#include "exception.hpp"

namespace sevenzip {
	ULONG WINAPI ImplArchiveExtractor::AddRef()
	{
		return UnknownImp::AddRef();
	}

	ULONG WINAPI ImplArchiveExtractor::Release()
	{
		return UnknownImp::Release();
	}

	HRESULT WINAPI ImplArchiveExtractor::QueryInterface(REFIID riid, void** object)
	{
		HRESULT ret = S_OK;

		if (object && (riid == IID_IArchiveExtractCallback)) {
			*object = static_cast<IArchiveExtractCallback*>(this);
			AddRef();
		} else if (object && (riid == IID_ICryptoGetTextPassword)) {
			*object = static_cast<ICryptoGetTextPassword*>(this);
			AddRef();
		} else {
			ret = UnknownImp::QueryInterface(riid, object);
		}
		return ret;
	}

	ImplArchiveExtractor::~ImplArchiveExtractor() noexcept {}

	ImplArchiveExtractor::ImplArchiveExtractor(const ImplArchive& arc, const IExtractCallback& callback) noexcept
		: arc(arc)
		, callback(callback)
		, m_dest()
		, total()
	{}

	HRESULT WINAPI ImplArchiveExtractor::SetTotal(UInt64 size)
	{
		total       = size;
		HRESULT ret = callback.notify_progress(0, total) == true ? S_OK : S_FALSE;

		return ret;
	}

	HRESULT WINAPI ImplArchiveExtractor::SetCompleted(const UInt64* completeValue)
	{
		HRESULT ret = S_OK;

		if (completeValue) ret = callback.notify_progress(*completeValue, total) == true ? S_OK : S_FALSE;

		return ret;
	}

	HRESULT WINAPI ImplArchiveExtractor::GetStream(UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode)
	{
		*outStream = nullptr;

		//m_curr.reset(new CurrItem(askExtractMode, m_dest, arc.at(index)));

		if (askExtractMode != NArchive::NExtract::NAskMode::kExtract) return S_OK;
		if (file.get()) {
			*outStream = file.get();
		}

		//try {
		//if (m_curr->item.is_dir()) {
		//fsysx::dir::create_sh(m_curr->full_path);
		//} else {
		//// Create folders for file
		//size_t pos = m_curr->full_path.find_last_of(WPATH_SEPARATORS);
		//if (pos != ustring::npos) {
		//fsysx::dir::create_sh(m_curr->full_path.substr(0, pos));
		//}
		//
		//if (fsysx::is_exist(m_curr->full_path) && fsysx::is_file(m_curr->full_path)) {
		//fsysx::file::remove(m_curr->full_path);
		//}

		//FileWriteStreamImpl*             tmp(new FileWriteStream(m_curr->full_path, true));
		//com::Object<FileWriteStreamImpl> stream(tmp);
		//m_curr->stream = stream;
		//stream.detach(tmp);
		//*outStream = tmp;
		//}
		//} catch (exception::Abstract& e) {
		//return E_ABORT;
		//}
		return S_OK;
	}

	HRESULT WINAPI ImplArchiveExtractor::PrepareOperation(Int32 askExtractMode)
	{
		switch (askExtractMode) {
			case NArchive::NExtract::NAskMode::kExtract: break;
			case NArchive::NExtract::NAskMode::kTest: break;
			case NArchive::NExtract::NAskMode::kSkip: break;
		};
		return S_OK;
	}

	HRESULT WINAPI ImplArchiveExtractor::SetOperationResult(Int32 operationResult)
	{
		if (operationResult != NArchive::NExtract::NOperationResult::kOK) {
			//			failed_files.push_back(FailedFile(m_curr->item.path(), operationResult));
		} else {
			//if (m_curr->mode == NArchive::NExtract::NAskMode::kExtract) {
			//fsysx::set_attr(m_curr->full_path, m_curr->item.attr());
			//if (m_curr->stream) {
			//m_curr->stream->set_mtime(m_curr->item.mtime());
			//}
			//}
		}
		//m_curr.reset();
		return S_OK;
	}

	HRESULT WINAPI ImplArchiveExtractor::CryptoGetTextPassword(BSTR* pass)
	{
		//ustring ret = callback.request_password();
		//com::BStr(ret).detach(*pass);
		return S_OK;
	}

	void ImplArchiveExtractor::execute(bool test) {}

	File ImplArchiveExtractor::execute(uint32_t index, bool test)
	{
		const UInt32 indices[] = {index};
		file.reset(new FileWriteStream);
		file->AddRef();
		CheckCom(arc->Extract(indices, 1, test, static_cast<IArchiveExtractCallback*>(this)), "Extract");
		file->seek(0, IFile::Whence::set);

		return std::unique_ptr<IFile>(file.release());
	}
} // namespace sevenzip
