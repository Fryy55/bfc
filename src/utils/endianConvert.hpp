#pragma once

#include <concepts>
#include <bit>


template <bool Little = true, std::integral T>
	requires (sizeof(T) != 1z)
void endianConvert(T& ret, std::type_identity_t<T> value) noexcept {
	if constexpr (
		Little && std::endian::native == std::endian::little
		||
		!Little && std::endian::native == std::endian::big
	) {
		ret = value;
	} else {
		ret = std::byteswap(value);
	}

	return;
}