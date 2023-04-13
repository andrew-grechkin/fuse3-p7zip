#ifndef __FUSE3_7Z__PATTERN_HPP_
#define __FUSE3_7Z__PATTERN_HPP_

#include <cstdlib>

namespace pattern {
	struct Command {
		virtual ~Command() = default;

		virtual ssize_t execute() = 0;
	};

	struct Destroyable {
		virtual ~Destroyable() = default;

		virtual void destroy() const = 0;
	};

	class Uncopyable {
	public:
		Uncopyable(const Uncopyable&)            = delete;
		Uncopyable& operator=(const Uncopyable&) = delete;

	protected:
		~Uncopyable() = default;
		Uncopyable()  = default;
	};

	template <typename T> struct Singleton: private T {
		typedef T            implementation_type;
		typedef Singleton<T> this_type;
		static T&            instance()
		{
			static this_type instance;
			return instance;
		}

	private:
		~Singleton() = default;
		Singleton() {}
	};
} // namespace pattern

#endif
