#include <stdexcept>

struct ThrowingCopyCtor
{
	ThrowingCopyCtor() = default;
	ThrowingCopyCtor(const ThrowingCopyCtor&)
	{
		throw std::exception();
	}

	ThrowingCopyCtor(ThrowingCopyCtor&&) = default;
	~ThrowingCopyCtor() = default;
};