#include "7zip-impl.hpp"
#include "exception.hpp"
#include "string.hpp"

#include <algorithm>
#include <cstring>

namespace sevenzip {
	struct Guid: public GUID {};

	class ImplFormat: public IFormat {
	public:
		ImplFormat(const ImplLib& lib, size_t idx)
			: _lib(lib)
			, _idx(idx)
		{
			std::string result;
			_lib.get_prop(_idx, NArchive::NHandlerPropID::kExtension, result);
			_ext_def = split(" ", result);

			result.clear();
			_lib.get_prop(_idx, NArchive::NHandlerPropID::kAddExtension, result);
			_ext_add = split(" ", result);
		}

		std::string       name() const noexcept override;
		const Extensions& default_extension() const noexcept override;
		const Extensions& additional_extension() const noexcept override;
		const Guid*       guid() const noexcept override;
		bool              updatable() const noexcept override;

	private:
		const ImplLib& _lib;
		size_t         _idx;
		mutable Guid   _guid;
		Extensions     _ext_def;
		Extensions     _ext_add;
	};

	std::string ImplFormat::name() const noexcept
	{
		std::string result;
		CheckCom(_lib.get_prop(_idx, NArchive::NHandlerPropID::kName, result), "get_prop");
		return result;
	}

	const Extensions& ImplFormat::default_extension() const noexcept
	{
		return _ext_def;
	}

	const Extensions& ImplFormat::additional_extension() const noexcept
	{
		return _ext_add;
	}

	const Guid* ImplFormat::guid() const noexcept
	{
		NWindows::NCOM::CPropVariant prop;
		if (_lib.get_prop(_idx, NArchive::NHandlerPropID::kClassID, prop) == S_OK)
			std::memcpy(&_guid, prop.bstrVal, sizeof(GUID));
		return &_guid;
	}

	bool ImplFormat::updatable() const noexcept
	{
		bool result = false;
		CheckCom(_lib.get_prop(_idx, NArchive::NHandlerPropID::kUpdate, result), "get_prop");
		return result;
	}

	void ImplLib::fill_formats()
	{
		auto num_formats = UInt32();
		CheckCom(GetNumberOfFormats(&num_formats), "GetNumberOfFormats");

		for (size_t idx = 0; idx < num_formats; ++idx) {
			m_formats.emplace_back(std::make_shared<ImplFormat>(*this, idx));
		}

		auto override_formats_order = std::getenv("FUSE3_P7ZIP_FORMATS");
		if (override_formats_order) {
			auto formats = split(":", override_formats_order);

			Formats head, tail, *current = &head;
			for (auto& it : formats) {
				if (it == "*") {
					current = &tail;
					continue;
				}
				// there are ~40 formats so let's not bother with binary search here
				auto f =
					std::find_if(m_formats.begin(), m_formats.end(), [it](const auto& i) { return i->name() == it; });
				if (f != m_formats.end()) {
					current->emplace_back(*f);
					m_formats.erase(f);
				}
			}
			m_formats.insert(m_formats.begin(), head.begin(), head.end());
			m_formats.insert(m_formats.end(), tail.begin(), tail.end());
		}
	}
} // namespace sevenzip
