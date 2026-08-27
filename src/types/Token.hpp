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
		Type type, std::size_t backRef
	) : backRef{ backRef }, type{ type } {}

	std::size_t backRef;
	Type type;
};