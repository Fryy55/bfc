#include "build.hpp"

#include "buildSpecific.hpp"

#include <fmt/base.h>


int build() noexcept {
	TempFile compObj{ "bfc-compiled.o" };
	if (!compObj)
		return 1;
	fmt::println("-- Building file '{}'", compObj.getPath());

	if (auto ret = buildSpecific(compObj); ret != 0)
		return ret;

	// link

	return 0;
}