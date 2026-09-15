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
		return 0;
	}

	if (!args.source) {
		std::puts("Error: Missing argument 'SOURCE'");
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