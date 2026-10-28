#include "buildSpecific.hpp"

#include "endianConvert.hpp"

#include <elf.h>

#include <vector>
#include <array>
#include <cstring>


static std::vector<std::uint8_t> generateText() noexcept {
	return { 0xbf, 0x37, 0x00, 0x00, 0x00, 0xb8, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0x05 };
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
	rodata std::array<char, 64z> sectionStringTable{ "\0.shstrtab\0.strtab\0.symtab\0.text" };
	auto name = [](std::string_view str) static consteval noexcept {
		return std::string_view{ sectionStringTable.data(), sectionStringTable.size() }.find(str);
	};
	rodata std::array<char, 64z> symbolStringTable{ "\0_start" };
	/*
		null
		_start
	*/
	rodata std::size_t symbolCount = 2z;
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
	rodata auto sectStrTableName = name(".shstrtab");
	Elf64_Shdr sectionStringHeader;

	endianConvert(sectionStringHeader.sh_name, sectStrTableName);
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
	rodata auto symStrTableName = name(".strtab");
	Elf64_Shdr symbolStringHeader;

	endianConvert(symbolStringHeader.sh_name, symStrTableName);
	endianConvert(symbolStringHeader.sh_type, SHT_STRTAB);
	endianConvert(symbolStringHeader.sh_flags, SHF_ALLOC);
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
	rodata auto symTabName = name(".symtab");
	Elf64_Shdr symbolTableHeader;

	endianConvert(symbolTableHeader.sh_name, symTabName);
	endianConvert(symbolTableHeader.sh_type, SHT_SYMTAB);
	endianConvert(symbolTableHeader.sh_flags, SHF_ALLOC);
	symbolTableHeader.sh_addr = 0u;
	endianConvert(symbolTableHeader.sh_offset, curOffset);
	endianConvert(symbolTableHeader.sh_size, sizeof(Elf64_Sym) * symbolCount);
	endianConvert(symbolTableHeader.sh_link, 2u); // associated strtab
	symbolTableHeader.sh_info = 0u;
	endianConvert(symbolTableHeader.sh_addralign, 1u);
	endianConvert(symbolTableHeader.sh_entsize, sizeof(Elf64_Sym));

	buf.sputn(TempFile::ptr(symbolTableHeader), sizeof(symbolTableHeader));
	curOffset += sizeof(Elf64_Sym) * symbolCount;


	// .text
	rodata auto textName = name(".text");
	Elf64_Shdr textHeader;

	endianConvert(textHeader.sh_name, textName);
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
	Elf64_Sym nullSym;

	nullSym.st_name = 0u;
	nullSym.st_value = 0u;
	nullSym.st_size = 0u;
	nullSym.st_info = 0u;
	nullSym.st_other = 0u;
	endianConvert(nullSym.st_shndx, SHN_UNDEF);

	buf.sputn(TempFile::ptr(nullSym), sizeof(nullSym));


	Elf64_Sym startSym;

	endianConvert(startSym.st_name, 1u);
	startSym.st_value = 0u;
	endianConvert(startSym.st_size, 8u);
	startSym.st_info = 0u;
	endianConvert(startSym.st_other, );
	endianConvert(startSym.st_shndx, 4u); // .text shtab index

	buf.sputn(TempFile::ptr(startSym), sizeof(startSym));


	// .text
	buf.sputn(TempFile::ptr(text.data()), text.size());

	return 0;
}