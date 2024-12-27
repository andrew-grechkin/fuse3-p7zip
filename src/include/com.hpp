#ifndef __FUSE3_7Z__COM_HPP_
#define __FUSE3_7Z__COM_HPP_

#include <atomic>

#include "CPP/Common/Common.h"

namespace com {
	HRESULT ConvertErrorToHRESULT(LONG error);
	HRESULT ConvertBoolToHRESULT(bool result);

	struct UnknownImp: public IUnknown {
		virtual ~UnknownImp() = default;
		UnknownImp();

		ULONG          use_count() const;
		ULONG WINAPI   AddRef() override;
		ULONG WINAPI   Release() override;
		HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObject) override;

	private:
		std::atomic<ULONG> _ref_counter;
	};

	template <typename Interface> class Object {
		using this_type     = Object;
		using pointer       = Interface*;
		using const_pointer = const Interface*;

	public:
		~Object();

		Object() noexcept;
		explicit Object(const_pointer param);
		Object(const this_type& other) noexcept;
		Object(this_type&& other) noexcept;
		template <typename OInterface> Object(const Object<OInterface>& other);

		this_type&                                operator=(const this_type& other) noexcept;
		this_type&                                operator=(this_type&& other) noexcept;
		template <typename OInterface> this_type& operator=(const Object<OInterface>& other);

		void release();

		explicit operator bool() const noexcept
		{
			return ptr;
		}
		operator pointer() const noexcept
		{
			return ptr;
		}

		pointer* operator&();
		pointer  operator->() const noexcept;
		pointer  get_ptr() const noexcept
		{
			return ptr;
		}

		bool operator==(const pointer other) const noexcept
		{
			return ptr == other;
		}
		bool operator==(const this_type& other) const noexcept
		{
			return ptr == other.ptr;
		}

		bool operator!=(const pointer other) const noexcept
		{
			return ptr != other;
		}
		bool operator!=(const this_type& other) const noexcept
		{
			return ptr != other.ptr;
		}

		void attach(pointer& other);
		void detach(pointer& other) noexcept;

		void swap(this_type& other) noexcept;

	private:
		pointer ptr;
	};

	template <typename Interface> Object<Interface>::~Object()
	{
		release();
	}

	template <typename Interface> Object<Interface>::Object() noexcept
		: ptr()
	{}

	template <typename Interface> Object<Interface>::Object(const_pointer other)
		: ptr(const_cast<pointer>(other))
	{}

	template <typename Interface> Object<Interface>::Object(const this_type& other) noexcept
		: ptr(other.ptr)
	{
		if (ptr) ptr->AddRef();
	}

	template <typename Interface> Object<Interface>::Object(this_type&& other) noexcept
		: ptr(other.ptr)
	{
		other.ptr = nullptr;
	}

	template <typename Interface> template <typename OInterface>
	Object<Interface>::Object(const Object<OInterface>& other)
		: ptr(static_cast<Interface*>(other.ptr))
	{
		if (ptr) ptr->AddRef();
	}

	template <typename Interface>
	typename Object<Interface>::this_type& Object<Interface>::operator=(const this_type& other) noexcept
	{
		if (ptr != other.ptr) this_type(other).swap(*this);
		return *this;
	}

	template <typename Interface>
	typename Object<Interface>::this_type& Object<Interface>::operator=(this_type&& other) noexcept
	{
		if (ptr != other.ptr) this_type(std::move(other)).swap(*this);
		return *this;
	}

	template <typename Interface> template <typename OInterface>
	typename Object<Interface>::this_type& Object<Interface>::operator=(const Object<OInterface>& other)
	{
		if (ptr != static_cast<OInterface*>(other)) { // FIXME
			this_type tmp(other);
			swap(tmp);
		}
		return *this;
	}

	template <typename Interface> void Object<Interface>::release()
	{
		if (ptr) {
			ptr->Release();
			ptr = nullptr;
		}
	}

	template <typename Interface> typename Object<Interface>::pointer* Object<Interface>::operator&()
	{
		release();
		return &ptr;
	}

	template <typename Interface> typename Object<Interface>::pointer Object<Interface>::operator->() const noexcept
	{
		return ptr;
	}

	template <typename Interface> void Object<Interface>::attach(pointer& other)
	{
		release();
		ptr   = other;
		other = nullptr;
	}

	template <typename Interface> void Object<Interface>::detach(pointer& other) noexcept
	{
		other = ptr;
		ptr   = nullptr;
	}

	template <typename Interface> void Object<Interface>::swap(this_type& other) noexcept
	{
		using std::swap;
		swap(ptr, other.ptr);
	}
} // namespace com

#endif
