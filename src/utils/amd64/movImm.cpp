#include "movImm.hpp"

#include "endianConvert.hpp"
#include "fits32.hpp"


namespace amd64 {

MovInstruction movImm(Reg reg, std::int64_t imm) noexcept {
	// xor eRR, eRR      | [REX.RB] 31 Ed, Gd
	// mov eRR, posimm32 | [REX.B] b8+rd imm32
	// mov rRR, negimm32 | REX.W[B] c7-/0 Eq, Id
	// mov rRR, imm64    | REX.W[B] b8+rd imm64

	using Bytes = std::initializer_list<std::uint8_t>;

	bool rexR = false;
	bool rexB = false;
	auto genREX = [&rexR, &rexB](bool rexW) noexcept {
		// 0100WRXB
		return static_cast<std::uint8_t>(0x40 | rexW << 3z | rexR << 2z | rexB);
	};

	if (auto regInt = std::to_underlying(reg); regInt & 0b1'000) {
		reg = static_cast<Reg>(regInt ^ 0b1'000);
		rexB = true;
	}

	if (fits32(imm)) {
		if (imm == 0) {
			MovInstruction ret{};

			if (rexB) {
				rexR = true;
				ret.add(genREX(false));
			}
			
			ret.add(Bytes{
				// [REX] 31 ModR/M(reg)
				0x31, static_cast<std::uint8_t>(
					0b11'000'000 | std::to_underlying(reg) << 3z | std::to_underlying(reg)
				)
			});

			return ret;
		} else if (imm < 0) {
			std::int32_t immVal;
			endianConvert(immVal, imm);
			auto immBytes = std::bit_cast<std::array<std::uint8_t, 4z>>(immVal);

			// REX c7-/0 ModR/M(op) imm32
			return {
				genREX(true), 0xc7,
				static_cast<std::uint8_t>(0b11'000'000 | std::to_underlying(reg)),
				immBytes[0z], immBytes[1z], immBytes[2z], immBytes[3z]
			};
		} else { // positive
			MovInstruction ret{};

			std::uint32_t immVal;
			endianConvert(immVal, imm);
			auto immBytes = std::bit_cast<std::array<std::uint8_t, 4z>>(immVal);

			if (rexB)
				ret.add(genREX(false));

			ret.add(Bytes{
				// [REX] b8+rd imm32
				static_cast<std::uint8_t>(0xb8 + std::to_underlying(reg)),
				immBytes[0z], immBytes[1z], immBytes[2z], immBytes[3z]
			});

			return ret;
		}
	}

	// 64bit imm
	std::int64_t immVal;
	endianConvert(immVal, imm);
	auto immBytes = std::bit_cast<std::array<std::uint8_t, 8z>>(immVal);

	// REX b8+rd imm64
	return {
		genREX(true),
		static_cast<std::uint8_t>(0xb8 + std::to_underlying(reg)),
		immBytes[0z], immBytes[1z], immBytes[2z], immBytes[3z], immBytes[4z], immBytes[5z], immBytes[6z], immBytes[7z]
	};
}

} // namespace amd64