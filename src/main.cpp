#include "CLIArgs.hpp"
#include "DataManager.hpp"
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
	DataManager::get()->setArgs(args);
	if (args.help) {
		parser.printHelp();
		return 0;
	} else if (args.version) {
		std::puts("\e[34mBrainfuck Compiler v" BFC_VERSION "\e[0m");
		return 0;
	}

	if (!args.source) {
		std::puts("\e[31mError: Missing argument 'SOURCE'\e[0m");
		parser.printHelp();
		return 1;
	}

	return compile();
}