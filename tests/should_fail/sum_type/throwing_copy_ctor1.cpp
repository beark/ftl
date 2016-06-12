#include <ftl/sum_type.h>
#include "throwing_copy_ctor.h"

using namespace ftl;

int main()
{
	sum_type<ThrowingCopyCtor,int> x{type<ThrowingCopyCtor>};
	sum_type<ThrowingCopyCtor,int> y{type<int>, 5};

	// This must not be allowed, or x would end up in an inconsistent state
	// Technically, this particular assignment could be allowed since we're
	// assigning an int. However, at runtime we can't know which state y is in,
	// so all assignments like this must be disallowed.
	x = y;

	// The following would be an acceptable and safe "workaround":
	x = y.match(
			[](int value){ return value; },
			// We must provide a default value in case y doesn't contain an int
			[](otherwise){ return 0; }
		)
	);

	return 0;
}
