#pragma once

#include <slic.hpp>


struct CLIArgs final {
	std::optional<std::string_view> source;

	std::string_view outFile{ "./a.out" };
	std::string_view linker{ "/usr/bin/ld" };
	std::string_view linkerOptions{ "" };
	std::string_view tempDir{ "/tmp" };
	std::uint64_t tapeSize = 32'768u;
	bool o0 = false;
	bool o1 = false;
	bool o2 = false;
	std::uint8_t o;
	bool help = false;
	bool version = false;
	bool cmd = false;
	bool dump = false;

	static constexpr std::string_view Description{ "Brainfuck Compiler" };
	static constexpr std::tuple Options{
		slic::Option{ "--help", "-h", &CLIArgs::help, "Show help" },
		slic::Option{ "--version", "-v", &CLIArgs::version, "Show version info" },
		slic::Option{ "--tape-size", "-t", &CLIArgs::tapeSize, "Tape size to use (default 32768 bytes)" },
		slic::Option{ "-O0", &CLIArgs::o0, "Disable all optimizations" },
		slic::Option{ "-O1", &CLIArgs::o1, "Enable dead code elimination optimizations when possible (default)" },
		slic::Option{ "-O2", &CLIArgs::o2, "Enable all -O1 optimizations + instruction collapsing (e.g. consecutive `+`s become a single add instruction)" },
		slic::Option{ "--linker", "-L", &CLIArgs::linker, "Linker to use for the final executable (default `/usr/bin/ld`)" },
		slic::Option{ "--linker-options", "-l", &CLIArgs::linkerOptions, "Options passed to the linker (default none)" },
		slic::Option{ "--output", "-o", &CLIArgs::outFile, "Output file name (default `a.out`)" },
		slic::Option{ "--dump", "-d", &CLIArgs::dump, "Dump object files into current working directory post compilation instead of linking them" },
		slic::Option{ "--tmp-dir", "-T", &CLIArgs::tempDir, "Directory to use for temporary object files storage (default `/tmp`)" },
		slic::Option{ "--cli-source", "-c", &CLIArgs::cmd, "Read source code directly from the command string instead of the file specified by that string" },
		slic::Arg{ "SOURCE", &CLIArgs::source, "Source file/string" }
	};
};