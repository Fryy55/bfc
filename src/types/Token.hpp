#pragma once

#include <cstdint>


struct Token final {
	enum class Type : std::uint8_t {
		Right,
		Left,
		Plus,
		Minus,
		Out,
		In,
		OpenLoop,
		CloseLoop
	};

	constexpr Token(Type type) : type{ type } {}
	constexpr Token(
		Type type, std::uint64_t refVal
	) : refVal{ refVal }, type{ type } {}

	constexpr operator bool() const noexcept { return valid; }

	std::uint64_t refVal;
	Type type;
	bool valid = true;
};