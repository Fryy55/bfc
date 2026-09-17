#pragma once

#include <cstdint>


struct Token final {
	enum class Type : std::uint8_t {
		None = 0u,

		Shift = 1u,
		Add = 1u << 1z,
		Out = 1u << 2z,
		In = 1u << 3z,
		OpenLoop = 1u << 4z,
		CloseLoop = 1u << 5z
	};


	constexpr Token(Type type) : type{ type } {}
	constexpr Token(
		Type type, std::int64_t refVal
	) : refVal{ refVal }, type{ type } {}

	constexpr operator bool() const noexcept { return valid; }


	std::int64_t refVal;
	Type type;
	bool valid = true;
};