#pragma once

#include <cstdint>
#include <utility>


constexpr bool fits32(std::int64_t x) noexcept {
	return std::in_range<std::int32_t>(x) || std::in_range<std::uint32_t>(x);
}