#include <ftl/sum_type.h>
#include "throwing_copy_ctor.h"

using namespace ftl;

int main()
{
	sum_type<ThrowingCopyCtor,int> x{type<int>, 5};
	ThrowingCopyCtor y{};

	// This must not be allowed, or x would end up in an inconsistent state
	x.emplace(type<ThrowingCopyCtor>, y);

	return 0;
}