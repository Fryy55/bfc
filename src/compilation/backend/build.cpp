#include "build.hpp"

#include "TempFile.hpp"
#include "optimize.hpp"

#include <fmt/base.h>


int build() noexcept {
	optimize();

	TempFile compObj{ "bfc-compiled.o" };
	if (!compObj)
		return 1;
	fmt::println("-- Building file '{}'", compObj.getPath());

	return 0;
}