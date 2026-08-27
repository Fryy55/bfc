#pragma once

#include <slic.hpp>


struct CLIArgs final {
	bool help = false;
	bool version = false;
	bool cmd = false;
	std::string_view outFile{ "./a.out" };

	std::optional<std::string_view> source;


	static constexpr std::string_view Description{ "Brainfuck Compiler" };
	static constexpr std::tuple Options{
		slic::Option{ "--help", "-h", &CLIArgs::help, "Show help" },
		slic::Option{ "--version", "-v", &CLIArgs::version, "Show version info" },
		slic::Option{ "-o", &CLIArgs::outFile, "Output file name" },
		slic::Option{ "-c", &CLIArgs::cmd, "Read source code directly from the command string instead of the file specified by that string" },
		slic::Arg{ "SOURCE", &CLIArgs::source, "Source file/string" }
	};
};