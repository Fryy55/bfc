#include "buildSpecific.hpp"

#include "endianConvert.hpp"
#include "DataManager.hpp"

#include <elf.h>

#include <vector>
#include <array>
#include <cstring>


static std::vector<std::uint8_t> generateText() noexcept {
	std::vector<std::uint8_t> ret{};

	

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
	auto text = generateText();


//* elf header
	Elf64_Ehdr elfHeader;

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
	Elf64_Shdr nullSectHeader;

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
	Elf64_Shdr sectionStringHeader;

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
	Elf64_Shdr symbolStringHeader;

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
	Elf64_Shdr symbolTableHeader;

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
	Elf64_Shdr textHeader;

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
	Elf64_Sym nullSym;

	nullSym.st_name = 0u;
	nullSym.st_value = 0u;
	nullSym.st_size = 0u;
	nullSym.st_info = 0u;
	nullSym.st_other = 0u;
	endianConvert(nullSym.st_shndx, SHN_UNDEF);

	buf.sputn(TempFile::ptr(nullSym), sizeof(nullSym));


	Elf64_Sym mainSym;

	endianConvert(mainSym.st_name, name("main", true));
	mainSym.st_value = 0u;
	mainSym.st_size = 0u;
	mainSym.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
	mainSym.st_other = STV_DEFAULT;
	endianConvert(mainSym.st_shndx, textIndex);

	buf.sputn(TempFile::ptr(mainSym), sizeof(mainSym));


	Elf64_Sym tapeSizeSym;

	endianConvert(tapeSizeSym.st_name, name("tapeSize", true));
	endianConvert(tapeSizeSym.st_value, DataManager::get()->getArgs().tapeSize);
	tapeSizeSym.st_size = 0u;
	tapeSizeSym.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE);
	tapeSizeSym.st_other = STV_HIDDEN; // preemption protection for pie linkage
	endianConvert(tapeSizeSym.st_shndx, SHN_ABS);

	buf.sputn(TempFile::ptr(tapeSizeSym), sizeof(tapeSizeSym));


	Elf64_Sym silentFailSym;

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