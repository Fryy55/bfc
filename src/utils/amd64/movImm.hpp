#pragma once

#include <cstdint>
#include <array>
#include <algorithm>


namespace amd64 {

enum class Reg : std::uint8_t {
	A = 0b0'000,
	C = 0b0'001,
	D = 0b0'010,
	B = 0b0'011,
	SP = 0b0'100,
	BP = 0b0'101,
	SI = 0b0'110,
	DI = 0b0'111,
	R8 = 0b1'000,
	R9 = 0b1'001,
	R10 = 0b1'010,
	R11 = 0b1'011,
	R12 = 0b1'100,
	R13 = 0b1'101,
	R14 = 0b1'110,
	R15 = 0b1'111,
};


struct MovInstruction final {
	constexpr MovInstruction(std::initializer_list<std::uint8_t> initList) {
		size = initList.size();
		std::ranges::copy(initList, data.begin());
	}


	[[nodiscard]] constexpr auto begin(this auto&& self) noexcept {
		return self.data.begin();
	}
	[[nodiscard]] constexpr auto end(this auto&& self) noexcept {
		return self.data.begin() + self.size;
	}

	[[nodiscard]] constexpr auto& operator[](this auto&& self, std::size_t i) noexcept {
		return self.data[i];
	}

	constexpr void add(std::uint8_t x) noexcept {
		if (size == data.size())
			return;

		data[size++] = x;

		return;
	}

	template <std::ranges::sized_range R>
	constexpr void add(R&& range) noexcept {
		if (data.size() < size + range.size())
			return;

		auto oldSize = size;
		size += range.size();
		std::ranges::copy(std::forward<R>(range), data.begin() + oldSize);

		return;
	}


	std::size_t size = 0z;
	std::array<std::uint8_t, 10z> data{};
};


[[nodiscard]] MovInstruction movImm(Reg reg, std::int64_t imm) noexcept;

} // namespace amd64