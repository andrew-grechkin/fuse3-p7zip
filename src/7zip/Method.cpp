#include "7zip-impl.hpp"
#include "exception.hpp"
#include "string.hpp"

namespace sevenzip {
	using namespace NWindows::NCOM;

	struct ImplMethod: public IMethod {
		ImplMethod(const ImplLib& lib, size_t idx)
			: _lib(lib)
			, _idx(idx)
		{}

		std::string name() const noexcept override;

	private:
		const ImplLib& _lib;
		size_t         _idx;
	};

	std::string ImplMethod::name() const noexcept
	{
		CPropVariant prop;
		CheckCom(_lib.GetMethodProperty(_idx, 1, &prop), "GetMethodProperty");
		if (prop.bstrVal) return utf8((const wchar_t*)prop.bstrVal);

		return std::string();
	}

	void ImplLib::fill_methods()
	{
		auto num_methods = UInt32();
		CheckCom(GetNumberOfMethods(&num_methods), "GetNumberOfMethods");

		for (size_t idx = 0; idx < num_methods; ++idx) {
			m_methods.emplace_back(std::make_unique<ImplMethod>(*this, idx));
		}
	}
} // namespace sevenzip
