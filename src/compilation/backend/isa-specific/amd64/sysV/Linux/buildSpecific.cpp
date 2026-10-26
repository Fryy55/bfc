#include "buildSpecific.hpp"

#include "endianConvert.hpp"

#include <vector>
#include <elf.h>
#include <cstring>


static std::vector<std::uint8_t> generateText() noexcept {
	return { 0xbf, 0x37, 0x00, 0x00, 0x00, 0xb8, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0x05 };
}

#define rodata static constexpr

int buildSpecific(TempFile& file) noexcept {
	/*
		elf header

		section header table

		section header string table
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


	rodata auto sectStrTableName = name(".shstrtab");
	rodata std::size_t sectStrTableOffset = sizeof(elfHeader) + sizeof(Elf64_Shdr) * sectionHeaderCount;
	Elf64_Shdr sectionStringHeader;

	endianConvert(sectionStringHeader.sh_name, sectStrTableName);
	endianConvert(sectionStringHeader.sh_type, SHT_STRTAB);
	sectionStringHeader.sh_flags = 0u;
	sectionStringHeader.sh_addr = 0u;
	endianConvert(sectionStringHeader.sh_offset, sectStrTableOffset);
	endianConvert(sectionStringHeader.sh_size, sectionStringTable.size());
	sectionStringHeader.sh_link = 0u;
	sectionStringHeader.sh_info = 0u;
	endianConvert(sectionStringHeader.sh_addralign, 1u);
	sectionStringHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(sectionStringHeader), sizeof(sectionStringHeader));


	rodata auto symStrTableName = name(".strtab");
	rodata std::size_t symStrTableOffset = sectStrTableOffset + sectionStringTable.size();
	Elf64_Shdr symbolStringHeader;

	endianConvert(symbolStringHeader.sh_name, symStrTableName);
	endianConvert(symbolStringHeader.sh_type, SHT_STRTAB);
	symbolStringHeader.sh_flags = 0u;
	symbolStringHeader.sh_addr = 0u;
	endianConvert(symbolStringHeader.sh_offset, symStrTableOffset);
	endianConvert(symbolStringHeader.sh_size, symbolStringTable.size());
	symbolStringHeader.sh_link = 0u;
	symbolStringHeader.sh_info = 0u;
	endianConvert(symbolStringHeader.sh_addralign, 1u);
	symbolStringHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(symbolStringHeader), sizeof(symbolStringHeader));


	//! symtab


	rodata auto textName = name(".text");
	rodata std::size_t textOffset = symStrTableOffset + symbolStringTable.size();
	Elf64_Shdr textHeader;

	endianConvert(textHeader.sh_name, textName);
	endianConvert(textHeader.sh_type, SHT_PROGBITS);
	endianConvert(textHeader.sh_flags, SHF_ALLOC | SHF_EXECINSTR);
	textHeader.sh_addr = 0u;
	endianConvert(textHeader.sh_offset, textOffset);
	endianConvert(textHeader.sh_size, text.size());
	textHeader.sh_link = 0u;
	textHeader.sh_info = 0u;
	endianConvert(textHeader.sh_addralign, 16u);
	textHeader.sh_entsize = 0u;

	buf.sputn(TempFile::ptr(textHeader), sizeof(textHeader));


//* section data
	buf.sputn(sectionStringTable.data(), sectionStringTable.size());
	buf.sputn(symbolStringTable.data(), symbolStringTable.size());

	//* symbol table


	//* text
	buf.sputn(TempFile::ptr(text.data()), text.size());

	return 0;
}