#include "CLIArgs.hpp"
#include "compile.hpp"


int main(int argc, char** argv) {
	slic::ArgParser<CLIArgs> parser{ argc, argv };

	auto res = parser.parse();
	if (!res) {
		res.print();
		parser.printHelp();
		return 1;
	}

	CLIArgs const& args = parser.result();
	if (args.help) {
		parser.printHelp();
		return 0;
	} else if (args.version) {
		std::puts("Brainfuck Compiler v" BFC_VERSION);
		return 0;
	}

	if (!args.source) {
		std::puts("Error: Missing argument 'SOURCE'");
		parser.printHelp();
		return 1;
	}

	return compile(args);
}