#include "buildSpecific.hpp"

#include "DataManager.hpp"
#include "TokensIterator.hpp"
#include "fits32.hpp"
#include "amd64/movImm.hpp"
#include "endianConvert.hpp"

#include <elf.h>

#include <fmt/color.h>

#include <vector>
#include <array>
#include <cstring>


[[nodiscard]] static std::vector<std::uint8_t> generateText() {
	std::vector<std::uint8_t> ret{};
	auto& args = DataManager::get()->getArgs();
	using Bytes = std::initializer_list<std::uint8_t>;

	for (auto& token : TokensIterator{}) {
		switch (token.type) {
			case Token::Type::Shift:
				if (args.inlineCalls) {
					bool isImm = fits32(token.refVal);
					if (!isImm)
						ret.append_range(movImm(amd64::Reg::A, std::abs(token.refVal)));

					if (token.refVal < 0) { // left
						/*
							?                    3 + 2 = 5
							?                    +-------+
							?                    |       |
							?                   rip      v
							? sub lShL: cmp jbe add jmp cShL:
							? 7/3       3   2   3   2   rip
							?           ^                |
							?           +----------------+
							?  -(3 + 2 + 3 + 2) = -10 = 0b1111'0110
						*/

						if (isImm) {
							std::uint32_t immVal;
							endianConvert(immVal, std::abs(token.refVal));
							auto immBytes = std::bit_cast<std::array<std::uint8_t, 4z>>(immVal);

							//# sub rsi, imm32
							//# REX.Wrxb(d32) 81-/5 Ev(0.110), Iz
							ret.append_range(Bytes{
								0b0100'1000, 0x81, 0b11'101'110,
								immBytes[0z], immBytes[1z], immBytes[2z], immBytes[3z]
							});
						} else {
							//# sub rsi, rax
							//# REX.Wrxb(d32) 29 Ev(0.110), Gv(0.000)
							ret.append_range(Bytes{ 0b0100'1000, 0x29, 0b11'000'110 });
						}

						//* loopShl:
						//# cmp r8, rsi
						//# REX.WrxB(d32) 39 Ev(1.000), Gv(0.110)
						ret.append_range(Bytes{ 0b0100'1001, 0x39, 0b11'110'000 });
						//# jbe rip+contShL
						//# (f64) 76 Jb(chart above)
						ret.append_range(Bytes{ 0x76, 5 });

						//# add rsi, r10
						//# REX.WRxb(d32) 01 Ev(0.110), Gv(1.010)
						ret.append_range(Bytes{ 0b0100'1100, 0x01, 0b11'010'110 });
						//# jmp rip+loopShL
						//# (f64) eb Jb(chart above)
						ret.append_range(Bytes{ 0xeb, 0b1111'0110 });
					} else { // right
						/*
							?                   3 + 2 = 5
							?                   +-------+
							?                   |       |
							?                  rip      v
							? add lShR: cmp jb sub jmp cShR:
							? 7/3       3   2  3   2   rip
							?           ^               |
							?           +---------------+
							?  -(3 + 2 + 3 + 2) = -10 = 0b1111'0110
						*/

						if (isImm) {
							std::uint32_t immVal;
							endianConvert(immVal, std::abs(token.refVal));
							auto immBytes = std::bit_cast<std::array<std::uint8_t, 4z>>(immVal);

							//# add rsi, imm32
							//# REX.Wrxb(d32) 81-/0 Ev(0.110), Iz
							ret.append_range(Bytes{
								0b0100'1000, 0x81, 0b11'000'110,
								immBytes[0z], immBytes[1z], immBytes[2z], immBytes[3z]
							});
						} else {
							//# add rsi, rax
							//# REX.Wrxb(d32) 01 Ev(0.110), Gv(0.000)
							ret.append_range(Bytes{ 0b0100'1000, 0x01, 0b11'000'110 });
						}

						//* loopShl:
						//# cmp rsi, r9
						//# REX.WRxb(d32) 39 Ev(0.110), Gv(1.001)
						ret.append_range(Bytes{ 0b0100'1100, 0x39, 0b11'001'110 });
						//# jb rip+contShR
						//# (f64) 72 Jb(chart above)
						ret.append_range(Bytes{ 0x72, 5 });

						//# sub rsi, r10
						//# REX.WRxb(d32) 29 Ev(0.110), Gv(1.010)
						ret.append_range(Bytes{ 0b0100'1100, 0x29, 0b11'010'110 });
						//# jmp rip+loopShR
						//# (f64) eb Jb(chart above)
						ret.append_range(Bytes{ 0xeb, 0b1111'0110 });
					}
				} else { // indirect call
					//# mov rax, shift
					ret.append_range(movImm(amd64::Reg::A, std::abs(token.refVal)));
					if (token.refVal < 0) { // left
						//# call r14
						//# REX.wrxB(f64) ff-/2 Ev(1.110)
						ret.append_range(Bytes{ 0b0100'0001, 0xff, 0b11'010'110 });
					} else { // right
						//# call r15
						//# REX.wrxB(f64) ff-/2 Ev(1.111)
						ret.append_range(Bytes{ 0b0100'0001, 0xff, 0b11'010'111 });
					}
				}
				break;

			case Token::Type::Add: { //* token.refVal is guaranteed to be within [-255; 255]
				auto oldVal = token.refVal;
				token.refVal = std::abs(token.refVal);

				if (oldVal == token.refVal) { // positive
					if (token.refVal == 1) {
						//# inc byte [rsi]
						//# fe-/0 Eb(no disp; 0.110)
						ret.append_range(Bytes{ 0xfe, 0b00'000'110 });
					} else {
						//# add byte [rsi], imm8
						//# 80-/0 Eb(no disp; 0.110), Ib
						ret.append_range(Bytes{
							0x80, 0b00'000'110,
							static_cast<std::uint8_t>(token.refVal)
						});
					}
				} else { // negative
					if (token.refVal == 1) {
						//# dec byte [rsi]
						//# fe-/1 Eb(no disp; 0.110)
						ret.append_range(Bytes{ 0xfe, 0b00'001'110 });
					} else {
						//# sub byte [rsi], imm8
						//# 80-/5 Eb(no disp; 0.110), Ib
						ret.append_range(Bytes{
							0x80, 0b00'101'110,
							static_cast<std::uint8_t>(token.refVal)
						});
					}
				}
			} break;

			case Token::Type::Out:
				if (args.inlineCalls) {
					ret.append_range(movImm(amd64::Reg::A, 1));
					//# syscall
					//# 0f 05
					ret.append_range(Bytes{ 0x0f, 0x05 });
				} else { // indirect call
					//# call r13
					//# REX.wrxB(f64) ff-/2 Ev(1.101)
					ret.append_range(Bytes{ 0b0100'0001, 0xff, 0b11'010'101 });
				}
				break;

			case Token::Type::In:
				if (args.inlineCalls) {
					ret.append_range(movImm(amd64::Reg::A, 0));
					//# syscall
					//# 0f 05
					ret.append_range(Bytes{ 0x0f, 0x05 });
				} else { // indirect call
					//# call r12
					//# REX.wrxB(f64) ff-/2 Ev(1.100)
					ret.append_range(Bytes{ 0b0100'0001, 0xff, 0b11'010'100 });
				}
				break;

			//* validity of matching `[`/`]` tokens is guaranteed by the frontend
			/*
				?     sizes
				?     ( n ) x
				? , [  ++>  ] .
				?   ^  ^    ^ ^
				?   |  loop | cont:
				?   |       test byte [rsi], 0xff; jnz rip-???(loop)
				?   test byte [rsi], 0xff; jz rip+???(cont)
				?
				? ??? = n + x
				? x = 3 + 2/6 = 5/9
				?
				? jz/jnz rip+disp8  | 74/75 disp8     | 2 bytes
				? jz/jnz rip+disp32 | 0f 84/85 disp32 | 6 bytes
			*/
			case Token::Type::OpenLoop: {
				//# test byte [rsi], 0xff
				//# f6-/0 Ev(0.110) Ib
				std::array<std::uint8_t, 3z> testInstr{
					0xf6, 0b00'000'110, 0xff
				};

				token.valid = false;
				auto loopText = generateText();
				std::size_t n = loopText.size();
				bool disp8 = false;

				ret.append_range(testInstr);
				// disp8 = [-128; 127] =>
				// => to fit, n + x <= 127; n + 5 <= 127
				if (n + 5z <= 127z) { // fits disp8 both ways
					//# jz rip+disp8
					//# (f64) 74 Jb
					ret.append_range(Bytes{
						0x74, static_cast<std::uint8_t>(n + 5z)
					});

					disp8 = true;
				} else {
					std::int32_t dispVal;
					endianConvert(dispVal, n + 9z);
					auto dispBytes = std::bit_cast<std::array<std::uint8_t, 4z>>(dispVal);

					//# jz rip+disp32
					//# (f64) 0f 84 Jd
					ret.append_range(Bytes{
						0x0f, 0x84,
						dispBytes[0z], dispBytes[1z], dispBytes[2z], dispBytes[3z]
					});
				}

				ret.append_range(std::move(loopText));

				ret.append_range(testInstr);
				if (disp8) {
					//# jnz rip+disp8
					//# (f64) 75 Jb
					ret.append_range(Bytes{
						0x75, static_cast<std::uint8_t>(-(n + 5z))
					});

					disp8 = true;
				} else {
					std::int32_t dispVal;
					endianConvert(dispVal, -(n + 9z));
					auto dispBytes = std::bit_cast<std::array<std::uint8_t, 4z>>(dispVal);

					//# jnz rip+disp32
					//# (f64) 0f 85 Jd
					ret.append_range(Bytes{
						0x0f, 0x85,
						dispBytes[0z], dispBytes[1z], dispBytes[2z], dispBytes[3z]
					});
				}
			} break;

			case Token::Type::CloseLoop:
				token.valid = false;
				return ret;

			default:
				throw std::runtime_error{ fmt::format("invalid token of type '{}'", Token::typeToString(token.type)) };
		}

		token.valid = false;
	}

	//# ret
	//# c3
	ret.emplace_back(0xc3);
	return ret;
}

#define rodata static constexpr

int buildSpecific(TempFile& file) noexcept {
	/*
		elf header

		section header table

		section string table
		symbol string table

		symbol table

		text
	*/

	auto buf = file.getWriteBuf();

	/*
		null
		.shstrtab
		.strtab
		.symtab
		.text
	*/
	rodata std::size_t sectionHeaderCount = 5z;
	rodata std::size_t strTabIndex = 2z;
	rodata std::size_t textIndex = 4z;
	rodata std::array<char, 64z> sectionStringTable{ "\0.shstrtab\0.strtab\0.symtab\0.text" };
	rodata std::array<char, 64z> symbolStringTable{ "\0main\0tapeSize\0silentFail" };
	auto name = [](std::string_view str, bool useSymStrTab = false) static consteval noexcept {
		return useSymStrTab ?
				std::string_view{ symbolStringTable.data(), symbolStringTable.size() }.find(str)
				:
				std::string_view{ sectionStringTable.data(), sectionStringTable.size() }.find(str);
	};
	/*
		null
		main
		tapeSize
		silentFail
	*/
	rodata std::size_t symbolCount = 4z;
	std::vector<std::uint8_t> text;
	try {
		text = generateText();
	} catch (std::exception const& e) {
		fmt::println(
			fmt::fg(fmt::color::crimson),
			"-- Error while generating machine code: {}",
			fmt::styled(e.what(), fmt::emphasis::bold)
		);
		return 1;
	}


//* elf header
	Elf64_Ehdr elfHeader{};

	elfHeader.e_ident[EI_MAG0] = 0x7f;
	elfHeader.e_ident[EI_MAG1] = 'E';
	elfHeader.e_ident[EI_MAG2] = 'L';
	elfHeader.e_ident[EI_MAG3] = 'F';
	elfHeader.e_ident[EI_CLASS] = ELFCLASS64;
	elfHeader.e_ident[EI_DATA] = ELFDATA2LSB;
	elfHeader.e_ident[EI_VERSION] = EV_CURRENT;
	elfHeader.e_ident[EI_OSABI] = ELFOSABI_LINUX;
	elfHeader.e_ident[EI_ABIVERSION] = 0u;
	std::memset(elfHeader.e_ident + EI_PAD, 0u, EI_NIDENT - EI_PAD);

	endianConvert(elfHeader.e_type, ET_REL);
	endianConvert(elfHeader.e_machine, EM_X86_64);
	endianConvert(elfHeader.e_version, EV_CURRENT);
	elfHeader.e_entry = 0u;
	endianConvert(elfHeader.e_phoff, 0u);
	endianConvert(elfHeader.e_shoff, sizeof(Elf64_Ehdr));
	elfHeader.e_flags = 0u;
	endianConvert(elfHeader.e_ehsize, sizeof(Elf64_Ehdr));
	endianConvert(elfHeader.e_phentsize, sizeof(Elf64_Phdr));
	endianConvert(elfHeader.e_phnum, 0u);
	endianConvert(elfHeader.e_shentsize, sizeof(Elf64_Shdr));
	endianConvert(elfHeader.e_shnum, sectionHeaderCount);
	endianConvert(elfHeader.e_shstrndx, 1u);

	buf.sputn(TempFile::ptr(elfHeader), sizeof(elfHeader));


//* section header table
	// null
	Elf64_Shdr nullSectHeader{};

	nullSectHeader.sh_name = 0u;
	endianConvert(nullSectHeader.sh_type, SHT_NULL);
	nullSectHeader.sh_flags = 0u;
	nullSectHeader.sh_addr = 0u;
	nullSectHeader.sh_offset = 0u;
	nullSectHeader.sh_size = 0u;
	nullSectHeader.sh_link = 0u;
	nullSectHeader.sh_info = 0u;
	nullSectHeader.sh_addralign = 0u;
	nullSectHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(nullSectHeader), sizeof(nullSectHeader));
	std::size_t curOffset = sizeof(elfHeader) + sizeof(Elf64_Shdr) * sectionHeaderCount;


	// .shstrtab
	Elf64_Shdr sectionStringHeader{};

	endianConvert(sectionStringHeader.sh_name, name(".shstrtab"));
	endianConvert(sectionStringHeader.sh_type, SHT_STRTAB);
	sectionStringHeader.sh_flags = 0u;
	sectionStringHeader.sh_addr = 0u;
	endianConvert(sectionStringHeader.sh_offset, curOffset);
	endianConvert(sectionStringHeader.sh_size, sectionStringTable.size());
	sectionStringHeader.sh_link = 0u;
	sectionStringHeader.sh_info = 0u;
	endianConvert(sectionStringHeader.sh_addralign, 1u);
	sectionStringHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(sectionStringHeader), sizeof(sectionStringHeader));
	curOffset += sectionStringTable.size();


	// .strtab
	Elf64_Shdr symbolStringHeader{};

	endianConvert(symbolStringHeader.sh_name, name(".strtab"));
	endianConvert(symbolStringHeader.sh_type, SHT_STRTAB);
	symbolStringHeader.sh_flags = 0u;
	symbolStringHeader.sh_addr = 0u;
	endianConvert(symbolStringHeader.sh_offset, curOffset);
	endianConvert(symbolStringHeader.sh_size, symbolStringTable.size());
	symbolStringHeader.sh_link = 0u;
	symbolStringHeader.sh_info = 0u;
	endianConvert(symbolStringHeader.sh_addralign, 1u);
	symbolStringHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(symbolStringHeader), sizeof(symbolStringHeader));
	curOffset += symbolStringTable.size();


	// .symtab
	Elf64_Shdr symbolTableHeader{};

	endianConvert(symbolTableHeader.sh_name, name(".symtab"));
	endianConvert(symbolTableHeader.sh_type, SHT_SYMTAB);
	symbolTableHeader.sh_flags = 0u;
	symbolTableHeader.sh_addr = 0u;
	endianConvert(symbolTableHeader.sh_offset, curOffset);
	endianConvert(symbolTableHeader.sh_size, sizeof(Elf64_Sym) * symbolCount);
	endianConvert(symbolTableHeader.sh_link, strTabIndex);
	endianConvert(symbolTableHeader.sh_info, 1u); //! for one (null) local symbol
	endianConvert(symbolTableHeader.sh_addralign, 1u);
	endianConvert(symbolTableHeader.sh_entsize, sizeof(Elf64_Sym));

	buf.sputn(TempFile::ptr(symbolTableHeader), sizeof(symbolTableHeader));
	curOffset += sizeof(Elf64_Sym) * symbolCount;


	// .text
	Elf64_Shdr textHeader{};

	endianConvert(textHeader.sh_name, name(".text"));
	endianConvert(textHeader.sh_type, SHT_PROGBITS);
	endianConvert(textHeader.sh_flags, SHF_ALLOC | SHF_EXECINSTR);
	textHeader.sh_addr = 0u;
	endianConvert(textHeader.sh_offset, curOffset);
	endianConvert(textHeader.sh_size, text.size());
	textHeader.sh_link = 0u;
	textHeader.sh_info = 0u;
	endianConvert(textHeader.sh_addralign, 16u);
	textHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(textHeader), sizeof(textHeader));
	curOffset += text.size();


//* section data
	buf.sputn(sectionStringTable.data(), sectionStringTable.size());
	buf.sputn(symbolStringTable.data(), symbolStringTable.size());

	// .symtab
	//? one local symbol, sh_info on .symtab is 1
	Elf64_Sym nullSym{};

	nullSym.st_name = 0u;
	nullSym.st_value = 0u;
	nullSym.st_size = 0u;
	nullSym.st_info = 0u;
	nullSym.st_other = 0u;
	endianConvert(nullSym.st_shndx, SHN_UNDEF);

	buf.sputn(TempFile::ptr(nullSym), sizeof(nullSym));


	Elf64_Sym mainSym{};

	endianConvert(mainSym.st_name, name("main", true));
	mainSym.st_value = 0u;
	mainSym.st_size = 0u;
	mainSym.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
	mainSym.st_other = STV_DEFAULT;
	endianConvert(mainSym.st_shndx, textIndex);

	buf.sputn(TempFile::ptr(mainSym), sizeof(mainSym));


	Elf64_Sym tapeSizeSym{};

	endianConvert(tapeSizeSym.st_name, name("tapeSize", true));
	endianConvert(tapeSizeSym.st_value, DataManager::get()->getArgs().tapeSize);
	tapeSizeSym.st_size = 0u;
	tapeSizeSym.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE);
	tapeSizeSym.st_other = STV_HIDDEN; // preemption protection for pie linkage
	endianConvert(tapeSizeSym.st_shndx, SHN_ABS);

	buf.sputn(TempFile::ptr(tapeSizeSym), sizeof(tapeSizeSym));


	Elf64_Sym silentFailSym{};

	endianConvert(silentFailSym.st_name, name("silentFail", true));
	endianConvert(
		silentFailSym.st_value,
		DataManager::get()->getArgs().silentFail ? 1u : 0u
	);
	silentFailSym.st_size = 0u;
	silentFailSym.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE);
	silentFailSym.st_other = STV_HIDDEN; // preemption protection for pie linkage
	endianConvert(silentFailSym.st_shndx, SHN_ABS);

	buf.sputn(TempFile::ptr(silentFailSym), sizeof(silentFailSym));


	// .text
	buf.sputn(TempFile::ptr(text.data()), text.size());

	return 0;
}