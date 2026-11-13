#include "CLIArgs.hpp"
#include "DataManager.hpp"
#include "compile.hpp"

#include <fmt/color.h>


int main(int argc, char** argv) {
	slic::ArgParser<CLIArgs> parser{ argc, argv };

	auto res = parser.parse();
	if (!res) {
		res.print();
		parser.printHelp();
		return 1;
	}

	CLIArgs& args = parser.result();
	DataManager::get()->setArgs(args);
	if (args.help) {
		parser.printHelp();
		return 0;
	} else if (args.version) {
		fmt::println(fmt::fg(fmt::color::blue), "Brainfuck Compiler v" BFC_VERSION);
		fmt::println(
			fmt::fg(BFC_CROSSCOMPILING ? fmt::color::blue_violet : fmt::color::cadet_blue),
			BFC_CROSSCOMPILER " " BFC_OS_NAME " on architecture " BFC_ISA " (ABI: " BFC_ABI ")"
		);
		return 0;
	}

	if (!args.source) {
		fmt::println(fmt::emphasis::bold, "Error: Missing argument 'SOURCE'\n");
		parser.printHelp();
		return 1;
	}

	if (args.tapeSize == 0u) {
		fmt::println(fmt::emphasis::bold, "Error: Tape size cannot be 0\n");
		parser.printHelp();
		return 1;
	}

	args.o = 1u;
	if (args.o0)
		args.o = 0u;
	else if (args.o2)
		args.o = 2u;

	return compile();
}